#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <msp430.h>
#include "system.h"
#include "CommDataDefine.h"

#include "Main.h"
#include "MainMenu.h"
#include "Measinter.h"
#include "Keyboard.h"
#include "oled.h"
#include "Input.h"


#include "Wifi.h"

#ifdef USE_WIFI

WORD FindString(char *Str);


extern DWORD System_Freq;

/*************************************************/
/*             Wifi  �����                        */
/*************************************************/
#define WIFI_BUF_LEN 256
BYTE WifiRecvBuf[WIFI_BUF_LEN];
WORD WifiRecLen = 0;

#define AT_CMD_LEN 128
BYTE AtCmdBuf[AT_CMD_LEN];

void Wifi_Uart_Config();


void Wifi_IoInit()
{
    P8DIR |= BIT0 | BIT1 | BIT2; //Wifi_Rst  Wifi_Ch_Pd  Wifi_Update
}


void Wifi_Mode(BYTE Md)
{
    if (Md == 1)   // Flash Boot
    {
        P8OUT |= BIT2;
    }
    else          // UART download
    {
        P8OUT &= ~BIT2;
    }
}

void Wifi_Rst(BYTE lv)
{
    if (lv == 1)    //�ߵ�ƽ����
    {
        P8OUT |= BIT0;
    }
    else           // �͵�ƽ��λ
    {
        P8OUT &= ~BIT0;
    }
}


void Wifi_Reset()
{
    WIFI_ON;
    Sleep(100);
    
    Wifi_Rst(0);   
    Sleep(500);
    Wifi_Rst(1);

    
    Wifi_Mode(1);
}

void Wifi_ShutDown()
{
    WIFI_OFF;
    Wifi_Rst(0);   
}

void Wifi_UartInit()
{
    P4SEL |= BIT4+BIT5;        
    Wifi_Uart_Config();
}

#pragma vector=USCI_A1_VECTOR
__interrupt void Wifi_Uart_ISR(void)
{
    //_BIC_SR_IRQ(LPM3_bits);   // ����
    //LPM3_EXIT; 
    
    
    //while (!(UCA0IFG&UCTXIFG));


    WifiRecvBuf[WifiRecLen++] = UCA1RXBUF;

    //UCA0TXBUF=UCA0RXBUF;   // ����
    
    if (WifiRecLen >= WIFI_BUF_LEN) 
    {
        WifiRecLen = 0;
    }
}


void Clear_Wifi_Buf()
{
    memset(WifiRecvBuf,0,WIFI_BUF_LEN);
    WifiRecLen = 0;
}


