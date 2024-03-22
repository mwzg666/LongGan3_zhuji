#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <msp430.h>
#include "system.h"
#include "CommDataDefine.h"
#include "oled.h"
#include "Flash.h"
#include "Measinter.h"
#include "DataStore.h"

#include "Senser.h"
#include "Uart.h"
#include "Ds1337.h"

#ifdef UART_DOSE

extern QStorageData RunData;
extern CMPTIME CmpTime;
extern QInnerParam InnerParam;
extern SENSER Senser;
extern DS1337_QTIME curtime;
extern char * const UnitStr[];
extern QPara Param;
extern unsigned char ucVoltPer;

unsigned char RecvBuf = 0;
unsigned char SendBuf[UART_BUF_LEN] = {0};



int chInde = 0;

#pragma vector=USCI_A0_VECTOR
__interrupt void Uart_ISR(void)
{
    //_BIC_SR_IRQ(LPM3_bits);   // 唤醒
    //LPM3_EXIT; 
    
    
    //while (!(UCA0IFG&UCTXIFG));


    RecvBuf = UCA0RXBUF;
    chInde++;

    
}


void Clear_Uart_Buf()
{
    //memset(RecvBuf,0,UART_BUF_LEN);
    chInde = 0;
}


void JumpUpload()
{
    //unsigned int RemoteAddr = 0x1133;
    Clear_DispBuf();
    
    GT_Show12X12Char(38,26,"程序升级",0);
    DisplayRefresh();
    Erase_Data_Per_Seg((char*)FLASH_UPDATE_ADDR);
    asm(" mov &0xFA00, PC;");
    
    //Write_Data_Seg((unsigned char*)REMOTE_SAVE_ADDR2,(unsigned char*)&RemoteAddr,2);
    //asm(" mov 0xFF70, r6; mov @r6, pc");
    //while(1);
    //unsigned char ucRemoteAddr[2] = "\0";
    //Flash_Read((unsigned char*)REMOTE_SAVE_ADDR,ucRemoteAddr,2);
    //RemoteAddr = ucRemoteAddr[1]*256 + ucRemoteAddr[0];
    //fptr = (pfun)62210;
    //fptr();
}


void Uart_Config(unsigned long  SMCLK_Freq,unsigned long  BaudRate)
{
    UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
   
    #if 1
    // 用ACLK (32k) 做串口的时钟
    // 防止唤醒的时候时钟不稳导致收到乱码
    UCA0CTL1 |= UCSSEL__ACLK;
    UCA0BR0 = 3;                                // 查表获得  固定波特率9600
    UCA0BR1 = 1;                                // UCA0BRX和UCA0MCTL数值  
    UCA0MCTL |= UCBRS_3 + UCBRF_0;              //  
    
    #else
    
    volatile unsigned long  real_baudrate;
    unsigned int BRx=0;
    char BRFx=0;
    float tmp;
    int TmpValue;

    if((BaudRate>=UART_MIN_BAUDRATE)  && (BaudRate<=UART_MAX_BAUDRATE))
    {
        real_baudrate=BaudRate;
    }           
    else
    {
        real_baudrate=UART_MIN_BAUDRATE;   
    }
    
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    tmp=SMCLK_Freq/(float)real_baudrate/16.0;
    BRx=(unsigned int)tmp;
    tmp=(tmp-BRx)*16;
    TmpValue=(unsigned int)(tmp*10);
    TmpValue%=10; 
    BRFx=(unsigned int)tmp;
    if(TmpValue>=5) BRFx++;

    UCA0BR0=BRx&0xff;
    UCA0BR1=BRx>>8;     
    UCA0MCTL=(BRFx<<4)+UCOS16;
    #endif

    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
}


void InitUart(void)
{
    UART_ON;
    
    P3SEL |= BIT3+BIT4;                       // P3.3,4 = USCI_A0 TXD/RXD
    //Uart_Config(System_Freq,UART_USE_BAUDRATE);
    Uart_Config(32768,UART_USE_BAUDRATE);
}


void Uart_Send_Data(BYTE *Data, int Len)
{
    static int i;
    
    for(i=0;i<Len;i++)
    {
        while(!(UCA0IFG & UCTXIFG)); //query tx ready?
        UCA0TXBUF=Data[i];
    }
}

