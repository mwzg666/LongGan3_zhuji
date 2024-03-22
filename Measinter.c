#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <msp430x552x.h>
#include <math.h>

#include "system.h"
#include "CommDataDefine.h"

#include "Keyboard.h"
#include "MainMenu.h"
#include "oled.h"
#include "Measinter.h"
#include "QuickMenu.h"
#include "ds1337.h"
#include "flash.h"
#include "adc12.h"
#include "Pkur.h"
#include "main.h"
#include "Hvadc.h"
#include "Senser.h"
#include "Eeprom.h"
#include "SelfCheck.h"
#include "Input.h"
#include "Uart.h"
#include "RadioSet.h"
#include "DataStore.h"
#include "BlueTooth.h"
#include "DoseRate.h"
#include "Mcp2510.h"
#include "TMeas.h"

extern CHECKRES CheckRes;
extern SENSER Senser;
extern char * const NucName[];
extern SYSALARM SysAlarm;
extern char * const AlarmName[];
extern DWORD InSenserCnt;
extern char StrTemp[24];
extern QInnerParam InnerParam;
//测量界面
extern INTERFACE currUI;
extern BYTE  CurRadIndex;   // 正在使用核素的索引
extern QPara Param;
extern BYTE  RadCount;     // 核素数量
extern RADIO pnuc[25];      //核素
extern WORD  InSerserErrTimer;
extern DS1337_QTIME cuTime;
extern BOOL DebugMode;
extern BATTERY SenserBatery;
extern BOOL BkStatus;
extern BOOL bTimerMeas;
extern BYTE MeasTime;
extern BOOL InSenserEnable;

// 历史报警状态
static BOOL HadAlarm = FALSE;

// 探头切换状态
static BOOL SwSenser = FALSE;  // TRUE : 切换到内部探头


DS1337_QTIME curtime;
METEFACEINFO FaceInfo;
DATAINFO     DataInfo;
unsigned char ucVoltPer;
BYTE          RadMeasMode;   // 混合场测量模式：0:Mix 1:α 2：β

char * const UnitStr[] =
{
    "Err",
    "CPS",
    "Bq",
    "Bq/cm@2",
    #ifdef UNIT_GY
    "uGy/h",
    "mGy/h",
    "Gy/h",
    "uGy",
    "mGy",
    "Gy"
    #else
    "uSv/h",
    "mSv/h",
    "Sv/h",
    "uSv",
    "mSv",
    "Sv"
    #endif
};

#ifdef DATA_IN_INNER_FALSH

// 用于缓存存储的日志和log
__no_init unsigned char Data_Cache[512] @0x3E00;
//__no_init unsigned char Log_Cache[512] @0x3C00;

// 缓存测量数据
// 60 * 3 * 4 = 720

float OutA_Cache[60];  // -- 256字节
float OutB_Cache[60];
float In_Cache[60];
#else
__no_init float OutA_Cache[64] @0x3C00;  // -- 256字节
__no_init float OutB_Cache[64] @0x3D00;
__no_init float In_Cache[64]   @0x3E00;
#endif


void Init_Cache()
{
    #ifdef DATA_IN_INNER_FALSH
    memset(Data_Cache,0,512);
    //memset(Log_Cache,0,512);
    memset(&OutA_Cache,0,240);
    memset(&OutB_Cache,0,240);
    memset(&In_Cache,0,240);
    #else
    //memset(Log_Cache,0,512);
    memset(&OutA_Cache,0,256);
    memset(&OutB_Cache,0,256);
    memset(&In_Cache,0,256);
    #endif
}

#if 0
void ShowOverLoad()
{
    const unsigned char Overload[] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x01,0x80,0x06,0x38,0x00,0x0C,0x01,0x80,0x06,0x3F,0x00,0x06,0x01,
        0x80,0x06,0xFB,0x80,0x06,0x01,0xB0,0x3F,0xF8,0x00,0x01,0xFF,0xF8,0x06,0x38,0xC0,
        0x00,0x01,0x80,0xFF,0xFF,0x80,0x06,0x61,0x80,0x0C,0x18,0x00,0x3E,0x31,0x80,0x1C,
        0xFB,0x80,0x06,0x39,0x80,0x7F,0x9B,0x80,0x06,0x19,0x80,0x37,0x1B,0x00,0x06,0x01,
        0x80,0x76,0x1F,0x00,0x06,0x01,0x80,0x7F,0xFE,0x00,0x06,0x01,0x80,0x06,0x0E,0x00,
        0x06,0x0F,0x80,0x06,0xEE,0x60,0x0F,0x03,0x80,0x7F,0x9E,0x60,0x19,0xC0,0x00,0x66
    };

    Diplay(33,20,48,24,Overload,0);
}
#endif

#if 0
void ShowBqprecm()
{
    const unsigned char unitBqprecm[]={
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x04,
        0xF8,0x02,0x00,0x08,0x44,0x02,0x00,0x1C,0x44,0x04,0x00,0x00,0x78,0xE4,0xED,0xB2,
        0x45,0x25,0x26,0xD0,0x45,0x25,0x04,0x90,0x45,0x29,0x04,0x92,0xF8,0xE8,0xEE,0xD8,
        0x00,0x20,0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };

    Diplay(0,30,32,16,unitBqprecm,0);
}
#endif

void ShowBatPower()
{
    const unsigned char PowerValu[4][16] = {
        0x00,0x00,0x3F,0xF8,0x20,0x0E,0x20,0x0E,0x20,0x0E,0x20,0x0E,0x3F,0xF8,0x00,0x00,
        0x00,0x00,0x3F,0xF8,0x3C,0x0E,0x3C,0x0E,0x3C,0x0E,0x3C,0x0E,0x3F,0xF8,0x00,0x00,
        0x00,0x00,0x3F,0xF8,0x3F,0x8E,0x3F,0x8E,0x3F,0x8E,0x3F,0x8E,0x3F,0xF8,0x00,0x00,
        0x00,0x00,0x3F,0xF8,0x3F,0xFE,0x3F,0xFE,0x3F,0xFE,0x3F,0xFE,0x3F,0xF8,0x00,0x00
    };
    
    static BOOL POn = TRUE;
    static BYTE ChargeCnt = 0;  // 充电进度条计数器
    static unsigned char PreucVoltPer = 0;   // 前一次电池电压

    
    //float uiVolt = 7.0;
    //char chVoltPer[5] = {0};

    
    //uiVolt = GetVlotValu();
    ucVoltPer = GetVlotPer();

    // 初始化历史电压
    if (PreucVoltPer == 0)
    {
        PreucVoltPer = ucVoltPer;
    }

    
    // 显示电池图标
    if (GetAlarm(ALARM_BAT_LOW))
    {
        POn = !POn;
    }
    else
    {
        POn = TRUE;
    }
    
    if(DC_IN == 0)   // 外接电源
    {
        //保证电池百分百在充电时 只增不降
        if(PreucVoltPer < ucVoltPer)
        {
            PreucVoltPer = ucVoltPer;
        }
        else
        {
            ucVoltPer = PreucVoltPer;
        }


        if (CHARGING == 0)  // 正在充电
        {
            Diplay(112,0,16,8,PowerValu[ChargeCnt++%4],0);    
        }
        else
        {   // 充电完成          
            ucVoltPer = 100;
            Diplay(112,0,16,8,PowerValu[3],0);           
        }

    }
    else
    {
        
        //保证电池百分百在不充电时 只降不增
        if(PreucVoltPer > ucVoltPer)
        {
            PreucVoltPer = ucVoltPer;
        }
        else
        {
            ucVoltPer = PreucVoltPer;
        }

        
        if(ucVoltPer>=75)
        {
            //电池满
            Diplay(112,0,16,8,PowerValu[3],0);
        }
        else if(ucVoltPer>=50)
        {
            //电池2格电
            Diplay(112,0,16,8,PowerValu[2],0);
        }
        else if(ucVoltPer>=25)
        {
            //电池1格电
            Diplay(112,0,16,8,PowerValu[1],0);
        }
        else if(ucVoltPer>=20)
        {
            Diplay(112,0,16,8,PowerValu[0],0);
        }
        else
        {
            if (POn)
            {
                Diplay(112,0,16,8,PowerValu[0],0);
            }
        }
    }

    // 显示百分比
    if(ucVoltPer >= 100)
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%d%%",ucVoltPer);
        Show5X7String(88,0,StrTemp,0); 
    }
    else if (ucVoltPer >= 20)
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%d%%",ucVoltPer);
        Show5X7String(94,0,StrTemp,0);
    }
    else if(ucVoltPer >= 10)
    {
        if (POn)
        {
            // 闪烁
            memset(StrTemp,0,24);
            sprintf(StrTemp,"%d%%",ucVoltPer);
            Show5X7String(94,0,StrTemp,0);
        }
    }        
    else
    {
        if (POn)
        {
            // 闪烁
            memset(StrTemp,0,24);
            sprintf(StrTemp,"%d%%",ucVoltPer);
            Show5X7String(99,0,StrTemp,0);
        }
    }


    // 电池报警
    if (DC_IN == 0)
    {
        // 外部电源不告警
        ClrAlarm(ALARM_BAT_LOW);
    }
    else
    {
        if (ucVoltPer >= 10)
        {
            ClrAlarm(ALARM_BAT_LOW);
        }
        else
        {
            SetAlarm(ALARM_BAT_LOW);
        }

        #ifdef DEBUG
        //Show5X7Float(80,64,uiVolt,0);
        #else
        if (ucVoltPer < 5)
        {
            ShutDown(1);
        }
        #endif
    }
}

void ShowSenser(BYTE x, BYTE y, BYTE type)
{
    switch (type)
    {
        case SENSER_IN:   Show5X7String(x,y,"In(@Y)",0);   break;
        case SENSER_PD:   Show5X7String(x,y,"PD",0);       break;
        case SENSER_GM:   Show5X7String(x,y,"GM",0);       break;
        case SENSER_A_A:  Show5X7String(x,y,"I(@A)",0);    break;
        case SENSER_A_B:  Show5X7String(x,y,"I(@B)",0);    break;
        case SENSER_A_AB: Show5X7String(x,y,"I(@A@B)",0);  break;
        case SENSER_B_B:  Show5X7String(x,y,"II(@B)",0);   break;
        case SENSER_B_A:  Show5X7String(x,y,"II(@A)",0);   break;
        case SENSER_B_AB: Show5X7String(x,y,"II(@A@B)",0); break;
        case SENSER_ABY:  Show5X7String(x,y,"@A@B@Y",0);   break;
        case SENSER_G1:   Show5X7String(x,y,"G1",0);       break;
        case SENSER_G2:   Show5X7String(x,y,"G2",0);       break;
        case SENSER_Y:    Show5X7String(x,y,"NaI",0);      break;
        case SENSER_LP:   Show5X7String(x,y,"LP(@Y)",0);   break;
        case SENSER_LC:   Show5X7String(x,y,"LC",0);       break;
    }
}

BOOL NeedHv()
{
    if ((CheckRes.SenserType == SENSER_Y)     || 
         (CheckRes.SenserType == SENSER_NONE) ||
         (CheckRes.SenserType == SENSER_IN)   ||
         (CheckRes.SenserType == SENSER_G1)   ||
         (CheckRes.SenserType == SENSER_G2) )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void OpenHv(WORD vol)
{
    //GM_ON;
    //HV_CTL(3);
    //P13V_ON;
    //Out_HV_Val(vol);   
}

void CloseHv()
{
    //Out_HV_Val(0);
    //HV_OFF;
    //HV_CTL(0);
    //GM_OFF;
}


void ShowHv()
{
    DWORD hv;
    //char str[8] = {0};
    hv = Get_HV_Vol();
    memset(StrTemp,0,24);
    sprintf(StrTemp,"%luV",hv);
    Show12X12String(0,16,"高压:",0);
    Show12X12String(32,16,StrTemp,0);
}


// 最大字体
void ShowCounter(BYTE x, BYTE y, char *Title, float Counter, BYTE Unit)
{
    BYTE XD=76;
    const BYTE YD=38;
    
    if (Counter < 0.0)
    {
        Counter = 0.0;
    }

    #ifdef FOR_TEMP  // 临时测试用
    Show5X7String(x,y,Title,0);   
    
    if (Unit <= UNIT_CPS)
    {
        Show16X24int(x,y+8,(DWORD)Counter,0);
    }
    else
    {
        Show16X24float(x,y+8,Counter,0);
    }

    #else

    Show5X7String(x,y,Title,0); 
    if (Unit <= UNIT_CPS)
    {
        if (Counter > 9999.9)
        {
            Show16X24int(x,y+8,(DWORD)Counter,0);
            //Show16X24float(x,y+8,Counter,0);
        }
        else
        {
            //Show24X36Int(x,y,(DWORD)Counter);
            if (CheckRes.SenserType == SENSER_ABY)
            {
                Show24X36float(x,y,Counter,2);
            }
            else
            {
                Show24X36float(x,y,Counter,1);
            }
        }
    }
    else
    {
        Show24X36float(x,y,Counter,0);
    }

    #endif
    
    if (Unit != 0)
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp, "%s", UnitStr[Unit]);

        // 单位居中
        XD = (128 - strlen(StrTemp)*9)/2;

        #ifdef FOR_TEMP  // 临时测试用
        ShowUnit(x+XD,y+YD-8,StrTemp,0);
        #else
        //显示单位
        ShowUnit(x+XD,y+YD,StrTemp,0);
        #endif
    }

    #ifdef UART_DOSE
    //PrintCounter(Counter,Unit);
    #endif
    
    #if 0
    if (Unit == UNIT_Bq_cm2)
    {
        Show16X24float(x,y,Counter/100,0);
    }
    else
    {
        Show16X24float(x,y,Counter,0);
    }
    #endif

    
}