void Wifi_Uart_Config()
{
    UCA1CTL1 |= UCSWRST;                      // **Put state machine in reset**
   
    #if 0
    // ��ACLK (32k) �����ڵ�ʱ��
    // ��ֹ���ѵ�ʱ��ʱ�Ӳ��ȵ����յ�����
    UCA1CTL1 |= UCSSEL__ACLK;
    UCA1BR0 = 3;                                // �����  �̶�������9600
    UCA1BR1 = 0;                                // UCA0BRX��UCA0MCTL��ֵ  
    UCA1MCTL |= UCBRS_3 + UCBRF_0;              //  
    
    #else
    
    unsigned long  real_baudrate = 115200;
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



void Wifi_Uart_SendCmd()
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

void Wifi_Uart_SendData(BYTE *Data, BYTE Len)
{
    int i;

    i = 0;
    for (i=0;i<Len;i++)
    {
        while(!(UCA1IFG & UCTXIFG)); //query tx ready?
        UCA1TXBUF=Data[i];
        i++;
    }
}




/*************************************************/
/*             Wifi  ��·��                                                        */
/*************************************************/
BYTE FindAtCmdOk()
{
    BYTE i;
    
    if (WifiRecLen < 2)
    {
        return 0xFF;
    }

    for (i=0;i<WifiRecLen;i++)
    {
        if ((WifiRecvBuf[i] == 'O') && (WifiRecvBuf[i+1] == 'K'))
        {
            return i;
        }
    }

    return 0xFF;
}


BYTE WaitAtCmdRet()
{
    static BYTE tm = 10;
    WORD loc;

    loc = 0xFF;
    tm = 10;
    while(tm--)
    {
        Sleep(100);
        //loc = FindAtCmdOk();
        loc = FindString("OK");
        if (loc != 0xFFFF)
        {
            return loc;
        }
    }
    return 0xFF; // TimeOut
}



BYTE FindAtCmdAck()
{
    BYTE i;
    
    if (WifiRecLen < 1)
    {
        return 0xFF;
    }

    for (i=0;i<WifiRecLen;i++)
    {
        if (WifiRecvBuf[i] == '>')
        {
            return i;
        }
    }

    return 0xFF;
}


BOOL WaitDataSendAck()
{
    static BYTE tm = 10;
    BYTE loc;

    loc = 0xFF;
    tm = 10;
    while(tm--)
    {
        Sleep(100);
        loc = FindAtCmdAck();
        if (loc != 0xFF)
        {
            //Clear_Wifi_Buf();
            return TRUE;
        }
    }
    
    //Clear_Wifi_Buf();
    return FALSE; // TimeOut
}


BOOL Wifi_Send_AtCmd()
{
    Wifi_Uart_SendCmd();
    if (0xFF == WaitAtCmdRet())
    {
        //Clear_Wifi_Buf();
        return FALSE;
    }
    
    return TRUE;
}


// ��������
BOOL Wifi_AtCmd_Test()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT\r\n");
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


// Wifi Reset
BOOL Wifi_AtCmd_Rst()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+RST\r\n");
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 


// Wifi Ӧ��ģʽ:   
//  1. Stationģʽ 2. APģʽ 3. AP+Stationģʽ
BOOL Wifi_AtCmd_CwMode(BYTE Mode)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    //sprintf((char *)AtCmdBuf,"AT+CWMODE_CUR=%d\r\n",Mode);
    sprintf((char *)AtCmdBuf,"AT+CWMODE_CUR=3\r\n");
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 

// �г����ý����
BOOL Wifi_AtCmd_CwLap()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CWLAP\r\n");
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 


// ��������
BOOL Wifi_AtCmd_CwJap(char *Ssid, char *Password)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CWJAP=%s,%s\r\n",Ssid,Password);
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 

// �˳������
BOOL Wifi_AtCmd_CwQap()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CWQAP\r\n");
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 


// ����AP ģʽ�µĲ���
BOOL Wifi_AtCmd_CwSap(BYTE ecn, char *ssid, char *password, BYTE chl)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CWSAP=%d,%s,%s,%d\r\n",ecn,ssid,password,chl);
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 


// ����Tcp/Udp ����
BOOL Wifi_AtCmd_CipStart(char *add, char *port)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CIPSTART=TCP,%s,%s\r\n",add,port);
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 


// ��ȡTCP/UDP ����״̬
BOOL Wifi_AtCmd_CipStatus()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CIPSTATUS\r\n");
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 


// ��������
BOOL Wifi_AtCmd_CipSend(BYTE *Data, BYTE Len)
{
    // ��������
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CIPSEND=%d\r\n",Len);
    Wifi_Uart_SendCmd();
    if (!WaitDataSendAck())
    {
        Clear_Wifi_Buf();
        return FALSE;
    }

    // ��������
    Clear_Wifi_Buf();
    
    Wifi_Uart_SendData(Data,Len);
    if (0xFF == WaitAtCmdRet())
    {
        Clear_Wifi_Buf();
        return FALSE;
    }

    return TRUE;
} 

// �ر�TCP/UDP ����
BOOL Wifi_AtCmd_CipClose()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CIPCLOSE=0\r\n");
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 


// ��ȡ����IP��ַ
void GetIpAddress();


// ��ѯ����IP ��ַ
BOOL Wifi_AtCmd_CiFsr()
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CIFSR\r\n");
    if (Wifi_Send_AtCmd())
    {
        GetIpAddress();
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 

// ����������
// 0: ������ģʽ  1:������ģʽ
BOOL Wifi_AtCmd_CipMux(BYTE Mux)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CIPMUX=%d\r\n",Mux);
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 

// Tcp/Ip Ӧ��ģʽ
// 0: ��͸�� 1:͸��ģʽ
BOOL Wifi_AtCmd_CipMode(BYTE Mode)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CIPMODE=%d\r\n",Mode);
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 


// �����������Ͽ���ʱʱ��0 -- 28800s
BOOL Wifi_AtCmd_CipSto(WORD Time)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CIPSTO=%d\r\n",Time);
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
} 


BOOL Wifi_AtCmd_CipServer(BYTE Mode, WORD Port)
{
    memset(AtCmdBuf,0,AT_CMD_LEN);
    sprintf((char *)AtCmdBuf,"AT+CIPSERVER=%d,%d\r\n",Mode,Port);
    if (Wifi_Send_AtCmd())
    {
        Clear_Wifi_Buf();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



/*************************************************/
/*             Wifi  ��·�㣬AT �������                 */
/*************************************************/
// ����յ����������Ƿ�����Ҫ������
// ������λ��0xFFFF Ϊû���ҵ�
WORD FindString(char *Str)
{
    WORD i=0, j=0, ret=0;

search:    
    // ��������ĸ
    j = 0;
    while(1)                
    {
        if(Str[j] == WifiRecvBuf[i])  
        {
            ret = i;
            break;                            //����յ��ַ������ֽڣ����˳�ѭ��
        }
        
        i++;
        if(i == WIFI_BUF_LEN)
        {
            return 0xFFFF;                   //���δ�յ��ַ������ֽ�
        }
    }

    // ��������
    i++;
    if(i == WIFI_BUF_LEN) 
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
        if(WifiRecvBuf[i] != Str[j])              
        {
            //�ַ���һ���뻺���������ݲ�ƥ�䣬���������
            goto search;
        }
        
        j++;
        if (Str[j] == '\0')
        {
            break;
        }
        
        i++;
        if(i == WIFI_BUF_LEN) 
        {
            return 0xFFFF;
        }
    }
    return ret;                                             //�����ɹ�
}


void GetIpAddress()
{
    WORD i;
    i = FindString("AT+CIFSR");
    if (i != 0xFFFF)
    {
        return;
    }
}



/*************************************************/
/*             Wifi  �����                          */
/*************************************************/
// ����·����



/*************************************************/
/*             Wifi  Ӧ�ò�                          */
/*************************************************/




/*************************************************/
/*             Wifi  �û���(����)                      */
/*************************************************/
extern INTERFACE currUI;
extern QPara Param;
extern char StrTemp[24];

void WifiParamDef()
{
    Param.Wifi.Enable = 0;
    Param.Wifi.Mode = WIFI_STATION;
    memset(Param.Wifi.SSID,0,32);
    memcpy(Param.Wifi.SSID,"MTS",3);
    memset(Param.Wifi.PassWord,0,32);
    memcpy(Param.Wifi.PassWord,"12345678",8);
    //memset(Param.Wifi.SrvIp,0,4);
    Param.Wifi.SrvIp[0] = 192;
    Param.Wifi.SrvIp[1] = 168;
    Param.Wifi.SrvIp[2] = 1;
    Param.Wifi.SrvIp[3] = 1;
    Param.Wifi.Port = 7000;
}


// wifi �������˵�
BYTE WifiMainMenuCnt = 5;
BYTE WifiMenuIndex = 1;
WIFIPARAM WifiParam;

void WifiStatusSet();
void WifiModeSet();
void WifiApSet();
void WifiSrvSet();
void WifiDetail();
void WifiParamConform();


void WifiManMenu();
void WifiManUI();
void WifiMultiFun();

BOOL WifiParamChanged()
{
    if (memcmp(&WifiParam,&Param.Wifi,sizeof(WIFIPARAM)) == 0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void EnterWifiMan()
{
    WifiMenuIndex = 1;
    memcpy(&WifiParam, &Param.Wifi, sizeof(WIFIPARAM));
    WifiManUI();
}

void WifiManUI()
{
    currUI = WIFIMAN;
    WifiManMenu();
}

void WifiManMenu()
{
    Clear_DispBuf();
    ShowMeteTitle();
    
    GT_Show16X16Char(16,16,"�����������",0);  
    Show12X12String(20,36,"״̬: ",0);
    
    if (WifiParam.Enable)
    {
        Show12X12String(60,36,"����",WifiMenuIndex == 1);

        Show12X12String(20,52,"ģʽ: ",0);
        if (WifiParam.Mode == WIFI_STATION)
        {
            Show12X12String(60,52,"�ͻ���",WifiMenuIndex == 2);
        }
        else
        {
            Show12X12String(60,52,"�����",WifiMenuIndex == 2);
        }

        Show12X12String(20,68,"���ý����", WifiMenuIndex == 3);
        Show12X12String(20,84,"���÷�����",WifiMenuIndex == 4);
        
        Show12X12String(20,100,"������������",WifiMenuIndex == 5);

        if (WifiParamChanged())
        {
            Show12X12String(100,116,"ȷ��",WifiMenuIndex == 6);
            WifiMainMenuCnt = 6;
        }
        else
        {
            WifiMainMenuCnt = 5;
        }
    }
    else
    {
        Show12X12String(60,36,"�ر�",WifiMenuIndex == 1);
        if (WifiParamChanged())
        {
            Show12X12String(100,52,"ȷ��",WifiMenuIndex == 2);
            WifiMainMenuCnt = 2;
        }
        else
        {
            WifiMainMenuCnt = 1;
        }
    }
    
    ShowOptHint();    
    DisplayRefresh();
}


void WifiManKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterMainMenu(); break;
        
        case MODE: break;
       
        case UP:
        {
            if(--WifiMenuIndex < 1)
            {
                WifiMenuIndex = WifiMainMenuCnt;
            }
            WifiManMenu();
        }
        break;
        
        case DOWN:
        {
            if(++WifiMenuIndex > WifiMainMenuCnt)
            {
                WifiMenuIndex = 1;
            }
            WifiManMenu();
        }
        break;
        
        case OKKEY:
        {
            switch (WifiMenuIndex)
            {
                case 1:  WifiStatusSet();  break;
                case 2:  WifiMultiFun();    break;
                case 3:  WifiApSet();       break;
                case 4:  WifiSrvSet();      break;
                case 5:  WifiDetail();      break;
                case 6:  WifiParamConform(); break;
            }
        }
        break;
    }
}


// �๦�ܲ˵�
void WifiMultiFun()
{
    if (WifiParam.Enable)
    {
        WifiModeSet();
    }
    else
    {
        WifiParamConform();
    }
}


// ����wifi ״̬
BYTE WifiStatusIndex = 1;
void WifiSatausSelect();
void WifiStatusSet()
{
    currUI = WIFISTATUS;
    WifiStatusIndex = WifiParam.Enable+1;
    WifiSatausSelect();
}

void WifiSatausSelect()
{
    PopWindow(60,32,34,36,"");
    Show12X12String(64,36,"�ر�", WifiStatusIndex == 1);
    Show12X12String(64,52,"����", WifiStatusIndex == 2);
    DisplayRefresh();
}



void WifiStatusKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER:  EnterMeasuInter(); break;
        case RETURN:  WifiManUI();  break;   
        
        case OKKEY:  
        {
            //if (WifiParam.Enable != WifiStatusIndex-1)
            {
                //WifiParamUpdated = TRUE;
                WifiParam.Enable = WifiStatusIndex-1;
            }
            WifiManUI();
            break;
        }

        case UP:
        {
            if(--WifiStatusIndex < 1)
            {
                WifiStatusIndex = 2;
            }
            WifiSatausSelect();
        }
        break;
        
        case DOWN:
        {
            if(++WifiStatusIndex > 2)
            {
                WifiStatusIndex = 1;
            }
            WifiSatausSelect();
        }
        break;
    }
}


