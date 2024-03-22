#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <msp430.h>
#include "system.h"
#include "CommDataDefine.h"

#include "Main.h"
#include "BlueTooth.h"
#include "Crc.h"
#include "OLed.h"
#include "Measinter.h"
#include "OtherParam.h"
#include "Input.h"
#include "Uart.h"
#include "SelfCheck.h"

BYTE BTErrCode = 0;

extern DWORD System_Freq;
extern QInnerParam InnerParam;
extern INTERFACE currUI;
extern CHECKRES CheckRes;

/*************************************************/
/*             BT  物理层                        */
/*************************************************/
void BT_IoInit();
void BT_Sleep();
void BT_WakeUp();
void BT_Setting();
void BT_Working();
void BT_UartInit();
void BT_Uart_Config();


BYTE BTRecvBuf[BT_BUF_LEN];
BYTE BTSendBuf[BT_BUF_LEN];

WORD BTRecLen = 0;

#define AT_CMD_LEN 24
BYTE AtCmdBuf[AT_CMD_LEN];

void BT_HwInit()
{
    BT_UartInit();
    BT_IoInit();
    
    BT_PowerOn();
    BT_Rst(1);
    BT_Working();
    BT_WakeUp();
}

void BT_IoInit()
{
    // P8.2 -- SET SW
    // P8.1 -- PowerDn
    P8DIR |=  BIT1 | BIT2; 
    P8DIR |=  BIT0;
}

void BT_PowerOn()
{
    #ifdef BATTERY_VER
    BT_ON;
    #endif
}

void BT_PowerOff()
{
    #ifdef BATTERY_VER
    BT_OFF;
    #endif
}

void BT_Rst(BYTE lv)
{
    if (lv == 1)    //高电平工作
    {
        P8OUT |= BIT0;
    }
    else           // 低电平复位
    {
        P8OUT &= ~BIT0;
    }
}


void BT_Sleep()
{
    delay_ms(10);
    P8OUT &= ~BIT1;
}

void BT_WakeUp()
{
    P8OUT |= BIT1;
    delay_ms(10);
}

void BT_Setting()
{
    P8OUT |= BIT2;
}

void BT_Working()
{
    P8OUT &= ~BIT2;
}


void BT_UartInit()
{
    P4SEL |= BIT4+BIT5;        
    BT_Uart_Config();
}

#pragma vector=USCI_A1_VECTOR
__interrupt void BT_Uart_ISR(void)
{
    //_BIC_SR_IRQ(LPM3_bits);   // 唤醒
    //LPM3_EXIT; 
    
    
    //while (!(UCA0IFG&UCTXIFG));

    //if (BTRecLen < BT_BUF_LEN)
    //{
    BTRecvBuf[BTRecLen++] = UCA1RXBUF;
    //}
    
    
    //UCA0TXBUF=UCA0RXBUF;   // 回显
    
    if (BTRecLen >= BT_BUF_LEN) 
    {
        Clear_BT_Buf();
    }
}


void Clear_BT_Buf()
{
    memset(BTRecvBuf,0,BT_BUF_LEN);
    BTRecLen = 0;
}


void BT_Uart_Config()
{
    UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
   
    #if 1
    // 用ACLK (32k) 做串口的时钟
    // 防止唤醒的时候时钟不稳导致收到乱码
    UCA1CTL1 |= UCSSEL__ACLK;
    UCA1BR0 = 3;                                // 查表获得  固定波特率9600
    UCA1BR1 = 0;                                // UCA0BRX和UCA0MCTL数值  
    UCA1MCTL |= UCBRS_3 + UCBRF_0;              //  
    
    #else
    
    unsigned long  real_baudrate = 9600;
    //unsigned long  SMCLK_Freq = System_Freq;
    unsigned int BRx=0;
    char BRFx=0;
    float tmp;
    int TmpValue;

    #if 0
    if((BaudRate>=UART_MIN_BAUDRATE)  && (BaudRate<=UART_MAX_BAUDRATE))
    {
        real_baudrate=BaudRate;
    }           
    else
    {
        real_baudrate=UART_MIN_BAUDRATE;   
    }
    #endif

    
    
    UCA1CTL1 |= UCSSEL_2;                     // SMCLK
    tmp=System_Freq/(float)real_baudrate/16.0;
    BRx=(unsigned int)tmp;
    tmp=(tmp-BRx)*16;
    TmpValue=(unsigned int)(tmp*10);
    TmpValue%=10; 
    BRFx=(unsigned int)tmp;
    if(TmpValue>=5) BRFx++;

    UCA1BR0=BRx&0xff;
    UCA1BR1=BRx>>8;     
    UCA1MCTL=(BRFx<<4)+UCOS16;
    #endif

    UCA1CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}