void ShowCounterMid(BYTE x, BYTE y, char *Title, float Counter, BYTE Unit)
{
    BYTE XD=76;
    const BYTE YD=38;
    
    if (Counter < 0.0)
    {
        Counter = 0.0;
    }

    if (strlen(Title) != 0)
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp, "%s: ", Title);
        Show5X7String(x,y+8,StrTemp,0); 
        x += strlen(StrTemp) * 3 + 3;
    }
    
    if (Unit <= UNIT_CPS)
    {
        //Show16X24int(x,y+8,(DWORD)Counter,0);
        Show16X24float(x,y+8,Counter,1);
    }
    else
    {
        Show16X24float(x,y+8,Counter,0);
    }
    
    if (Unit != 0)
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp, "%s", UnitStr[Unit]);

        // 单位居中
        XD = (128 - strlen(StrTemp)*9)/2;

        #ifdef FOR_TEMP  // 临时测试用
        ShowUnit(x+XD,y+YD-8,StrTemp,0);
        #else
        //显示单位
        ShowUnit(x+XD,y+YD-8,StrTemp,0);
        #endif
    }
    
    
    #if 0
    if (Unit == UNIT_Bq_cm2)
    {
        Show16X24float(x,y,Counter/100,0);
    }
    else
    {
        Show16X24float(x,y,Counter,0);
    }
    #endif

    
}


void ShowCounterSm(BYTE x, BYTE y, char *Title, float Counter, BYTE Unit)
{
    //const BYTE XD=2;
    //const BYTE YD=3;

    if (Counter < 0.0)
    {
        Counter = 0.0;
    }
    
    //BYTE xe=Show6X12Float(x,y,Counter,0);

    //显示单位
    //Show5X7String(xe+XD,y+YD,UnitStr[Unit],0);

    memset(StrTemp,0,24);

    if (strlen(Title) != 0)
    {
        if (Unit == UNIT_CPS)
        {
            sprintf(StrTemp,"%s: %.1f%s",Title, Counter,UnitStr[Unit]);
        }
        else
        {
            sprintf(StrTemp,"%s: %.2f%s",Title, Counter,UnitStr[Unit]);
        }
    }
    else
    {
        if (Unit == UNIT_CPS)
        {
            sprintf(StrTemp,"%.1f%s",Counter,UnitStr[Unit]);
        }
        else
        {
            sprintf(StrTemp,"%.2f%s",Counter,UnitStr[Unit]);
        }
    }
    
    Show5X7String(x,y,StrTemp,0);
    
    #if 0
    if (Unit == UNIT_Bq_cm2)
    {
        Show6X12Float(x,y,Counter/100,0);
    }
    else
    {
        Show6X12Float(x,y,Counter,0);
    }
    #endif
}



float Get_InCounter()
{
    #if 0
    BYTE PrukData[2] = {0};
    WORD c;
    SGetCount(PrukData);
    SCLR();
    
    c = PrukData[1];
    c <<= 8;
    c |= PrukData[0];

    return c;
    #else   
    DWORD Cps;

    Cps = CounterToCps(InSenserCnt);

    CpsFilter(Cps);    
    
    return CounterPH();
    #endif
}


// 根据计数时间累计累计剂量
void TotalAdd()
{
    FaceInfo.In_Total += FaceInfo.In_Rt*Param.InSenser.Param.Ct/1000/3600;    
}


void InTotalZhuAlarm()
{
    static float TtHis = 0.0;
    if (HaveValidAlarm())
    {
        return;
    }

    if (TtHis == 0.0)
    {
        TtHis = FaceInfo.In_Total;
    }

    if ((FaceInfo.In_Total - TtHis) >  
        (Param.InSenser.Alarm.Mx * Param.InSenser.Alarm.Zu / 100))
    {
        TtHis = FaceInfo.In_Total;
        StartBeep(300,0);
    }
}

// NaI 探头
void Check_InAlarmEx()
{
    if (!SwSenser) 
    {
        return;
    }
    
    if (FaceInfo.In_Rt >= Senser.Alarm.Y2)
    {
        ClrAlarm(ALARM_Y1);
        SetAlarm(ALARM_Y2);
    }
    else if (FaceInfo.In_Rt >= Senser.Alarm.Y1)
    {
        SetAlarm(ALARM_Y1);
        ClrAlarm(ALARM_Y2);
    }
    else
    {
        ClrAlarm(ALARM_Y1);
        ClrAlarm(ALARM_Y2);
    }

    if (FaceInfo.In_Total >= Senser.Alarm.Mx)
    {
        ClrAlarm(ALARM_YL);
        SetAlarm(ALARM_YH);
    }
    else if (FaceInfo.In_Total >= Senser.Alarm.Ma)  
    {
        SetAlarm(ALARM_YL);
        ClrAlarm(ALARM_YH);
    }
    else
    {
        ClrAlarm(ALARM_YL);
        ClrAlarm(ALARM_YH);
    }
}


//液体探头
void Check_InAlarmLC()
{
    if (FaceInfo.OutA_Cps_Rt >= Senser.Alarm.A2)
    {
        ClrAlarm(ALARM_A1);
        SetAlarm(ALARM_A2);
    }
    else if (FaceInfo.OutA_Cps_Rt >= Senser.Alarm.A1)
    {
        SetAlarm(ALARM_A1);
        ClrAlarm(ALARM_A2);
    }
    else
    {
        ClrAlarm(ALARM_A1);
        ClrAlarm(ALARM_A2);
    }

    if (FaceInfo.OutB_Cps_Rt >= Senser.Alarm.B2)
    {
        ClrAlarm(ALARM_B1);
        SetAlarm(ALARM_B2);
    }
    else if (FaceInfo.OutB_Cps_Rt >= Senser.Alarm.B1)
    {
        SetAlarm(ALARM_B1);
        ClrAlarm(ALARM_B2);
    }
    else
    {
        ClrAlarm(ALARM_B1);
        ClrAlarm(ALARM_B2);
    }
}

void Check_InAlarm()
{
    #ifndef DG105
    WORD tm = 0;
    if (InnerParam.DevType == DEV_MPRP_S2L)
    {
        tm = 1800;
    }
    else
    {
        tm = 600;
    }

    // 10分钟没有计数报异常
    if (InSerserErrTimer >= tm)
    {
        SetAlarm(ALARM_INSENSER_ERR);
        InSerserErrTimer = tm;  // 防止溢出
    }
    else
    {
        ClrAlarm(ALARM_INSENSER_ERR);
    }
    #endif
    
    if (FaceInfo.In_Rt >= Param.InSenser.Alarm.Y3)
    {
        ClrAlarm(ALARM_IN1);
        ClrAlarm(ALARM_IN2);
        SetAlarm(ALARM_IN3);
    }
    else if (FaceInfo.In_Rt >= Param.InSenser.Alarm.Y2)
    {
        ClrAlarm(ALARM_IN1);
        SetAlarm(ALARM_IN2);
        ClrAlarm(ALARM_IN3);
    }
    else if (FaceInfo.In_Rt >= Param.InSenser.Alarm.Y1)
    {
        SetAlarm(ALARM_IN1);
        ClrAlarm(ALARM_IN2);
        ClrAlarm(ALARM_IN3);
    }
    else
    {
        ClrAlarm(ALARM_IN1);
        ClrAlarm(ALARM_IN2);
        ClrAlarm(ALARM_IN3);
    }

    

    if (FaceInfo.In_Total >= Param.InSenser.Alarm.Mx)
    {
        SetAlarm(ALARM_YT);
        ClrAlarm(ALARM_YP);
        InTotalZhuAlarm();    // 柱报警
    }
    else if (FaceInfo.In_Total >= Param.InSenser.Alarm.Ma)
    {
        ClrAlarm(ALARM_YT);
        SetAlarm(ALARM_YP);
    }
    else
    {
        ClrAlarm(ALARM_YT);
        ClrAlarm(ALARM_YP);
    }
        
}


void Show_InCounter(BYTE x, BYTE y, float val)
{
    static BYTE UnitHis = UNIT_uSv_h;
    if (val < 0.0)
    {
        val = 0.0;
    }

    if (val > 99999000000)
    {
        val = 99999000000;
    }
    
    if (val >= 1000000)
    {
        ShowCounter(x,y,"",(float)val/1000000,UNIT_Sv_h);
        UnitHis = UNIT_Sv_h;
    }
    else if (val >= 200000)
    {
        // 单位不变
        if (UnitHis == UNIT_Sv_h)
        {
            ShowCounter(x,y,"",(float)val/1000000,UNIT_Sv_h);
        }
        else
        {
            ShowCounter(x,y,"",(float)val/1000,UNIT_mSv_h);
        }
    }
    else if (val >= 1000)
    {
        ShowCounter(x,y,"",(float)val/1000,UNIT_mSv_h);
        UnitHis = UNIT_mSv_h;
    }
    else if (val >= 200)
    {
        // 单位不变
        if (UnitHis == UNIT_mSv_h)
        {
            ShowCounter(x,y,"",(float)val/1000,UNIT_mSv_h);
        }
        else
        {
            ShowCounter(x,y,"",(float)val,UNIT_uSv_h);
        }
    }
    else 
    {
        ShowCounter(x,y,"",(float)val,UNIT_uSv_h);
        //ShowCounterMid(x,y,"@A",(float)val,UNIT_uSv_h);
        UnitHis = UNIT_uSv_h;
    }

    
}

void Show_InCounterMid(BYTE x, BYTE y, float val)
{
    static BYTE UnitHis = UNIT_uSv_h;
    if (val < 0.0)
    {
        val = 0.0;
    }

    if (val > 99999000000)
    {
        val = 99999000000;
    }
    
    if (val >= 1000000)
    {
        ShowCounterMid(x,y,"",(float)val/1000000,UNIT_Sv_h);
        UnitHis = UNIT_Sv_h;
    }
    else if (val >= 200000)
    {
        // 单位不变
        if (UnitHis == UNIT_Sv_h)
        {
            ShowCounterMid(x,y,"",(float)val/1000000,UNIT_Sv_h);
        }
        else
        {
            ShowCounterMid(x,y,"",(float)val/1000,UNIT_mSv_h);
        }
    }
    else if (val >= 1000)
    {
        ShowCounterMid(x,y,"",(float)val/1000,UNIT_mSv_h);
        UnitHis = UNIT_mSv_h;
    }
    else if (val >= 200)
    {
        // 单位不变
        if (UnitHis == UNIT_mSv_h)
        {
            ShowCounterMid(x,y,"",(float)val/1000,UNIT_mSv_h);
        }
        else
        {
            ShowCounterMid(x,y,"",(float)val,UNIT_uSv_h);
        }
    }
    else 
    {
        ShowCounterMid(x,y,"",(float)val,UNIT_uSv_h);
        UnitHis = UNIT_uSv_h;
    }
}


void Show_InCounterSm(BYTE x, BYTE y, float val)
{
    if (val < 0.0)
    {
        val = 0.0;
    }

    if (val > 99999990000)
    {
        val = 99999990000;
    }
    
    if (val >= 1000000)
    {
        ShowCounterSm(x,y,"",(float)val/1000000,UNIT_Sv_h);
    }
    else if (val >= 1000)
    {
        ShowCounterSm(x,y,"",(float)val/1000,UNIT_mSv_h);
    }
    else 
    {
        ShowCounterSm(x,y,"",(float)val,UNIT_uSv_h);
    }
}


void Show_InTotal(BYTE x, BYTE y, float val)
{
    if (val < 0.0)
    {
        val = 0.0;
    }

    if (val > 99999990000)
    {
        val = 99999990000;
    }
    
    if (val >= 1000000)
    {
        ShowCounterSm(x,y,"",(float)val/1000000,UNIT_Sv);
    }
    else if (val >= 1000)
    {
        ShowCounterSm(x,y,"",(float)val/1000,UNIT_mSv);
    }
    else
    {
        ShowCounterSm(x,y,"",(float)val,UNIT_uSv);
    }
    
    //ShowCounter(8,54,"",(float)Param.TotalCounter,5);
}