// Wifi ģʽ����
BYTE WifiModeIndex = 1;
void WifiModeSelect();
void WifiModeSet()
{
    currUI = WIFIMODE;
    WifiModeIndex = WifiParam.Mode;
    WifiModeSelect();
}

void WifiModeSelect()
{
    PopWindow(60,48,48,36,"");
    Show12X12String(64,52,"�ͻ���", WifiModeIndex == 1);
    Show12X12String(64,68,"�����", WifiModeIndex == 2);
    DisplayRefresh();
}

void WifiModeKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER:  EnterMeasuInter(); break;
        case RETURN:  WifiManUI();  break;   
        
        case OKKEY:  
        {
            //if (WifiParam.Mode != WifiModeIndex)
            {
                WifiParam.Mode = WifiModeIndex;
                //WifiParamUpdated = TRUE;
            }
            WifiManUI();
            break;
        }

        case UP:
        {
            if(--WifiModeIndex < 1)
            {
                WifiModeIndex = 2;
            }
            WifiModeSelect();
        }
        break;
        
        case DOWN:
        {
            if(++WifiModeIndex > 2)
            {
                WifiModeIndex = 1;
            }
            WifiModeSelect();
        }
        break;
    }
}


// ���������
BYTE WifiApIndex = 1;
void ShowApSet();
void WifiApSet()
{
    currUI = WIFIAP;
    ShowApSet();
}