void BT_Uart_SendCmd()
{
    int i;

    i = 0;
    while(AtCmdBuf[i] != 0)
    {
        while(!(UCA1IFG & UCTXIFG)); //query tx ready?
        UCA1TXBUF=AtCmdBuf[i];
        i++;
    }
}

void BT_Uart_SendData(BYTE *Data, BYTE Len)
{
    BYTE i;
    BT_WakeUp();
    for (i=0;i<Len;i++)
    {
        while(!(UCA1IFG & UCTXIFG)); //query tx ready?
        UCA1TXBUF=Data[i];
    }

    //BT_Sleep();
}



/*************************************************/
/*             BT  链路层                           */
/*************************************************/
// 检测收到的数据中是否有需要的数据
// 返回首位置0xFFFF 为没有找到
WORD BTFindString(char *Str)
{
    WORD i=0, j=0, ret=0;

search:    
    // 搜索首字母
    j = 0;
    while(1)                
    {
        if(Str[j] == BTRecvBuf[i])  
        {
            ret = i;
            break;                            //如果收到字符串首字节，则退出循环
        }
        
        i++;
        if(i == BT_BUF_LEN)
        {
            return 0xFFFF;                   //如果未收到字符串首字节
        }
    }

    // 继续搜索
    i++;
    if(i == BT_BUF_LEN) 
    {
        return 0xFFFF;
    }

    j++;
    if (Str[j] == '\0')
    {
        return ret;
    }
    while(1)                          
    {                                             
        if(BTRecvBuf[i] != Str[j])              
        {
            //字符串一旦与缓冲区中数据不匹配，则继续搜索
        	goto search;
        }
        
        j++;
        if (Str[j] == '\0')
        {
            break;
        }
        
        i++;
        if(i == BT_BUF_LEN) 
        {
            return 0xFFFF;
        }
    }
    return ret;                                             //搜索成功
}



BYTE BTWaitAtCmdRet()
{
    static BYTE tm = 10;
    WORD loc;

    loc = 0xFF;
    tm = 10;
    while(tm--)
    {
        Sleep(100);
        loc = BTFindString("OK");
        if (loc != 0xFFFF)
        {
            return loc;
        }
    }
    return 0xFF; // TimeOut
}


BOOL BT_Send_AtCmd()
{
    BT_Uart_SendCmd();
    Clear_BT_Buf();
    if (0xFF == BTWaitAtCmdRet())
    {
        Clear_BT_Buf();
        return FALSE;
    }
    
    return TRUE;
}

BOOL BT_WaitOpen()
{
    BYTE tm;
    WORD loc;

    loc = 0xFF;
    tm = 50;
    while(tm--)
    {
        Sleep(100);
        loc = BTFindString("OPEN");
        if (loc != 0xFFFF)
        {
            return TRUE;
        }
    }
    return FALSE; // TimeOut
}