void ShowNucName(BYTE y)
{
    BYTE len = strlen(NucName[Senser.Param.Hn]);

    //核素居中显示
    Show5X7String((21-len)/2*6,y,NucName[Senser.Param.Hn],0);
}


// 内部探头切换到外部探头
BOOL SwitchSenser()
{
    static BYTE Cnt1 = 0;
    
    if (FaceInfo.OutA_Cps_Rt > 100)
    {
        if (!SwSenser)
        {
            if (Cnt1 > 3) 
            {
                SwSenser = TRUE;
                //#ifdef DG105  --  // 2019.1.10  -- 所有NaI探头切换了都用高量程
                // 2018.8.24  -- DG105 只有高量程盖葛管
                GM_HIGH;
                FaceInfo.Gm = 1;
                //#endif
                return TRUE;
            }
            else
            {
                Cnt1 ++;
                
            }
        }
    }
    else
    {
        if (SwSenser)
        {
            // 下降到80%才往外部切换
            //if (FaceInfo.OutA_Cps_Rt < 90)
            if (FaceInfo.In_Rt < 90)
            {
                if (Cnt1 == 0)
                {
                    SwSenser = FALSE;
                    GM_LOW;             // 关闭继电器，降低功耗
                    return TRUE;
                }
                else
                {
                    Cnt1 --;
                }
            }
        }
        
    }

    return FALSE;
}






// 获取外部探头计数
void GetOutCounter()
{
    static BYTE ReConn_Count = 0;
    BYTE i,j;

    if (GetSenserCounter())
    {
        // 通信恢复后读一下电池电压
        if (ReConn_Count != 0)
        {
            if (CheckRes.CommInterface == COMM_BT)
            { 
                BTGetBattary(1);
            }
        }
        
        ReConn_Count = 0;
        ClrAlarm(ALARM_SENSER_ERR);

        #if 0
        if (CheckRes.CommInterface == COMM_CAN)
        {
            GetSenserWork();

            #if 0
            switch (Senser.WorkSt)
            {
                case 1:   Show12X12String(0,20,"电测试",0);  break;
                case 2:   Show12X12String(0,20,"物理测试",0); break;
                case 3:   Show12X12String(0,20,"辐射测试",0); break;
                default:  Show12X12String(0,20,"状态异常",0); break;
            }
            #endif

            if (Senser.WorkSt != 3)
            {
                SetAlarm(ALARM_STATUS_ERR);
            }
            else
            {
                ClrAlarm(ALARM_STATUS_ERR);
            }
        }
        #endif

        
        for (i=0;i<Senser.Contxt->CounterCnt;i++)
        {
            j = GetValidCounterLoc(i);

            switch (i)
            {
                case 0: FaceInfo.OutA_Cps_Rt =  GetCounterVal(j); break;    
                case 1: FaceInfo.OutB_Cps_Rt =  GetCounterVal(j); break;  

                //现在的探头最多两个计数
                //case 2: FaceInfo.OutC_Cps_Rt =  GetCounterVal(j); break;
            }
        }

        if (FaceInfo.OutA_Cps_Max < FaceInfo.OutA_Cps_Rt)
        {
            FaceInfo.OutA_Cps_Max = FaceInfo.OutA_Cps_Rt;
        }

        if (FaceInfo.OutB_Cps_Max < FaceInfo.OutB_Cps_Rt)
        {
            FaceInfo.OutB_Cps_Max = FaceInfo.OutB_Cps_Rt;
        }
    }
    else
    {
        // 出现异常后重新初始化
        if (!ReConnectSenser())
        {
            if (ReConn_Count < 10)
            {
                ReConn_Count ++;
            }
            else 
            {
                // 十次通信异常才报警
                SetAlarm(ALARM_SENSER_ERR);
            }
        }
        else
        {
            ReConn_Count = 0;
        }
    }

    
}


void Face_InSenser()
{       
    if (InSenserEnable)
    {
        Show_InCounter(0,16, FaceInfo.In_Rt);
            
        //ShowCurrProgress(Param.InSenser.Alarm.Y1,Param.InSenser.Alarm.Y2,FaceInfo.In_Rt);
        ShowProgress(82, 0,
                        Param.InSenser.Alarm.Y1,
                       Param.InSenser.Alarm.Y2,
                       //Param.InSenser.Alarm.Y3,
                       FaceInfo.In_Rt);


        Show5X7String(4,92,"Rmax:",0);
        Show_InCounterSm(34,92, FaceInfo.In_Max);
        
        Show5X7String(4,102,"Dose:",0);
        Show_InTotal(34,102, FaceInfo.In_Total);
    }
    else
    {
        GT_Show16X16Char(32, 58, "无探测器", 0);
    }
}


void Face_NaISerser()
{
    //FaceInfo.OutA_Cur_Rt = CpsUnit(FaceInfo.OutA_Cps_Rt, 
    //                              Param.pnuc[CurRadIndex].Unit);
    //Show_InCounter(0,16, FaceInfo.OutA_Cps_Rt);
    if (SwSenser)
    {
        Show_InCounter(0,16, FaceInfo.In_Rt);
        ShowProgress(82,0,
                   Senser.Alarm.Y1,
                   Senser.Alarm.Y2,
                   FaceInfo.In_Rt);

        Show5X7String(4,92,"Rmax:",0);
        Show_InCounterSm(34,92, FaceInfo.In_Max);
        
        Show5X7String(4,102,"Dose:",0);
        Show_InTotal(34,102, FaceInfo.In_Total);
    }
    else
    {
        //#ifdef DG105
        // 2018.8.24  -- DG105 只有高量程盖葛管
        //GM_LOW;    // 关闭继电器，降低功耗
        //#endif
        
        Show_InCounter(0,16, FaceInfo.OutA_Cps_Rt);
        ShowProgress(82,0,
                   Senser.Alarm.Y1,
                   Senser.Alarm.Y2,
                   FaceInfo.OutA_Cps_Rt);

        Show5X7String(4,92,"Rmax:",0);
        Show_InCounterSm(34,92, FaceInfo.OutA_Cps_Max);
        
        Show5X7String(4,102,"Dose:",0);
        Show_InTotal(34,102, FaceInfo.OutB_Cps_Rt);
    }
}


void Face_LPSenser()
{
    static BOOL bFirst = TRUE;

    if (bFirst)
    {
        if (SenserBatery.percent < 3)
        {
            GT_Show16X16Char(0,20,"请更换探测器电池",0);
        }
        else
        {
            bFirst = FALSE;
            Show_InCounter(0,16, FaceInfo.OutA_Cps_Rt);
        }
    }
    else
    {
        if (SenserBatery.percent == 0)
        {
            GT_Show16X16Char(0,20,"请更换探测器电池",0);
        }
        else
        {
            Show_InCounter(0,16, FaceInfo.OutA_Cps_Rt);
        }
    }
    
    ShowProgress(82,0,
               Senser.Alarm.Y1,
               Senser.Alarm.Y2,
               FaceInfo.OutA_Cps_Rt);

    Show5X7String(4,92,"Rmax:",0);
    Show_InCounterSm(34,92, FaceInfo.OutA_Cps_Max);
    
    Show5X7String(4,102,"Dose:",0);
    Show_InTotal(34,102, FaceInfo.OutB_Cps_Rt);
}


void Face_ABSenser()
{
    #if 0
    if (!HvValid())
    {
        SetAlarm(ALARM_HV_ERR);
    }
    else
    {
        ClrAlarm(ALARM_HV_ERR);
    }
    #endif

    ShowNucName(10);

    FaceInfo.OutA_Cur_Rt = CpsUnit(FaceInfo.OutA_Cps_Rt, 
                                  pnuc[CurRadIndex].Unit);
    
    if (FaceInfo.OutA_Cur_Max < FaceInfo.OutA_Cur_Rt)
    {
        FaceInfo.OutA_Cur_Max = FaceInfo.OutA_Cur_Rt;
    }
    
    ShowCounter(0,18,"@A:",FaceInfo.OutA_Cur_Rt,pnuc[CurRadIndex].Unit);

    #if 0
    // 内部探头
    Show5X7String(0,76,"@Y:",0);
    Show_InCounterSm(14,74, FaceInfo.In_Rt);
    
    ShowCurrProgress(0,
                        CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                       FaceInfo.OutA_Cur_Rt);
    #else
    ShowProgress(82,0,
                   0,
                   CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                   FaceInfo.OutA_Cps_Rt);

    Show5X7String(4,92,"SAmax:",0);
    ShowCounterSm(40,92,"", FaceInfo.OutA_Cur_Max,pnuc[CurRadIndex].Unit);
    
    Show5X7String(4,102,"@Yrate:",0);
    Show_InCounterSm(40,102, FaceInfo.In_Rt);
    //Show_InTotal(40,102, FaceInfo.In_Total);
    #endif
    
}



void Face_AaSenser()
{
    
}

void Face_PdSenser()
{

}

void Face_GmSenser()
{
    Show_InCounter(0,16, FaceInfo.OutA_Cps_Rt);
    
    ShowProgress(82,0,
               Senser.Alarm.A1,
               Senser.Alarm.A2,
               FaceInfo.OutA_Cps_Rt);

    Show5X7String(4,97,"Rmax:",0);
    Show_InCounterSm(34,97, FaceInfo.OutA_Cps_Max);
    
    //Show5X7String(4,102,"Dose:",0);
    //Show_InTotal(34,102, FaceInfo.OutB_Cps_Rt);
}

void Face_AbSenser()
{

}

void Face_AabSenser()
{
    ShowNucName(10);

    FaceInfo.OutA_Cur_Rt = CpsUnit(FaceInfo.OutA_Cps_Rt, 
                                  pnuc[CurRadIndex].Unit);
    
    if (FaceInfo.OutA_Cur_Max < FaceInfo.OutA_Cur_Rt)
    {
        FaceInfo.OutA_Cur_Max = FaceInfo.OutA_Cur_Rt;
    }
    
    ShowCounter(0,18,"@A:",FaceInfo.OutA_Cur_Rt,pnuc[CurRadIndex].Unit);
    ShowCounter(0,82,"@B:",FaceInfo.OutB_Cur_Rt,pnuc[CurRadIndex].Unit);

    #if 0
    ShowProgress(82,0,
                   0,
                   CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                   FaceInfo.OutA_Cps_Rt);

    Show5X7String(4,92,"SAmax:",0);
    ShowCounterSm(40,92,"", FaceInfo.OutA_Cur_Max,pnuc[CurRadIndex].Unit);
    
    Show5X7String(4,102,"@Yrate:",0);
    Show_InCounterSm(40,102, FaceInfo.In_Rt);
    #endif
}

void Face_BbSenser()
{
    #if 0
    if (!HvValid())
    {
        SetAlarm(ALARM_HV_ERR);
    }
    else
    {
        ClrAlarm(ALARM_HV_ERR);
    }
    #endif

    ShowNucName(10);

    FaceInfo.OutA_Cur_Rt = CpsUnit(FaceInfo.OutA_Cps_Rt, 
                                  pnuc[CurRadIndex].Unit);
    
    if (FaceInfo.OutA_Cur_Max < FaceInfo.OutA_Cur_Rt)
    {
        FaceInfo.OutA_Cur_Max = FaceInfo.OutA_Cur_Rt;
    }
    
    ShowCounter(0,18,"@B:",FaceInfo.OutA_Cur_Rt,pnuc[CurRadIndex].Unit);

    #if 0
    // 内部探头
    Show5X7String(0,76,"@Y:",0);
    Show_InCounterSm(14,74, FaceInfo.In_Rt);
    
    ShowCurrProgress(0,
                        CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                       FaceInfo.OutA_Cur_Rt);
    #else
    ShowProgress(82,0,
                   0,
                   CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                   FaceInfo.OutA_Cps_Rt);

    Show5X7String(4,92,"SAmax:",0);
    ShowCounterSm(40,92,"", FaceInfo.OutA_Cur_Max,pnuc[CurRadIndex].Unit);
    
    Show5X7String(4,102,"@Yrate:",0);
    Show_InCounterSm(40,102, FaceInfo.In_Rt);
    //Show_InTotal(40,102, FaceInfo.In_Total);
    #endif
    
}


void Face_BaSenser()
{

}


void Face_BabSenser()
{

}