void ShowApSet()
{
    Clear_DispBuf();
    ShowMeteTitle();
    
    GT_Show16X16Char(16,16,"���������",0);  
    Show5X7String(8,36,"SSID:",0);
    Show5X7String(8,52,(char *)WifiParam.SSID,0);
    
    Show12X12String(8,76,"����:",0);
    Show5X7String(8,92,(char *)WifiParam.PassWord,0);

    ShowOptHint();    
    DisplayRefresh();
}

void WifiApKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER:  EnterMeasuInter(); break;
        case RETURN:  WifiManUI();  break;   
    }
}


// ����������
void ShowSrvSet();
void WifiSrvSet()
{
    currUI = WIFISRV;
    ShowSrvSet();
}

void ShowSrvSet()
{
    Clear_DispBuf();
    ShowMeteTitle();
    
    GT_Show16X16Char(16,16,"����������",0);  

    Show12X12String(8,36,"IP��ַ:",0);
    memset(StrTemp,0,24);
    sprintf(StrTemp,"%d.%d.%d.%d",WifiParam.SrvIp[0],WifiParam.SrvIp[1],
                                   WifiParam.SrvIp[2],WifiParam.SrvIp[3]);
    Show5X7String(8,52,StrTemp,0);
    
    Show12X12String(8,76,"�˿�:",0);
    memset(StrTemp,0,24);
    sprintf(StrTemp,"%d",WifiParam.Port);
    Show5X7String(8,92,StrTemp,0);
    
    ShowOptHint();    
    DisplayRefresh();
}

void WifiSrvKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER:  EnterMeasuInter(); break;
        case RETURN:  WifiManUI();  break;   
    }
}


// ����
void ShowWifiDetail();
void WifiDetail()
{
    currUI = WIFIDETAIL;
    ShowWifiDetail();
}

void ShowWifiDetail()
{
    Clear_DispBuf();
    ShowMeteTitle();
    
    GT_Show16X16Char(16,16,"������������",0);  

    Show12X12String(8,36,"״̬:",0);
    Show12X12String(8,52,"����IP��ַ:",0);
    Show12X12String(8,68,"MAC��ַ:",0);
    Show12X12String(8,84,"�ź�ǿ��",0);

    ShowOptHint();    
    DisplayRefresh();
}

void WifiDetailKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER:  EnterMeasuInter(); break;
        case RETURN:  case OKKEY:  WifiManUI();  break;   
    }
}


BOOL WifiAp()
{
    if (!Wifi_AtCmd_CwMode(WifiParam.Mode))
    {
        return FALSE;
    }

    if (!Wifi_AtCmd_CipMux(1))
    {
        return FALSE;
    }

    if (!Wifi_AtCmd_CiFsr())
    {
        return FALSE;
    }

    if (!Wifi_AtCmd_CipServer(1,WifiParam.Port))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL WifiClient()
{
    return TRUE;
}

BOOL InitWifi()
{
    BOOL ret = TRUE; 
    
    if (WifiParam.Enable)
    {
        Wifi_Reset();

        Sleep(1000);

        if (!Wifi_AtCmd_Rst())
        {
            return FALSE;
        }
    

        switch (WifiParam.Mode)
        {
            case 1: ret = WifiClient(); break;
            case 2: ret = WifiAp();     break;
        }        
    }
    else
    {
        Wifi_ShutDown();
    }

    return ret;
}


void WifiParamConform()
{
    PopHint("���������²���,���Ժ�...");
    //Sleep(3000);
    if (InitWifi())
    {
        memcpy(&Param.Wifi, &WifiParam, sizeof(WIFIPARAM));
        ShowHint("�²������óɹ�",InterMainMenu);
    }
    else
    {
        ShowHint("�²�������ʧ��",InterMainMenu);
    }
}

#endif