// 测试命令
BOOL BT_AtCmd_Test()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT\r\n");
    if (BT_Send_AtCmd())
    {
        Clear_BT_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// 主从模式
//1: 主设备0: 从设备
BOOL BT_AtCmd_Role(BYTE Mode)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+ROLE=%d\r\n",Mode);
    if (BT_Send_AtCmd())
    {
        Clear_BT_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 

// 低功耗
//0：禁止低功耗
//1：允许低功耗
BOOL BT_AtCmd_LowPw(BYTE Mode)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+LOWPOWER=%d\r\n",Mode);
    if (BT_Send_AtCmd())
    {
        Clear_BT_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 

BOOL BT_AtCmd_GetAddr()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+RADDR?\r\n");
    if (BT_Send_AtCmd())
    {
        //Clear_BT_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL BT_AtCmd_SetAddr(char *Addr)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+RADDR=%s\r\n",Addr);
    if (BT_Send_AtCmd())
    {
        Clear_BT_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL BT_AtCmd_Bind(BYTE Enable)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+BIND=%d\r\n",Enable);
    if (BT_Send_AtCmd())
    {
        Clear_BT_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL BT_AtCmd_Right(BYTE Enable)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+AUTH=%d\r\n",Enable);
    if (BT_Send_AtCmd())
    {
        Clear_BT_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL BT_AtCmd_SetPw(BYTE Pw)
{
    BYTE p = Pw;
    if ((Pw > 7) || (Pw == 0))
    {
        p = 7;
    }
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+TXPOWER=%d\r\n",p);
    if (BT_Send_AtCmd())
    {
        Clear_BT_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 
BOOL BT_ATCmd_ClearDev()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CLEARADDR\r\n");
    if (BT_Send_AtCmd())
    {
        Clear_BT_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


BOOL BT_Init()
{
    //BYTE tm = 10;
    BT_WakeUp();
    Sleep(500);
    BT_Setting();

    if (!BT_WaitOpen())
    {
        StorageLog(ERR_BT_WAIT);
        return FALSE;
    }
    
    if (!BT_AtCmd_Test())
    {
        StorageLog(ERR_BT_ATTEST);
        return FALSE;
    }

    if (!BT_AtCmd_SetPw(InnerParam.BtPower))
    {
        StorageLog(ERR_BT_SETPW);
        return FALSE;
    }

    if (!BT_AtCmd_Role(1))
    {
        StorageLog(ERR_BT_ROLE);
        return FALSE;
    }

    if (!BT_ATCmd_ClearDev())
    {
        StorageLog(ERR_BT_CLEAR);
        return FALSE;
    }

    #if 1
    if (!BT_AtCmd_Right(1))
    {
        StorageLog(ERR_BT_RIGHT);
        return FALSE;
    }

    if (!BT_AtCmd_Bind(1))
    {
        StorageLog(ERR_BT_BIND);
        return FALSE;
    }
    #endif

    //BT_AtCmd_GetAddr();
    
    if (!BT_AtCmd_SetAddr(InnerParam.BtAddress)) //("001B3510D7D7"))
    {
        StorageLog(ERR_BT_SETADD);
        return FALSE;
    }

    

    #if 0
    while(tm--)
    {
        Sleep(200);
        if (BT_AtCmd_GetAddr())
        {
            if (BTFindString("001B3510D7D4"))
            {
                Clear_BT_Buf();
                break;
            }
        }
    }

    if (tm == 0)
    {
        return FALSE;
    }
    #endif


    if (!BT_AtCmd_LowPw(1))
    {
        StorageLog(ERR_BT_LOWPW);
        return FALSE;
    }

    BT_Working();

    Sleep(1000);
    //BT_Sleep();
    Sleep(1000);

    return TRUE;
}

BOOL BT_Test()
{
    BT_Uart_SendData("1234567890",10);
    return TRUE;
}

/********蓝牙协议层*************************/
void BTSendCmd(BYTE Cmd, BYTE Addr, WORD Len)
{
    WORD Crc;
    //memset(BTSendBuf, 0, BT_BUF_LEN);
    BTSendBuf[0] = 0x68;
    BTSendBuf[1] = Cmd;
    memcpy(&BTSendBuf[2], &Len, 2);

    Crc = CRC16(&BTSendBuf[0],Len+4);
    BTSendBuf[Len+4] = (BYTE)Crc;
    BTSendBuf[Len+5] = (BYTE)(Crc>>8);
    BTSendBuf[Len+6] = 0x16;
    BT_Uart_SendData(BTSendBuf, Len+7);
}


BOOL BTValidFrame()
{
    WORD Len,Crc;

    if (BTRecLen < 7)
    {
        BTErrCode = 1;
        return FALSE;
    }
    
    if (BTRecvBuf[0] != 0x68)
    {
        //Clear_BT_Buf();
        BTErrCode = 2;
        return FALSE;
    }

    memcpy(&Len, &BTRecvBuf[2], 2);
    if (BTRecvBuf[Len+6] != 0x16)
    {
        //Clear_BT_Buf();
        BTErrCode = 3;
        return FALSE;
    }

    memcpy(&Crc, &BTRecvBuf[Len+4], 2);

    if (Crc != CRC16(BTRecvBuf, Len+4))
    {
        Clear_BT_Buf();
        BTErrCode = 4;
        return FALSE;
    }
    
    return TRUE;
}

BOOL BTWaitCmd(BYTE Cmd)
{
    BYTE tm;

    Clear_BT_Buf();

    tm = 3;
    while(tm--)
    {
        Sleep(100);
        if (BTValidFrame())
        {
            if (Cmd == BTRecvBuf[1])
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return FALSE;
}


/********蓝牙应用层*************************/
void SetBtPower()
{
    
}



BYTE BtAddrIndex = 0;
BYTE BtAddrTmp[13] = {0};
BYTE BtPwTmp = 0x37;
void ShowBtAddress()
{
    BYTE i;
    char tmp[2];

    Clear_DispBuf();
    ShowMeteTitle();

    Show12X12String(0,20,"蓝牙功耗:",0);
    tmp[0] = BtPwTmp;
    tmp[1] = 0;
    GT_Show8X16Ascii(65,20,tmp,BtAddrIndex == 0);
    
    Show12X12String(0,40,"蓝牙设备地址:",0);
    
    for (i=0;i<12;i++)
    {
        tmp[0] = BtAddrTmp[i];
        tmp[1] = 0;
        GT_Show8X16Ascii(i*8,60, tmp, (i+1) == BtAddrIndex);
    }

    Show12X12String(100,80,"保存",BtAddrIndex == 13);

    ShowOptHint();
    DisplayRefresh();
}

void BlueToothSetUI()
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        #ifdef NO_BT_WIFI 
        ShowHint("无蓝牙接口",InterParamSetUI);
        #else
        ShowHint("蓝牙禁用",InterParamSetUI);
        #endif
        return;
    }

    if (CheckRes.WifiSt == FALSE)
    {
        #ifdef NO_BT_WIFI 
        ShowHint("无蓝牙接口",InterParamSetUI);
        #else
        ShowHint("蓝牙异常",InterParamSetUI);
        #endif
        return;
    }
    
    currUI = BLUETOOTHSET;
    BtAddrIndex = 0;
    memcpy(BtAddrTmp,InnerParam.BtAddress,13);
    BtPwTmp = InnerParam.BtPower + 0x30;
    ShowBtAddress();
}

void BtPwAdd()
{
    if (BtPwTmp < '7')
    {
        BtPwTmp ++;
    }
    else
    {
        BtPwTmp = '1';
    }
}

void BtPwDec()
{
    if (BtPwTmp > '1')
    {
        BtPwTmp --;
    }
    else
    {
        BtPwTmp = '7';
    }
}

void BtAddAdd()
{
    if (BtAddrTmp[BtAddrIndex-1] < '9')
    {
        BtAddrTmp[BtAddrIndex-1] ++;
    }
    else
    {
        if (BtAddrTmp[BtAddrIndex-1] == '9')
        {
            BtAddrTmp[BtAddrIndex-1] = 'A';
        }
        else
        {
            if (BtAddrTmp[BtAddrIndex-1] < 'F')
            {
                BtAddrTmp[BtAddrIndex-1] ++;
            }
            else
            {
                BtAddrTmp[BtAddrIndex-1] = '0';
            }
        }
    }
}

void BtAddDec()
{
    if (BtAddrTmp[BtAddrIndex-1] > 'A')
    {
        BtAddrTmp[BtAddrIndex-1] --;
    }
    else
    {
        if (BtAddrTmp[BtAddrIndex-1] == 'A')
        {
            BtAddrTmp[BtAddrIndex-1] = '9';
        }
        else
        {
            if (BtAddrTmp[BtAddrIndex-1] > '0')
            {
                BtAddrTmp[BtAddrIndex-1] --;
            }
            else
            {
                BtAddrTmp[BtAddrIndex-1] = 'F';
            }
        }
    }
}

void BTSetKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT:  case POWER: EnterMeasuInter();  break;
        case RETURN:   InterParamSetUI();    break;
        
        case OKKEY:
        {
            if (BtAddrIndex == 0)
            {
                SetBtPower();
            }
            else if (BtAddrIndex == 13)
            {
                PopHint("正在设置,请稍候...");
                
                if (CheckRes.CommInterface == COMM_BT)
                {
                    BTSenserPower(0);
                }
                
                memcpy(InnerParam.BtAddress,BtAddrTmp,13);
                InnerParam.BtPower = BtPwTmp-0x30;
                if (BT_Init())
                {
                    ShowHint("蓝牙设备地址设置成功",InterParamSetUI);
                    //InterParamSetUI();
                    SaveInnerParam();
                }
                else
                {
                    ShowHint("蓝牙设备地址设置失败",BlueToothSetUI);
                }
            }
        }
        break;
        
        case MODE:
        {
            BtAddrIndex ++;
            if (BtAddrIndex > 13)
            {
                BtAddrIndex = 0;
            }
            ShowBtAddress();
        }
        break;
        
        case DOWN:
        {   
            if (BtAddrIndex == 0)
            {
                BtPwDec();
            }
            else
            {
                BtAddDec();
            }
            ShowBtAddress();
        }
        break;
        
        case UP:
        {
            if (BtAddrIndex == 0)
            {
                BtPwAdd();
            }
            else
            {
                BtAddAdd();
            }
            ShowBtAddress();
        }
        break;
    }

}