void Face_G1Senser()
{
    FaceInfo.OutA_Cur_Rt = CpsUnit(FaceInfo.OutA_Cps_Rt, 
                                  pnuc[CurRadIndex].Unit);

    if (FaceInfo.OutA_Cur_Max < FaceInfo.OutA_Cur_Rt)
    {
        FaceInfo.OutA_Cur_Max = FaceInfo.OutA_Cur_Rt;
    }
    
    FaceInfo.OutB_Cur_Rt = CpsUnit(FaceInfo.OutB_Cps_Rt, 
                                  pnuc[CurRadIndex].Unit);

    if (FaceInfo.OutB_Cur_Max < FaceInfo.OutB_Cur_Rt)
    {
        FaceInfo.OutB_Cur_Max = FaceInfo.OutB_Cur_Rt;
    }

    if (Senser.Param.Hn == 0) // MIX
    {
        if (CheckRes.SenserType  == SENSER_LC)
        {
            if (bTimerMeas)  // 长杆定时测量
            {
                memset(StrTemp,0,24);
                sprintf(StrTemp,"%d",MeasTime--);
                Show5X7String(60,10,StrTemp,0);
                if (MeasTime == 0)
                {
                    ShowMeasRes();
                    return;
                }
            }
        }
        
        #ifdef FOR_TEMP  // 临时测试用
        
        // 临时版本
        ShowCounter(0,18,"@B:",FaceInfo.OutB_Cur_Rt,pnuc[CurRadIndex].Unit);  // 显示一个单位就可以了
        Show_InCounter(0,58,FaceInfo.In_Rt);

        #else
        // 混合场的单位一定是CPS

        if (RadMeasMode == RAD_MODE_MIX)
        {
            ShowCounter(0,18,"@A:",FaceInfo.OutA_Cur_Rt,0);  // 显示一个单位就可以了
            ShowCounter(0,58,"@B@Y:",FaceInfo.OutB_Cur_Rt,pnuc[CurRadIndex].Unit);
        }
        else if (RadMeasMode == RAD_MODE_A)
        {
            ShowCounter(0,28,"@A:",FaceInfo.OutA_Cur_Rt,pnuc[CurRadIndex].Unit);  // 显示一个单位就可以了
            ShowProgress(92, 0,
                   0,
                   CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                   FaceInfo.OutA_Cur_Rt);

            Show5X7String(4,102,"Max:",0);
            ShowCounterSm(34,102,"", FaceInfo.OutA_Cur_Max,pnuc[CurRadIndex].Unit);
        }
        else
        {
            ShowCounter(0,28,"@B@Y:",FaceInfo.OutB_Cur_Rt,pnuc[CurRadIndex].Unit);
            ShowProgress(92, 0,
                   0,
                   CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                   FaceInfo.OutB_Cur_Rt);

            Show5X7String(4,102,"Max:",0);
            ShowCounterSm(34,102,"", FaceInfo.OutB_Cur_Max,pnuc[CurRadIndex].Unit);
        }

        
        
        #endif
        
    }
    else if (GetCurChannel() == CHANNEL_A)  // α
    {
        ShowNucName(16);
        
        #ifdef FOR_TEMP  // 临时测试用
        
        // 临时版本
        ShowCounter(0,18,"@A:",FaceInfo.OutA_Cur_Rt,pnuc[CurRadIndex].Unit);  
        Show_InCounter(0,58,FaceInfo.In_Rt);
        
        #else
        ShowCounter(0,28,"",FaceInfo.OutA_Cur_Rt,pnuc[CurRadIndex].Unit);
        
        ShowProgress(92, 0,
                   0,
                   CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                   FaceInfo.OutA_Cur_Rt);

        Show5X7String(4,102,"Max:",0);
        ShowCounterSm(34,102,"", FaceInfo.OutA_Cur_Max,pnuc[CurRadIndex].Unit);

        #endif
    
    }
    else  if (GetCurChannel() == CHANNEL_B) // BY
    {
        ShowNucName(16);
        
        #ifdef FOR_TEMP  // 临时测试用
        
        // 临时版本
        ShowCounter(0,18,"@B:",FaceInfo.OutB_Cur_Rt,pnuc[CurRadIndex].Unit);  // 显示一个单位就可以了
        Show_InCounter(0,58,FaceInfo.In_Rt);
        
        #else
        ShowCounter(0,28,"",FaceInfo.OutB_Cur_Rt,pnuc[CurRadIndex].Unit);

        ShowProgress(92, 0,
                   0,
                   CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                   FaceInfo.OutB_Cur_Rt);

        Show5X7String(4,102,"Max:",0);
        ShowCounterSm(34,102,"", FaceInfo.OutB_Cur_Max,pnuc[CurRadIndex].Unit);

        #endif
    }
    else
    {
        Show12X12String(0,22,"参数错误",0);
    }
}

void Face_ABYSenser()
{
    FaceInfo.OutA_Cur_Rt = CpsUnit(FaceInfo.OutA_Cps_Rt, 
                                  pnuc[CurRadIndex].Unit);

    if (FaceInfo.OutA_Cur_Max < FaceInfo.OutA_Cur_Rt)
    {
        FaceInfo.OutA_Cur_Max = FaceInfo.OutA_Cur_Rt;
    }
    
    FaceInfo.OutB_Cur_Rt = CpsUnit(FaceInfo.OutB_Cps_Rt, 
                                  pnuc[CurRadIndex].Unit);

    if (FaceInfo.OutB_Cur_Max < FaceInfo.OutB_Cur_Rt)
    {
        FaceInfo.OutB_Cur_Max = FaceInfo.OutB_Cur_Rt;
    }

    {
        ShowNucName(16);
        ShowCounter(0,28,"",FaceInfo.OutA_Cur_Rt,pnuc[CurRadIndex].Unit);
        
        ShowProgress(92, 0,
                   0,
                   CpsUnit(pnuc[CurRadIndex].AlarmThres,
                                  pnuc[CurRadIndex].Unit),
                   FaceInfo.OutA_Cur_Rt);

        Show5X7String(4,102,"Max:",0);
        ShowCounterSm(34,102,"", FaceInfo.OutA_Cur_Max,pnuc[CurRadIndex].Unit);

    }
    
    
}



void Face_G2Senser()
{

}


BYTE ValidAlarmCnt()
{
    BYTE i,ret;

    ret = 0;
    for (i=0;i<32;i++)
    {
        if (SysAlarm.Mask1 & (1l<<i))
        {
            if (SysAlarm.Status1 & (1l<<i))
            {
                ret ++;
            }
        }
    }

    for (i=0;i<32;i++)
    {
        if (SysAlarm.Mask2 & (1l<<i))
        {
            if (SysAlarm.Status2 & (1l<<i))
            {
                ret ++;
            }
        }
    }

    return ret;
}


BYTE AlarmCnt()
{
    BYTE i,ret;

    ret = 0;
    for (i=0;i<32;i++)
    {
        //if (SysAlarm.Mask1 & (1l<<i))
        {
            if (SysAlarm.Status1 & (1l<<i))
            {
                ret ++;
            }
        }
    }

    for (i=0;i<32;i++)
    {
        //if (SysAlarm.Mask2 & (1l<<i))
        {
            if (SysAlarm.Status2 & (1l<<i))
            {
                ret ++;
            }
        }
    }

    return ret;
}


// 获取第几个有效报警
BYTE GetAlarmIndex(BYTE no)
{
    BYTE i,c;
    

    c = 0;
    for (i=0;i<32;i++)
    {
        //if (SysAlarm.Mask1 & (1l<<i))
        {
            if (SysAlarm.Status1 & (1l<<i))
            {
                c ++;
                if (c == no)
                {
                    return i;
                }
            }
        }
    }
    

    for (i=0;i<32;i++)
    {
        //if (SysAlarm.Mask2 & (1l<<i))
        {
            if (SysAlarm.Status2 & (1l<<i))
            {
                c ++;
                if (c == no)
                {
                    return i+32;
                }
            }
        }
    }

    return 0xFF;
}

BYTE ValidAlarmIndex(BYTE no)
{
    BYTE i,c;
    

    c = 0;
    for (i=0;i<32;i++)
    {
        if (SysAlarm.Mask1 & (1l<<i))
        {
            if (SysAlarm.Status1 & (1l<<i))
            {
                c ++;
                if (c == no)
                {
                    return i;
                }
            }
        }
    }
    

    for (i=0;i<32;i++)
    {
        if (SysAlarm.Mask2 & (1l<<i))
        {
            if (SysAlarm.Status2 & (1l<<i))
            {
                c ++;
                if (c == no)
                {
                    return i+32;
                }
            }
        }
    }

    return 0xFF;
}



#if 0
void ShowAlarm()
{
    static BYTE tm=0;
    BYTE i;
    
    if (HadAlarm == FALSE)
    {
        // 如果以前没有告警则立刻显示告警
        tm = 0;
    }

    HadAlarm = TRUE;
    
    if (tm % 2 == 0)  // 两秒轮流显示
    {
        i = GetCurAlarm();
        Show12X12String(4,114,AlarmName[i],0);
    }



    tm ++;
}

#else
void ShowAlarm()
{
    static BYTE tm = 0;
    static BYTE ac = 0;
    static BYTE i;
    static BYTE j;

    if (HadAlarm == FALSE)
    {
        // 如果以前没有告警则立刻显示告警
        tm = 0;
        ac = 1;
    }

    HadAlarm = TRUE;

    // 同时多个报警轮流显示
    if (tm % 2 == 0)  // 两秒轮流显示
    {
        j = AlarmCnt();

        if (ac>j)
        {
            ac = 0;
        }

        if (ac > 0)
        {
            i = GetAlarmIndex(ac);
        }
        
        ac++;
    }

    tm ++;

    if (ac == 1)
    {  
        // 显示完所有报警后，再显示一下时间
        ShowTime2(117);
    }
    else
    {
        Show12X12String(4,114,AlarmName[i],0);
    }
}
#endif

void ShowSpeaker()
{
    // 宽9
    const BYTE SoundPic[] = {
        0x0C, 0x80, 
        0x15, 0x00, 
        0xE4, 0x00,
        0xA5, 0x80,
        0xE4, 0x00,
        0x15, 0x00, 
        0x0C, 0x80, 
    };

    const BYTE NoSound[] = {
        #if 0
        0x8C, 0x00, 
        0x54, 0x00, 
        0xE4, 0x00,
        0xB4, 0x00,
        0xEC, 0x00,
        0x14, 0x00, 
        0x0E, 0x00, 
        #else
        0x0C,0x00,0x14,0x00,0xE6,0x80,0xA5,0x00,0xE6,0x80,0x14,0x00,0x0C,0x00
        #endif
    };
    
    if (InnerParam.Sound)
    {
        Diplay(79,0,16,7,SoundPic,0);
    }
    else
    {
        Diplay(79,0,16,7,NoSound,0); 
    }
}


void ShowLight()
{
    // 宽 11
    const BYTE LampPic[] = {
        0x4E, 0x40,
        0x11, 0x00,
        0xD5, 0x60,
        0x0E, 0x00,
        0x2A, 0x80,
        0x4E, 0x40,
        0x04, 0x00,   
    };

    if (InnerParam.pisScreen)
    {
        Diplay(67,0,16,7,LampPic,0);
    }
}

void ShowFlash()
{
    const BYTE FlashPic[] = {
        0x08, 
        0x10, 
        0x30, 
        0x7C, 
        0x18, 
        0x10, 
        0x20,    
    };

    //if (CheckRes.SenserType != SENSER_NONE)
    {
        Diplay(59,0,8,7,FlashPic,0);
    }
}

void ShowPlug()
{
    // 宽9
    const BYTE PlugPic[] = {
        0x0C,0x00,0x1F,0x80,0x3C,0x00,0xFC,0x00,0x3C,0x00,0x1F,0x80,0x0C,0x00
    };

    if(DC_IN == 0)   // 外接电源
    {
        Diplay(49,0,16,7,PlugPic,0);
    }
}

void ShowWifi()
{
    #ifdef USE_WIFI
    // 宽7
    const BYTE WifiPic[] = {
        0x38,0x44,0x82,0x10,0x28,0x00,0x10
    };

    if(Param.Wifi.Enable)   
    {
        Diplay(30,0,8,7,WifiPic,0);
    }
    #else
    // 宽7
    const BYTE BtPic[] = {
        0x18,0x54,0x38,0x10,0x38,0x54,0x18
    };

    if (CheckRes.CommInterface == COMM_BT)
    {
        Diplay(30,0,8,7,BtPic,0);
    }
    #endif
}


void ShowCable()
{
    // 宽9
    const BYTE CabPic[] = {
        0x07,0x00,0x08,0x80,0x10,0x40,0xF0,0x40,0x10,0x40,0x08,0x80,0x07,0x00
    };

    if(CheckRes.SenserType != SENSER_NONE)   
    {
        Diplay(38,0,16,7,CabPic,0);
    }
}


void ShowMeteTitle()
{
    if (CheckRes.SenserType != SENSER_NONE)
    {
        // 探头类型
        //ShowSenser(0,0,CheckRes.SenserType);
        if (CheckRes.SenserType == SENSER_Y)
        {   
            // 外部Y探头要切换
            //if (NeedSwitchSenser())
            if (SwSenser)
            {
                Show5X7String(0,0,"In(@Y)",0);
            }
            else
            {
                ShowSenser(0,0,CheckRes.SenserType);
            }
        }
        else
        {
            ShowSenser(0,0,CheckRes.SenserType);
        }
    }
    else
    {
        if (InSenserEnable)
        {
            Show5X7String(0,0,"In(@Y)",0);
        }
        else
        {
            Show5X7String(0,0,"None",0);
        }
    }

    ShowWifi();   // 30 + 8
    ShowCable();  // 38 + 10
    ShowPlug();   // 48 + 10
    //ShowFlash();  // 58 + 9
    ShowLight();  // 67 + 12
    ShowSpeaker(); // 79
    ShowBatPower();
    ShowLine(8);
}