void PrintData(WORD id, QStorageData *data)
{
    float val1; // , val2;
    BYTE u;

    if(data->ucCheck != GetVerfity((unsigned char*)data,sizeof(QStorageData)))
    {
        return;
    }
    
    memset(&CmpTime,0,sizeof(CMPTIME));
    memset(SendBuf,0,UART_BUF_LEN);
    
    CmpTime.Time = data->Time;
    val1 = GetVal1(data);
    u = GetInUnit1(data);
    //val2 = GetVal2(data);
    
    sprintf((char *)SendBuf,"%d. %02d-%02d %02d:%02d:%02d %.2f %s\r\n",
                       id, CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t,CmpTime.s,
                       val1,UnitStr[u]);

    Uart_Send_Data(SendBuf,strlen((char *)SendBuf));
}

BYTE GetPrintAlarm(BYTE id)
{
    switch(id)
    {
        case ALARM_INO: 
        case ALARM_AO: 
        case ALARM_BO: 
        case ALARM_ABO: 
        case ALARM_PDO: 
        case ALARM_GMO:  
        case ALARM_GDO:
        case ALARM_YO:  return 0xFF;

        
        case ALARM_IN1: 
        case ALARM_A1: 
        case ALARM_B1: 
        case ALARM_AB1: 
        case ALARM_PD1: 
        case ALARM_GM1:  
        case ALARM_GD1:
        case ALARM_Y1:  return 1;

        
        case ALARM_IN2: 
        case ALARM_A2: 
        case ALARM_B2: 
        case ALARM_AB2: 
        case ALARM_PD2: 
        case ALARM_GM2:  
        case ALARM_GD2:
        case ALARM_Y2:  return 2;
        
        case ALARM_IN3: return 3;
        
        case ALARM_YP:
        case ALARM_YL: return 4;
        
        case ALARM_YT:  
        case ALARM_YH: return 5;
    }

    return 0;
}

// 打印实时计数

// 获取当前报警ID , 如果同时有多个报警
// 则轮流返回
BYTE GetCurAlarm()
{
    BYTE c=0,ret;
    static BYTE cur=0;   // 当前返回的第几个报警

    if (!HaveAlarm())
    {
        cur = 0;
        return 0xFF;   // 无报警
    }

    c = AlarmCnt();
    cur ++;
    if (cur > c)
    {
        cur = 1;
    }

    ret = GetAlarmIndex(cur);
    
    return ret;
}



void PrintCounter(float Cnt, BYTE Unit)
{
    static BYTE t=0;
    BYTE Alarm=0,k;
    
    if (t++ >= Param.MeteTime)
    {
        t = 0; 
    }

    GetTimer(&curtime);

    if (HaveAlarm())
    {
        k = GetCurAlarm();
        Alarm = GetPrintAlarm(k);   
    }
    //GetAlarm(BYTE Alarm);
    
    memset(&CmpTime,0,sizeof(CMPTIME));
    memset(SendBuf,0,UART_BUF_LEN);
    
    sprintf((char *)SendBuf,"%d. %04d-%02d-%02d %02d:%02d:%02d %.2f%s %02d %d%%\r\n",
                       t, curtime.year,curtime.month,curtime.day,
                          curtime.hour,curtime.minute,curtime.second,
                       Cnt,UnitStr[Unit],Alarm,ucVoltPer);

    Uart_Send_Data(SendBuf,strlen((char *)SendBuf));
}



void PrintAllData()
{
    WORD i,j;

    if (InnerParam.StorageNum == 0)
    {
        Uart_Send_Data("No data.\r\n",10);
        return;
    }
    
    
    for (i=0; i<InnerParam.StorageNum; i++)
    {        
        j = i%DATA_COUNT;
        
        memset(&RunData,0,sizeof(QStorageData));
        GetOneData(j,&RunData);
        
        PrintData(i+1, &RunData);
    }
}



void Handle_Uart_Rec()
{
    if (chInde == 0)
    {
        return;
    }

    if (RecvBuf == 0x0D)
    {
        //Clear_Uart_Buf();
        PrintAllData();
    }

    chInde = 0;
    RecvBuf = 0;
    //Clear_Uart_Buf();
}

void PrintFloat(float val)
{
    memset(SendBuf,0,UART_BUF_LEN);
    sprintf((char *)SendBuf,"%0.2f\r\n",val);
    Uart_Send_Data(SendBuf,strlen((char *)SendBuf));
}

void PrintDword(DWORD val)
{
    memset(SendBuf,0,UART_BUF_LEN);
    sprintf((char *)SendBuf,"%lu\r\n",val);
    Uart_Send_Data(SendBuf,strlen((char *)SendBuf));
}

#endif