// 屏幕的最下方为状态条
void ShowStatusBar()
{
    const BYTE LVLine[] = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80};
    const BYTE RVLine[] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
    ShowLine(113);
    ShowLine(127);
    Diplay(0,114,8,13,LVLine,0);
    Diplay(120,114,8,13,RVLine,0);

    if (HaveAlarm())
    {
        ShowAlarm();
    }
    else
    {
        HadAlarm = FALSE;
        ShowTime2(117);
    }
}


// 主界面1 秒钟任务
void Task_1s()
{

    static WORD t=0;
    if (t++ >= 600)
    {
        // 10 分钟采集一次电池电压
        t = 0;
        ADC12_Start();

        // 10 分钟保存一下参数
        //memcpy(&Param.MetInfo, &FaceInfo,  sizeof(METEFACEINFO));
        InnerParam.DoseMax = FaceInfo.In_Max;
        InnerParam.DoseTotal = FaceInfo.In_Total;
        WriteParamToFlash();
    }
    else
    {
        if (t % 60 == 0)
        {
            // 一分钟读取一下时钟
            GetTimer(&cuTime);

            if (CheckRes.CommInterface == COMM_BT)
            { 
                BTGetBattary(1);
            }
        }
    }

    //BT_Test();
}




static WORD TestTime = 0;
const WORD MeteTimeCnt[] = {10,30,60,600,1800,3600};
void RefreshMeteCache(BYTE id)
{
    // 根据数据类型和记录时间刷新数据
    OutA_Cache[id] = FaceInfo.OutA_Cps_Rt;
    OutB_Cache[id] = FaceInfo.OutB_Cps_Rt;
    In_Cache[id]   = FaceInfo.In_Rt;
}

static BOOL AlarmContinue = FALSE;

void SaveAlarmData()
{
    if (AlarmContinue == FALSE)
    {
        AlarmContinue = TRUE;
        TestTime = 0;
        StorageRtData();
    }
    else
    {
        // 持续报警时强制每60秒存储
        // 平均值
        TestTime++;
        if (TestTime >= 60)
        {
            TestTime = 0;
            StorageAvData();
        }
    }

}

void SaveAllAlarm()
{
    if (HaveSsAlarm())
    {
        SaveAlarmData();
    }
    else
    {
        AlarmContinue = FALSE;
    }
}

void SaveInAlarm()
{
    if (HaveInAlarm())
    {
        SaveAlarmData();
    }
    else
    {
        AlarmContinue = FALSE;
    }
}

void SaveOutAlarm()
{
    if (HaveOutAlarm())
    {
        SaveAlarmData();
    }
    else
    {
        AlarmContinue = FALSE;
    }
}



void SaveMeteData()
{
    BYTE i;
    
    // 存储测试记录
    if (!Param.AlarmStore)
    {
        if (Param.ActiveMode)
        {
            if (MeteTimeCnt[Param.MeteTime-1] <= 60)
            {
                // 每秒缓存一次数据
                RefreshMeteCache(TestTime);
            }
            else if (MeteTimeCnt[Param.MeteTime-1] == 600)
            {
                // 10s 缓存一下
                if ((TestTime % 10) == 0)
                {
                    i = (BYTE)(TestTime / 10);
                    RefreshMeteCache(i);
                }
                
            }
            else if (MeteTimeCnt[Param.MeteTime-1] == 1800)
            {
                // 30s 缓存一下
                if ((TestTime % 30) == 0)
                {
                    i = (BYTE)(TestTime / 30);
                    RefreshMeteCache(i);
                }
            }
            else if (MeteTimeCnt[Param.MeteTime-1] == 3600)
            {
                // 60s 缓存一下
                if ((TestTime % 60) == 0)
                {
                    i = (BYTE)(TestTime / 60);
                    RefreshMeteCache(i);
                }
            }
            
            TestTime++;
            if (TestTime >= MeteTimeCnt[Param.MeteTime-1])
            {
                TestTime = 0;
                switch (Param.DataType)
                {
                    case DATA_AV:  StorageAvData();    break;
                    case DATA_MAX: StorageMaxData();   break;
                    case DATA_RT:  StorageRtData();    break;
                }
            }
            
        }
    }
    else
    {
        // 每秒刷新一次数据
        RefreshMeteCache(TestTime);
        
        switch (Param.TrigerType)
        {
            case TRG_ALL: SaveAllAlarm(); break;
            case TRG_IN:  SaveInAlarm();  break;
            case TRG_OUT: SaveOutAlarm(); break;
        }
    }
}

void RefreshCounter()
{
    #if 1
    switch (CheckRes.SenserType)
    {
        case SENSER_IN:   Face_InSenser();     break;
        case SENSER_PD:   Face_GmSenser();     break;   //      0x02    //水下高量程探头（PD）
        case SENSER_GM:   Face_GmSenser();     break;   //      0x03    //水下中高量程探头（GM）
        case SENSER_A_A:  Face_ABSenser();     break;   //      0x05    //α探头(仅测alphy)
        case SENSER_A_B:  Face_ABSenser();     break;   //      0x14    //α探头(仅测beta)
        case SENSER_A_AB: Face_G1Senser();     break;   //      0x24    //α探头(可测alphy+beta)
        case SENSER_B_B:  Face_BbSenser();     break;   //      0x05    //β探头(仅测beta)
        case SENSER_B_A:  Face_ABSenser();     break;   //      0x15    //β探头(仅测alphy)
        case SENSER_B_AB: Face_G1Senser();     break;   //      0x25    //β探头(可测alphy+beta)
        case SENSER_ABY:  Face_ABYSenser();    break;   //      0x6    //β探头(可测alphy+beta)
        case SENSER_G1:   Face_G1Senser();     break;   //      0x07    //地面探头1
        case SENSER_G2:   Face_G1Senser();     break;   //      0x08    //地面探头2
        case SENSER_Y:    Face_NaISerser();    break;   //      0x09    外部Y 探头
        case SENSER_LP:   Face_LPSenser();     break;
        case SENSER_LC:   Face_G1Senser();     break;
        case SENSER_NONE: Face_InSenser();     break;   //      无外接探头
    }  
    #else
    if (CheckRes.SenserType == SENSER_NONE)
    {
        Face_InSerser();
    }
    else
    {
        Face_OutSerser();
    }

    #endif
}

void RefreshAll()
{
    //CpuHighSpeed();
    
    Clear_DispBuf();

    ShowMeteTitle();
    
    RefreshCounter();

    ShowStatusBar();

    ShowDebugInfo();
    
    DisplayRefresh();

    //CpuLowSpeed();
}


// 局部刷新
void RefeshPart()
{
    static BYTE t = 0;
    BOOL rf = FALSE; 
    
    if (GetAlarm(ALARM_BAT_LOW) || VlotPerChanged())
    {
        Clear(88,0,40,7);
        ShowBatPower();

        rf = TRUE;
    }

    if (CounterChanged())
    {
        Clear(0,9,128,104);
        RefreshCounter();
        
        rf = TRUE;
    }
    
    t++;
    if (HaveAlarm())
    {
        // 状态栏
        Clear(4,114,120,12);
        ShowAlarm();

        rf = TRUE;        
    }
    else
    {
        if (HadAlarm)
        {
            // 报警结束后刷新一下时间
            Clear(4,114,120,12);
            ShowTime2(117);
        }
        HadAlarm = FALSE;

        if (t>=60)
        {
            t = 0;
            // 一分钟刷新一下时间
            Clear(4,114,120,12);
            ShowTime2(117);

            rf = TRUE;
        }
    }


    if (rf || DebugMode)
    {
        ShowDebugInfo();
        DisplayRefresh();
    }
    
}



// 电池百分比变化
BOOL VlotPerChanged()
{
    static BYTE BatPerHis = 0;
    BYTE BatPer;
    BatPer = GetVlotPer();

    if(BatPerHis == 0)
    {
        BatPerHis = BatPer;
    }
    
    if(DC_IN == 0)   // 外接电源
    {
        //保证电池百分百在充电时 只增不降
        if(BatPerHis < BatPer)
        {
            BatPerHis = BatPer;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        //保证电池百分百在充电时 只降不增
        if(BatPerHis > BatPer)
        {
            BatPerHis = BatPer;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    //return FALSE;
}

BOOL CounterChanged()
{
    static float CinHis = 0;
    static float CoutAHis = 0;
    static float CoutBHis = 0;
    
    // 计数发生变化
    //if ((WORD)(CHis*100) != (WORD)(FaceInfo.In_Rt*100))
    if (CinHis != FaceInfo.In_Cps)
    {
        CinHis = FaceInfo.In_Cps;
        return TRUE;
    }

    if (CoutAHis != FaceInfo.OutA_Cps_Rt)
    {
        CoutAHis = FaceInfo.OutA_Cps_Rt;
        return TRUE;
    }

    if (CoutBHis != FaceInfo.OutB_Cps_Rt)
    {
        CoutBHis = FaceInfo.OutB_Cps_Rt;
        return TRUE;
    }
    
    return FALSE;
}

BOOL DcChanged()
{
    static BYTE DcinHis = 0;
    if (DC_IN == 0)   // 外接电源
    {
        if (DcinHis == 0)
        {
            DcinHis = 1;
            return TRUE;
        }
    }
    else
    {
        if (DcinHis == 1)
        {
            DcinHis = 0;
            return TRUE;
        }
    }

    return FALSE;
}


void ShowDebugInfo()
{
    if (!DebugMode) return;
    
    float vol = GetVlotValu();
    
    Clear(0,9,128,7);
    memset(StrTemp,0,24);
    sprintf(StrTemp,"%.2fV %.0f",vol, FaceInfo.In_Cps);
    
    Show5X7String(0,9,StrTemp,0);
    ADC12_Start();
}

void RefreshMeasuFace(BOOL Force)
{   
    BOOL RfAll = Force;

    if (CheckRes.SenserType != SENSER_NONE)
    {
        // 外部探头计数
        GetOutCounter();
    }

    if (CheckRes.SenserType == SENSER_Y)
    {
        if (SwitchSenser())
        {
            RfAll = TRUE;
        }
    }

    if (DC_IN == 0)
    {
        RfAll = TRUE;
    }

    if (DcChanged())
    {
        RfAll = TRUE;
    }
    
    
    if (RfAll)  
    {
        RefreshAll();
    }
    else
    {
        RefeshPart();
    }

    Task_1s();
    
    #ifndef DEBUG
    SaveMeteData();
    #endif

    //Uart_Send_Data("hello\r\n",7);
    //Wifi_AtCmd_Test();
}

void EnterMeasuInter()
{
    currUI = MEASU;

    CanSleep = TRUE;   // 只有在主界面才能休眠
    
    RefreshMeasuFace(TRUE);

    //if (NeedSaveParam())
    //{
    //    PopHint("正在启用新参数");
    //    WriteParamToFlash();
    //}

    Sleep(200);    // 等按键音结束
    EnableAllAlarm();

    CpuLowSpeed();

    ADC12_Start();
    //idle();
}




void MeasuInterKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT:  case POWER: 
        {
            //RfAll = FALSE;
            RefreshMeasuFace(FALSE); 
            break;
        }

        #ifdef NEW_KEYBOARD
        case MODE:   InterQuickMenu();  break;
        #else
        case OKKEY:   InterQuickMenu();  break;
        #endif
        
        case RETURN:  ConfirmAlarm();     break;

        #ifdef NEW_KEYBOARD
        case DOWN:
        {
            //Param.AlarmEnable  = !Param.DevParam.AlarmEnable ;
            InnerParam.Sound = !InnerParam.Sound;
            //RfAll = TRUE;
            RefreshMeasuFace(TRUE);
        }
        break;
        #endif

        #ifdef NEW_KEYBOARD
        case OKKEY:
        #else
        case DOWN:  //按键 DOWN键
        #endif
        {
            if (InnerParam.Sound == 0)
            {
                StartBeep(200,0);
            }
            SaveDose();
        }
        break;

        #ifdef NEW_KEYBOARD
        case UP:
        #else
        case MODE:  // 开关背光
        #endif
        {
            if ((InnerParam.DevType == DEV_MPRP_S2L) && (DebugMode == TRUE))
            {
                if (FaceInfo.Gm == 0)
                {
                    FaceInfo.Gm = 1;
                    GM_HIGH;
                }
                else
                {
                    FaceInfo.Gm = 0;
                    GM_LOW;
                }
            }
            else
            {
                if ( (BkStatus) || (InnerParam.pisScreen == FALSE) )
                {
                    SetBkParam();
                    RefreshMeasuFace(TRUE);
                }
                else
                {
                    OpenBkLight();
                }
            }
        }
        break;
    }
}


const float maxarr[] = {0.0,10.0,100.0,1000.0,10000.0,100000.0,1000000.0,10000000.0,99999000000.0};
float GetMax(float val)
{
    float max;
    if (val < maxarr[1])
    {
        max = maxarr[1];
    }
    else if (val < maxarr[2])
    {
        max = maxarr[2];
    }
    else if (val < maxarr[3])   // 1mSv
    {
        max = maxarr[3];
    }
    else if (val < maxarr[4])
    {
        max = maxarr[4];
    }
    else if (val < maxarr[5])
    {
        max = maxarr[5];
    }
    else if (val < maxarr[6])  // 1Sv
    {
        max = maxarr[6];
    }
    else if (val < maxarr[7])
    {
        max = maxarr[7];
    }
    else
    {
        max = maxarr[8];
    }
    return max;
}


float GetShowMax(float val)
{
    static float maxhis = 0.0;
    float max;
    
    if (maxhis == maxarr[0])   // 0
    {
        max = GetMax(val);
    }
    else if (maxhis == maxarr[1])  // 10
    {
        if (val > maxarr[1])
        {
            max = GetMax(val);
        }
        else
        {
            max = maxhis;
        }
    }
    else if (maxhis == maxarr[2]) // 100
    {
        if (val < 2)
        {
            max = GetMax(val);
        }
        else if (val > maxarr[2])
        {
            max = GetMax(val);
        }
        else
        {
            max = maxhis;
        }
    }
    else if (maxhis == maxarr[3]) // 1000
    {
        if (val < 20)
        {
            max = GetMax(val);
        }
        else if (val > maxarr[3])
        {
            max = GetMax(val);
        }
        else
        {
            max = maxhis;
        }
    }
    else if (maxhis == maxarr[4])  // 10000
    {
        if (val < 200)
        {
            max = GetMax(val);
        }
        else if (val > maxarr[4])
        {
            max = GetMax(val);
        }
        else
        {
            max = maxhis;
        }
    }
    else if (maxhis == maxarr[5])  // 100000
    {
        if (val < 2000)
        {
            max = GetMax(val);
        }
        else if (val > maxarr[5])
        {
            max = GetMax(val);
        }
        else
        {
            max = maxhis;
        }
    } 
    else if (maxhis == maxarr[6])  // 1000000
    {
        if (val < 20000)
        {
            max = GetMax(val);
        }
        else if (val > maxarr[6])
        {
            max = GetMax(val);
        }
        else
        {
            max = maxhis;
        }
    }
    else if (maxhis == maxarr[7])  // 10000000
    {
        if (val < 200000)
        {
            max = GetMax(val);
        }
        else if (val > maxarr[7])
        {
            max = GetMax(val);
        }
        else
        {
            max = maxhis;
        }
    }
    else if (maxhis == maxarr[8])  // 99999000000
    {
        if (val < 2000000)  // 2Sv
        {
            max = GetMax(val);
        }
        else
        {
            max = maxhis;
        }
    }

    maxhis = max;
    return max;
}


void ShowProgress(BYTE y,float a1, float a2, float a3, float CuVal)
{
    BYTE x = 4;  // 必须大于3
    //const BYTE y = 82;
    const BYTE LEN = 120;
    
    const unsigned char Pro[] = {0x80,0x80,0x80,0x80};

    #if 1
    const BYTE coor[] = {  // 长度100 --> 120
      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
      0x80,0x00,0x00,0x02,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x40,0x00,0x00,0x01,
      0x80,0x00,0x00,0x02,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x40,0x00,0x00,0x01,
      0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x01,
    };
    #else
    const BYTE coor[] = {  // 长度100
      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,
      0x80,0x00,0x00,0x80,0x00,0x00,0x40,0x00,0x00,0x08,0x00,0x00,0x10,
      0x80,0x00,0x00,0x80,0x00,0x00,0x40,0x00,0x00,0x08,0x00,0x00,0x10,
      0x80,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x10,
    };
    #endif

    
    const BYTE UpArr[] = {
      0x08, 0x1C, 0x3E, 0x7F
    };

    // 空心三角形
    const BYTE EmArr[] = {
      //0x7F, 0x3E, 0x1C, 0x08
      0x08, 0x14, 0x22, 0x7F
    };

    WORD l1,l2,l3,Loc;
    float max;
    
    if (CuVal < 0.0)
    {
        CuVal = 0.0;
    }
    
    if ((a1 < 0.001) && (a2 < 0.001) && (a3 < 0.001))
    {
        Show12X12String(0,y-8,"报警参数异常",0);
        return;
    }

    if ((a1 > a2) || (a1 > a3) || (a2 > a3))
    {
        Show12X12String(0,y-8,"报警参数异常",0);
        return;
    }

    #if 0
    if ((CheckRes.SenserType == SENSER_NONE) || (CheckRes.SenserType == SENSER_Y))
    {
        max = GetShowMax(CuVal);
    }
    else
    {
        // 其他探头还没有调试
        max = 1000;
    }
    #else
    max = GetShowMax(CuVal);
    #endif
    
    //max = a3 * 1.2;
    l1 = (WORD)(a1 * LEN / max);
    l2 = (WORD)(a2 * LEN / max);
    l3 = (WORD)(a3 * LEN / max);
    
    Diplay(x,   y,  LEN,4,coor,0);
    if ((l1 != 0) && (l1 < LEN))
    {
        Diplay(x+l1-3,y+1,8,  4,UpArr,0);
    }
    
    if ((l2 != 0) && (l2 < LEN))
    {
        Diplay(x+l2-3,y+1,8,  4,EmArr,0);
    }

    if ((l3 != 0) && (l3 < LEN))
    {
        Diplay(x+l3-3,y+1,8,  4,UpArr,0);
    }

    if (CuVal >= max)
    {
        Loc = LEN;
    }
    else
    {
        Loc = (BYTE)(CuVal * LEN / max);
    }
    
    if (Loc > LEN)
    {
        Loc = LEN;
    }
    
    for(BYTE i = 0; i < Loc; i++)
    {
        Diplay(x+i,y-4,8,4,Pro,0);
    }

    #if 1
    // 最小值是0
    Show5X7String(2,y-12,"0",0);

    // 显示最大值和单位
    memset(StrTemp,0,24);
    if ((CheckRes.SenserType == SENSER_NONE) || 
         (CheckRes.SenserType == SENSER_Y)  ||
         (CheckRes.SenserType == SENSER_LP)  )
    {
        // 内部探头
        if (max > 1000000)
        {
            sprintf(StrTemp,"%.0f",max/1000000);
            x = 128- strlen(StrTemp)*6;
            Show5X7String(x,y-12,StrTemp,0);
        }
        else if (max > 1000)
        {
            sprintf(StrTemp,"%.0f",max/1000);
            x = 128- strlen(StrTemp)*6;
            Show5X7String(x,y-12,StrTemp,0);
        }
        else
        {
            sprintf(StrTemp,"%.0f",max);
            x = 128- strlen(StrTemp)*6;
            Show5X7String(x,y-12,StrTemp,0);
        }
    }
    else
    {
        //if (pnuc[CurRadIndex].Unit == UNIT_CPS)
        {
            sprintf(StrTemp,"%.0f",max);
            x = 128- strlen(StrTemp)*6;
            Show5X7String(x,y-12,StrTemp,0);
            //Show5X7String(94,88,UnitStr[pnuc[CurRadIndex].Unit],0);
        }
        #if 0
        else
        {
            sprintf(StrTemp,"%.0f",max);
            x = 128- strlen(StrTemp)*6;
            Show5X7String(x,y-12,StrTemp,0);
            //Show5X7String(94,88,UnitStr[pnuc[CurRadIndex].Unit],0);
        }
        #endif
    }
    #endif
}



#if 0

// Up: 报警上限  Dn: 下限   CuVal: 当前值
void ShowCurrProgress(float DnVal, float UpVal, float CuVal)
{
    const BYTE x = 4;  // 必须大于3
    const BYTE y = 100;
    
    const unsigned char Pro[] = {0x80,0x80,0x80,0x80};
    const BYTE coor[] = {  // 长度100
      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,
      0x80,0x00,0x00,0x80,0x00,0x00,0x40,0x00,0x00,0x08,0x00,0x00,0x10,
      0x80,0x00,0x00,0x80,0x00,0x00,0x40,0x00,0x00,0x08,0x00,0x00,0x10,
      0x80,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x10,
    };

    
    const BYTE UpArr[] = {
      0x08, 0x1C, 0x3E, 0x7F
    };

    const BYTE DnArr[] = {
      0x7F, 0x3E, 0x1C, 0x08
    };

    BYTE Dn,Up,Loc;
    float max;
    
    if (CuVal < 0.0)
    {
        CuVal = 0.0;
    }
    
    if ((DnVal < 0.01) && (UpVal < 0.01))
    {
        Show12X12String(0,94,"报警参数异常",0);
        return;
    }
    else if (DnVal < 0.01)
    {
        max = UpVal * 100 / 50;
        Up = (BYTE)(UpVal * 100 / max);
        Dn = 0;
    }
    else if (UpVal < 0.01)
    {
        max = DnVal * 100 / 50;
        Dn = (BYTE)(DnVal * 100 / max);
        Up = 0;
    }
    else
    {
        max = UpVal * 100 / 50;
        Up = (BYTE)(UpVal * 100 / max);
        Dn = (BYTE)(DnVal * 100 / max);
    }

    if (CuVal >= max)
    {
        Loc = 100;
    }
    else
    {
        Loc = (BYTE)(CuVal * 100 / max);
    }
    
    Diplay(x,   y,  104,4,coor,0);
    if (Up != 0)
    {
        Diplay(x+Up-3,y+1,8,  4,UpArr,0);
    }
    if (Dn != 0)
    {
        Diplay(x+Dn-3,y-8,8,  4,DnArr,0);
    }
    
    if (Loc > 100)
    {
        Loc = 100;
    }
    
    for(BYTE i = 0; i < Loc; i++)
    {
        Diplay(x+i,y-4,8,4,Pro,0);
    }

    // 最小值是0
    Show5X7String(2,104,"0",0);

    memset(StrTemp,0,24);
    // 显示最大值和单位
    if ((CheckRes.SenserType == SENSER_NONE) || (CheckRes.SenserType == SENSER_Y))
    {
        // 内部探头
        if (max >= 1000000)
        {
            sprintf(StrTemp,"%0.2f",max/1000000);
            Show5X7String(92,104,StrTemp,0);
            Show5X7String(92,88,"Sv/h",0);
        }
        else if (max >= 1000)
        {
            sprintf(StrTemp,"%0.2f",max/1000);
            Show5X7String(92,104,StrTemp,0);
            Show5X7String(92,88,"mSv/h",0);
        }
        else
        {
            sprintf(StrTemp,"%0.2f",max);
            Show5X7String(92,104,StrTemp,0);
            Show5X7String(92,88,"uSv/h",0);
        }
    }
    else
    {
        if (pnuc[CurRadIndex].Unit == UNIT_CPS)
        {
            sprintf(StrTemp,"%0.0f",max);
            Show5X7String(94,104,StrTemp,0);
            Show5X7String(94,88,UnitStr[pnuc[CurRadIndex].Unit],0);
        }
        else
        {
            sprintf(StrTemp,"%0.2f",max);
            Show5X7String(94,104,StrTemp,0);
            Show5X7String(94,88,UnitStr[pnuc[CurRadIndex].Unit],0);
        }
    }
    
}



//垂直的总体进度
void ShowTotalProgress(DWORD Max, DWORD Loc)
{
    const BYTE x = 118; 
    const BYTE y = 10; 
    const BYTE Pro[] = {0x3F,0x21};
    BYTE i;

    if (Loc > Max)
    {
        SetAlarm(ALARM_YT);
        Loc = Max;
    }
    else
    {
        ClrAlarm(ALARM_YT);
    }

    BYTE j = (BYTE)(100*Loc / Max);

    
    
    Diplay(x,y,8,1,&Pro[0],0);
    for (i=1;i<100-j;i++)
    {
        Diplay(x,y+i,8,1,&Pro[1],0);
    }

    for (i=100-j;i<100;i++)
    {
        Diplay(x,y+i,8,1,&Pro[0],0);
    }
    Diplay(x,y+100,8,1,&Pro[0],0);
}
#endif


//----------- 记录测量数据   --------------
DS1337_QTIME StorageTime;
QStorageData StorageData;
CMPTIME CmpTime;

void Cache_CurrDataBlock()
{
    #ifdef DATA_IN_INNER_FALSH
    DWORD block, Offset;
    if (InnerParam.StorageNum == DATA_COUNT)
    {
        // 存满了才需要循环存储
        if ((InnerParam.LastData == 0) || (InnerParam.LastData == DATA_COUNT))
        {
            block = DATA_COUNT/32 - 1; //63; // 最后一块
        }
        else
        {
            block = InnerParam.LastData / 32;
        }
        Offset = FLASH_DATA_START_ADDR + block * 512;
        memcpy((BYTE *)Data_Cache,(BYTE *)Offset,512);
    }
    #endif
}

void Cache_CurrLogBlock()
{
    #if 0
    // 日志不用循环存储了
    if (InnerParam.LogNum == LOG_COUNT)
    {
        WORD block = InnerParam.LastLog / 64;
        DWORD Offset = FLASH_LOG_START_ADDR + block * 512;
        memcpy((BYTE *)Log_Cache,(BYTE *)Offset,512);
    }
    #endif
}

// 保存前一块数据
void Save_PreBlock()
{    
    #ifdef DATA_IN_INNER_FALSH
    DWORD Offset;

    if (InnerParam.LastData == 0)
    {
        Offset = FLASH_DATA_START_ADDR + (DATA_COUNT-32) * sizeof(QStorageData);
    }
    else
    {
        Offset = FLASH_DATA_START_ADDR + (InnerParam.LastData-32) * sizeof(QStorageData);
    }
    
    Erase_Block((BYTE *)Offset);
    Write_Data_Seg((BYTE *)Offset, (BYTE *)Data_Cache, 512);     
    #endif
}


void Save_CurrBlock()
{
    #ifdef DATA_IN_INNER_FALSH
    DWORD Offset;
    WORD bk;
    if (InnerParam.StorageNum == DATA_COUNT)
    {
        bk = InnerParam.LastData / 32;
        
        Offset = FLASH_DATA_START_ADDR + bk * 32 * sizeof(QStorageData);
        
        
        Erase_Block((BYTE *)Offset);
        Write_Data_Seg((BYTE *)Offset, (BYTE *)Data_Cache, 512);
    }
    #endif
}

#ifdef DATA_IN_INNER_FALSH

void Loop_StorageData()
{
    WORD loc;
    WORD of;
    DWORD Offset;

    Offset = FLASH_DATA_START_ADDR + InnerParam.LastData * sizeof(QStorageData);
    
    if (InnerParam.LastData % 32 == 0)
    {
        // 先保存前一个块
        Save_PreBlock();
        
        
        // 缓存当前块
        memcpy((BYTE *)Data_Cache,(BYTE *)Offset,512);
    }

    loc = InnerParam.LastData % 32;
    of =  loc*sizeof(QStorageData);
    memcpy(&Data_Cache[of], (BYTE *)&StorageData, sizeof(QStorageData));

    InnerParam.LastData ++;
    
}
#endif

void StorageLog(BYTE Event)
{
    #ifndef DEBUG
    SYSLOG data;
    
    data.LogId = Event;
    
    GetTimer(&curtime);

    data.year = (BYTE)(curtime.year%100);
    data.month = curtime.month;
    data.day = curtime.day;
    data.hour = curtime.hour;
    data.minute = curtime.minute;
    data.second = curtime.second;
    
    data.ucCheck = GetVerfity((unsigned char*)&data,sizeof(data));

    if (InnerParam.LogNum < LOG_COUNT)
    {
        #ifdef LOG_IN_INNER_FALSH
        DWORD Offset = FLASH_LOG_START_ADDR + InnerParam.LogNum* sizeof(SYSLOG);
        Write_Data_Seg((unsigned char *)(Offset),
                         (unsigned char *)&data,
                         sizeof(SYSLOG));
        #else
        WORD Offset = LOG_OFFSET + InnerParam.LogNum* sizeof(SYSLOG);
        _DINT();
        BYTE *Data = (unsigned char *)&data; 
        for (int i=0;i<sizeof(SYSLOG);i++)
        {
            Eeprom_WriteByte(Offset+i,*(Data+i));
        }
        _EINT();
        #endif
        InnerParam.LogNum++;
        ClrAlarm(ALARM_LOG_FULL);
    }
    else
    {
        SetAlarm(ALARM_LOG_FULL);
    }
    #endif
}


 

void SetVal1(float val)
{
    DWORD lv ;  // 整数部分
    BYTE  rv;
    
    lv = (DWORD)val;  // 整数部分
    rv = ((DWORD)(round(val*100)))%100;  // 小数部分
    
    if (lv >= 0x10000)
    {
        StorageData.Value1 = lv % 0x10000;
        StorageData.Ext1 = (1<<7) | rv;  // 两位小数
    }
    else
    {
        StorageData.Value1 = lv % 0x10000;
        StorageData.Ext1 = rv;  // 两位小数
    }
}

void SetVal2(float val)
{
    DWORD lv;  // 整数部分
    BYTE  rv;  // 小数部分

    lv = (DWORD)val;  // 整数部分
    rv = ((DWORD)(round(val*100)))%100;  // 小数部分
    
    if (lv >= 0x10000)
    {
        StorageData.Value2 = (WORD)(lv % 0x10000);
        StorageData.Ext2 = (1<<7) | rv;  // 两位小数
    }
    else
    {
        StorageData.Value2 = (WORD)(lv % 0x10000);
        StorageData.Ext2 = rv;  // 两位小数
    }
}

void SetInAlarm(float val)
{
    //
    if (val >= 1000000)
    {
        StorageData.AlarmValue = (WORD)(val / 1000000);
        StorageData.Unit |= SUNIT_Sv_h << 4;
    }
    else if (val >= 1000)
    {
        StorageData.AlarmValue = (WORD)(val / 1000);
        StorageData.Unit |= SUNIT_mSv_h << 4;
    }
    else
    {
        StorageData.AlarmValue = (WORD)val;
        StorageData.Unit |= SUNIT_uSv_h << 4;
    }
}


// 实时剂量
void SetInValue1(float val)
{
    if (val >= 1000000)
    {
        SetVal1(val / 1000000);
        StorageData.Unit |= SUNIT_Sv_h << 2;
    }
    else if (val >= 1000)
    {
        SetVal1(val / 1000);
        StorageData.Unit |= SUNIT_mSv_h << 2;
    }
    else
    {
        SetVal1(val);
        StorageData.Unit |= SUNIT_uSv_h << 2;
    }
}


// 累计剂量
void SetInValue2(float val)
{
    if (val >= 1000000)
    {
        SetVal2(val / 1000000);
        StorageData.Unit |= SUNIT_Sv ;
    }
    else if (val >= 1000)
    {
        SetVal2(val / 1000);
        StorageData.Unit |= SUNIT_mSv;
    }
    else
    {
        SetVal2(val);
        StorageData.Unit |= SUNIT_uSv ;
    }
}

void SetOutAlarm(float val)
{
    BYTE ut;
    StorageData.AlarmValue = (WORD)val;
    switch (pnuc[CurRadIndex].Unit)
    {
        case UNIT_CPS:    ut = SUNIT_cps;    break;
        case UNIT_Bq:     ut = SUNIT_Bq;     break;
        case UNIT_Bq_cm2: ut = SUNIT_Bq_cm2; break;
    }
    StorageData.Unit |= ut << 4;
}

// 外部探头
void SetOutValue1(float val)
{
    BYTE ut;
    SetVal1(val);
    switch (pnuc[CurRadIndex].Unit)
    {
        case UNIT_CPS:    ut = SUNIT_cps;    break;
        case UNIT_Bq:     ut = SUNIT_Bq;     break;
        case UNIT_Bq_cm2: ut = SUNIT_Bq_cm2; break;
    }
    StorageData.Unit |= ut << 2;
}

// 内部探头实时剂量
void SetOutValue2(float val)
{
    if (val >= 1000000)
    {
        SetVal2(val / 1000000);
        StorageData.Unit |= SUNIT_Sv_h;
    }
    else if (val >= 1000)
    {
        SetVal2(val / 1000);
        StorageData.Unit |= SUNIT_mSv_h;
    }
    else
    {
        SetVal2(val);
        StorageData.Unit |= SUNIT_uSv_h;
    }
}


void SetMeterTime()
{
    GetTimer(&StorageTime);

    CmpTime.y = StorageTime.year%100;
    CmpTime.m = StorageTime.month;
    CmpTime.d = StorageTime.day;
    CmpTime.h = StorageTime.hour;
    CmpTime.t = StorageTime.minute;
    CmpTime.s = StorageTime.second;

    StorageData.Time = CmpTime.Time;
}

float GetVal1(QStorageData *data)
{
    float ret;
    ret = (float)data->Value1;
    if (data->Ext1 & 0x80)
    {
        ret += 0x10000;
    }

    ret += ((float)(data->Ext1 & 0x7F)) / 100;
    return ret;
}

float GetVal2(QStorageData *data)
{
    float ret;
    ret = (float)data->Value2;
    if (data->Ext2 & 0x80)
    {
        ret += 0x10000;
    }

    ret += ((float)(data->Ext2 & 0x7F)) / 100;
    return ret;
}

BYTE GetInUnit0(QStorageData *data)
{
    BYTE unit;
    unit = (data->Unit >> 4) & 0x3;
    switch(unit)
    {
        case SUNIT_uSv_h: return UNIT_uSv_h;
        case SUNIT_mSv_h: return UNIT_mSv_h;
        case SUNIT_Sv_h:  return UNIT_Sv_h;
    }
    return 0xFF;
}


BYTE GetInUnit1(QStorageData *data)
{
    BYTE unit;
    unit = (data->Unit >> 2) & 0x3;
    switch(unit)
    {
        case SUNIT_uSv_h: return UNIT_uSv_h;
        case SUNIT_mSv_h: return UNIT_mSv_h;
        case SUNIT_Sv_h:  return UNIT_Sv_h;
    }
    return 0xFF;
}

BYTE GetInUnit2(QStorageData *data)
{
    BYTE unit;
    unit = data->Unit & 0x3;
    
    switch(unit)
    {
        case SUNIT_uSv: return UNIT_uSv;
        case SUNIT_mSv: return UNIT_mSv;
        case SUNIT_Sv:  return UNIT_Sv;
    }
    return 0xFF;
}

BYTE GetOutUnit0(QStorageData *data)
{
    BYTE unit;
    unit = (data->Unit >> 4) & 0x3;
    switch(unit)
    {
        case SUNIT_cps:     return UNIT_CPS;
        case SUNIT_Bq:      return UNIT_Bq;
        case SUNIT_Bq_cm2:  return UNIT_Bq_cm2;
    }
    return 0xFF;
}


BYTE GetOutUnit1(QStorageData *data)
{
    BYTE unit;
    unit = (data->Unit >> 2) & 0x3;
    switch(unit)
    {
        case SUNIT_cps:     return UNIT_CPS;
        case SUNIT_Bq:      return UNIT_Bq;
        case SUNIT_Bq_cm2:  return UNIT_Bq_cm2;
    }
    return 0xFF;
}

BYTE GetOutUnit2(QStorageData *data)
{
    BYTE unit;
    unit = (data->Unit) & 0x3;
    switch(unit)
    {
        case SUNIT_uSv_h: return UNIT_uSv_h;
        case SUNIT_mSv_h: return UNIT_mSv_h;
        case SUNIT_Sv_h:  return UNIT_Sv_h;
    }
    return 0xFF;
}


void SetAvData()
{
    BYTE i,m;

    DataInfo.In_Av = 0;
    DataInfo.OutA_Cps_Av = 0;
    DataInfo.OutB_Cps_Av = 0;

    if (MeteTimeCnt[Param.MeteTime-1] <= 60)
    {
        m = MeteTimeCnt[Param.MeteTime-1];
    }
    else
    {
        m = 60;
    }
    
    for (i=0;i<m;i++)
    {
        DataInfo.In_Av        += In_Cache[i];
        DataInfo.OutA_Cps_Av  += OutA_Cache[i];
        DataInfo.OutB_Cps_Av  += OutB_Cache[i];
    }

    DataInfo.In_Av         /= m;
    DataInfo.OutA_Cps_Av   /= m;
    DataInfo.OutA_Cur_Av   = CpsUnit(DataInfo.OutA_Cps_Av,
                                       pnuc[CurRadIndex].Unit);
    DataInfo.OutB_Cps_Av   /= m;
    DataInfo.OutB_Cur_Av   = CpsUnit(DataInfo.OutB_Cps_Av,
                                       pnuc[CurRadIndex].Unit);
}

void StorageAvData()
{
    SetAvData();
    
    memset(&StorageTime,0,sizeof(StorageTime));
    memset(&StorageData,0,sizeof(StorageData));
    
    
    if (CheckRes.SenserType == SENSER_NONE)
    {
        // 只有内部探头
        StorageData.NucName = 0;
        StorageData.Channel = SENSER_IN;
        
        SetInAlarm(Param.InSenser.Alarm.A2);
        SetInValue1(DataInfo.In_Av);
        SetInValue2(FaceInfo.In_Total);   // 内部探头时把累计剂量存放在这里
    }
    else if (CheckRes.SenserType == SENSER_Y)
    {
        StorageData.NucName = 0;
        StorageData.Channel = SENSER_Y;
        
        SetInAlarm(Param.InSenser.Alarm.Y2);
        SetInValue1(DataInfo.OutA_Cps_Av);
        SetInValue2(DataInfo.OutB_Cps_Av); 
    }
    else if ( (CheckRes.SenserType == SENSER_LP) ||
               (CheckRes.SenserType == SENSER_GM) ||
               (CheckRes.SenserType == SENSER_PD) )
    {
        StorageData.NucName = 0;
        StorageData.Channel = CheckRes.SenserType;
        
        SetInAlarm(Senser.Alarm.Y2);
        SetInValue1(DataInfo.OutA_Cps_Av);
        SetInValue2(DataInfo.OutB_Cps_Av); 
    }
    else if (CheckRes.SenserType == SENSER_LC)
    {

        StorageData.NucName = 0;
        StorageData.Channel = CheckRes.SenserType; 
        //start:change by zhaozhenxiang,20190912
        SetInAlarm(Senser.Alarm.A2);
        SetInValue1(DataInfo.OutA_Cps_Av);
        SetInValue2(DataInfo.OutB_Cps_Av); 
        /*
        SetOutAlarm(CpsUnit(pnuc[0].AlarmThres,
                               pnuc[0].Unit)
                     );
        
        SetOutValue1(DataInfo.OutA_Cps_Av);
        SetOutValue2(DataInfo.OutB_Cps_Av); 
        */
        //end:change by zhaozhenxiang,20190912
    }
    else
    {
        StorageData.NucName = Senser.Param.Hn;
        StorageData.Channel = CheckRes.SenserType;  
        
        SetOutAlarm(CpsUnit(pnuc[CurRadIndex].AlarmThres,
                               pnuc[CurRadIndex].Unit)
                     );
        
        SetOutValue1(DataInfo.OutA_Cur_Av);
        SetOutValue2(DataInfo.In_Av); 
    }
    
    SetMeterTime();
    StorageData.ucCheck = GetVerfity((unsigned char*)&StorageData,sizeof(StorageData));

    StorageMeterData();
}


void SetMaxData()
{
    BYTE i,m;

    DataInfo.In_Max= 0;
    DataInfo.OutA_Cps_Max = 0;
    DataInfo.OutB_Cps_Max = 0;

    if (MeteTimeCnt[Param.MeteTime-1] <= 60)
    {
        m = MeteTimeCnt[Param.MeteTime-1];
    }
    else
    {
        m = 60;
    }
    
    for (i=0;i<m;i++)
    {
        if (DataInfo.In_Max  < In_Cache[i])
        {
            DataInfo.In_Max = In_Cache[i];
        }

        if (DataInfo.OutA_Cps_Max  < OutA_Cache[i])
        {
            DataInfo.OutA_Cps_Max  = OutA_Cache[i];
        }

        if (DataInfo.OutB_Cps_Max  < OutB_Cache[i])
        {
            DataInfo.OutB_Cps_Max  = OutB_Cache[i];
        }
    }
    
    DataInfo.OutA_Cur_Max   = CpsUnit(DataInfo.OutA_Cps_Max,
                                       pnuc[CurRadIndex].Unit);
    
    DataInfo.OutB_Cur_Max   = CpsUnit(DataInfo.OutB_Cps_Max,
                                       pnuc[CurRadIndex].Unit);
}


void StorageMaxData()
{
    SetMaxData();
    
    memset(&StorageTime,0,sizeof(StorageTime));
    memset(&StorageData,0,sizeof(StorageData));
    
    
    if (CheckRes.SenserType == SENSER_NONE)
    {
        // 只有内部探头
        StorageData.NucName = 0;
        StorageData.Channel = SENSER_IN;
        
        SetInAlarm(Param.InSenser.Alarm.A2);
        SetInValue1(DataInfo.In_Max);
        SetInValue2(FaceInfo.In_Total);   // 内部探头时把累计剂量存放在这里
    }
    else if (CheckRes.SenserType == SENSER_Y)
    {
        StorageData.NucName = 0;
        StorageData.Channel = SENSER_Y;
        
        SetInAlarm(Param.InSenser.Alarm.Y2);
        SetInValue1(DataInfo.OutA_Cps_Max);
        SetInValue2(DataInfo.OutB_Cps_Max); 
    }
    else if ( (CheckRes.SenserType == SENSER_LP) ||
               (CheckRes.SenserType == SENSER_GM) ||
               (CheckRes.SenserType == SENSER_PD) )
    {
        StorageData.NucName = 0;
        StorageData.Channel = CheckRes.SenserType;
        
        SetInAlarm(Senser.Alarm.Y2);
        SetInValue1(DataInfo.OutA_Cps_Max);
        SetInValue2(DataInfo.OutB_Cps_Max); 
    }
    else if (CheckRes.SenserType == SENSER_LC)
    {

        StorageData.NucName = 0;
        StorageData.Channel = CheckRes.SenserType; 

        SetOutAlarm(CpsUnit(pnuc[0].AlarmThres,
                               pnuc[0].Unit)
                     );
        
        SetOutValue1(DataInfo.OutA_Cps_Max);
        SetOutValue2(DataInfo.OutB_Cps_Max); 
    }
    else
    {
        StorageData.NucName = Senser.Param.Hn;
        StorageData.Channel = CheckRes.SenserType;  
        
        SetOutAlarm(CpsUnit(pnuc[CurRadIndex].AlarmThres,
                               pnuc[CurRadIndex].Unit)
                     );
        
        SetOutValue1(DataInfo.OutA_Cur_Max);
        SetOutValue2(DataInfo.In_Max);  
    }
    
    SetMeterTime();
    StorageData.ucCheck = GetVerfity((unsigned char*)&StorageData,sizeof(StorageData));

    StorageMeterData();
}

void StorageRtData()
{
    memset(&StorageTime,0,sizeof(StorageTime));
    memset(&StorageData,0,sizeof(StorageData));
    
    
    if (CheckRes.SenserType == SENSER_NONE)
    {
        // 只有内部探头
        StorageData.NucName = 0;
        StorageData.Channel = SENSER_IN;
        
        SetInAlarm(Param.InSenser.Alarm.Y2);
        SetInValue1(FaceInfo.In_Rt);
        SetInValue2(FaceInfo.In_Total);   // 内部探头时把累计剂量存放在这里
    }
    else if (CheckRes.SenserType == SENSER_Y)
    {
        StorageData.NucName = 0;
        StorageData.Channel = SENSER_Y;
        
        SetInAlarm(Param.InSenser.Alarm.Y2);
        SetInValue1(FaceInfo.OutA_Cps_Rt);
        SetInValue2(FaceInfo.OutB_Cps_Rt); 
    }
    else if ((CheckRes.SenserType == SENSER_LP) ||
               (CheckRes.SenserType == SENSER_GM) ||
               (CheckRes.SenserType == SENSER_PD) )
    {
        StorageData.NucName = 0;
        StorageData.Channel = CheckRes.SenserType;
        
        SetInAlarm(Senser.Alarm.Y2);
        SetInValue1(FaceInfo.OutA_Cps_Rt);
        SetInValue2(FaceInfo.OutB_Cps_Rt); 
    }
    else if (CheckRes.SenserType == SENSER_LC)
    {

        StorageData.NucName = 0;
        StorageData.Channel = CheckRes.SenserType; 

        SetOutAlarm(CpsUnit(pnuc[0].AlarmThres,
                               pnuc[0].Unit)
                     );
        
        SetOutValue1(FaceInfo.OutA_Cps_Rt);
        SetOutValue2(FaceInfo.OutB_Cps_Rt); 
    }
    else
    {
        StorageData.NucName = Senser.Param.Hn;
        StorageData.Channel = CheckRes.SenserType;  
        
        SetOutAlarm(CpsUnit(pnuc[CurRadIndex].AlarmThres,
                               pnuc[CurRadIndex].Unit)
                     );
        
        SetOutValue1(FaceInfo.OutB_Cur_Rt);
        SetOutValue2(FaceInfo.In_Rt);  
    }
    
    SetMeterTime();
    StorageData.ucCheck = GetVerfity((unsigned char*)&StorageData,sizeof(StorageData));

    StorageMeterData();
}


#ifdef DATA_IN_INNER_FALSH
void StorageMeterData()
{   
    // Flash 情况
    // 512BYTE/BLOCK  -- BACKC 32K  -- 64BLOCK
    // 每条记录16字节，每块可以存32条记录

    // Bank C Storage Test Data   32K  == 2048*16
    if (InnerParam.StorageNum < DATA_COUNT)
    {
        DWORD Offset = FLASH_DATA_START_ADDR + InnerParam.StorageNum * sizeof(QStorageData);
        Write_Data_Seg((unsigned char *)(Offset),
                         (unsigned char *)&StorageData,
                         sizeof(QStorageData));
        InnerParam.StorageNum++;
        InnerParam.LastData ++;

        Cache_CurrDataBlock();
        //ClrAlarm(ALARM_STOREAGE_FULL);
    }
    else
    {
        // 存满了要循环存储
        if (InnerParam.LastData == DATA_COUNT)
        {
            InnerParam.LastData = 0;
        }

        Loop_StorageData(); //((BYTE *)&StorageData);
        //SetAlarm(ALARM_STOREAGE_FULL);
    }
}
#else
void StorageMeterData()
{   
    DWORD Offset = DATA_OFFSET + (InnerParam.LastData % DATA_COUNT) * sizeof(QStorageData);
    
    _DINT();
    BYTE *Data = (unsigned char *)&StorageData; 
    for (int i=0;i<sizeof(QStorageData);i++)
    {
        Eeprom_WriteByte(Offset+i,*(Data+i));
    }
    _EINT();


    InnerParam.LastData ++;

    if (InnerParam.StorageNum < DATA_COUNT)
    {
        InnerParam.StorageNum++;
    }
    
}
#endif

#if 0
void Storage30Sec()
{
    QStorageData data;
    data.NucName = Param.RadioSelIndex;
    
    if(Param.RadioSelIndex == 0)
    {
        data.Channel = 3;
    }
    else
    {
        data.Channel = pnuc[Param.RadioSelIndex].Channel;
    }
    
    data.AlarmValue = (WORD)pnuc[Param.RadioSelIndex].AlarmThres;
    data.Value = (WORD)Senser.Conter.C1;
    data.Unit = pnuc[Param.RadioSelIndex].Unit;
    
    GetTimer(&curtime);

    data.year = curtime.year;
    data.month = curtime.month;
    data.day = curtime.day;
    data.hour = curtime.hour;
    data.minute = curtime.minute;
    data.second = curtime.second;
    data.ucReserve = 0xFF;
    data.ucCheck = GetVerfity((unsigned char*)&data,sizeof(data));
    ArrData[TimeIndex] = data;
}


void StaryArrData(unsigned char count)
{
    for(int i = 0; i < count; i++)
    {
        unsigned char ucUserFullBlock = Param.StorageNum/32;
        unsigned int uiAddrOffset = (Param.ucStartBlock + ucUserFullBlock)*32 + Param.ucStartInTailBlock;
        uiAddrOffset %= 1056; //128  1056
      
        Write_Data_Seg((unsigned char *)(FLASH_DATA_START_ADDR+uiAddrOffset*sizeof(QStorageData)),
                          (unsigned char *)&(ArrData[i]),
                          sizeof(QStorageData));
        Param.StorageNum++;
        if(Param.StorageNum > 1024) //96 1024
        {
            Param.ucStartInHeadBlock++;
            if(Param.ucStartInHeadBlock >= 32)
            {
                Erase_Data_Per_Seg((char *)(FLASH_DATA_START_ADDR+Param.ucStartBlock*512));
                Param.ucStartInHeadBlock = 0;
                Param.ucStartBlock++;
                if(Param.ucStartBlock > 32) // 3  32
                {
                    Param.ucStartBlock = 0;
                }
            }
            Param.StorageNum = 1024; //96   1024
        }
        ++Param.ucStartInTailBlock;
        if(Param.ucStartInTailBlock >= 32)  
        {
            Param.ucStartInTailBlock = 0;
        }
    }
}

#endif


static BYTE AlarmIndex = 0;
static BYTE AlarmNo = 1;
extern char StrHint[24];

void SelectAlarm();

void SkipAlarm()
{
    AlarmNo ++;
    SelectAlarm();
}

void ClearAlarm()
{
    DisableAlarm(AlarmIndex);
    SelectAlarm();
}

void ClearAllAlarm()
{
    BYTE i,j,cnt;
    cnt = ValidAlarmCnt();
    for (i=0;i<cnt;i++)
    {
        j = ValidAlarmIndex(1);
        DisableAlarm(j);
    }
}


void SelectAlarm()
{
    AlarmIndex = GetAlarmIndex(AlarmNo);
    if (AlarmIndex != 0xFF)
    {
        memset(StrHint,0,24);
        sprintf(StrHint,"清除:%s?",AlarmName[AlarmIndex]);
        ShowComfirm(StrHint,ClearAlarm,SkipAlarm);
    }
    else
    {
        EnterMeasuInter();
    }
}

void ConfirmAlarm()
{
    if (HaveValidAlarm())
    {
        //AlarmNo = 1;
        //SelectAlarm();
        ClearAllAlarm();
    }
}


