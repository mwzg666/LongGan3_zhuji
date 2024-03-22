#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <msp430.h>
#include "system.h"
#include "CommDataDefine.h"

#include "oled.h"
#include "Keyboard.h"
#include "mcp4725.h"
#include "GT21L16S2W.h"
#include "adc12.h"
#include "Ds1337.h"
#include "Mcp2510.h"
#include "Hvadc.h"
#include "Measinter.h"
#include "MainMenu.h"
#include "QuickMenu.h"
#include "Background.h"
#include "VerifPassword.h"
#include "OtherParam.h"
#include "DataStore.h"
#include "RadioSet.h"
#include "Pkur.h"
#include "main.h"
#include "Senser.h"
#include "SsParam.h"
#include "SsAlarm.h"
#include "Input.h"
#include "SelfCheck.h"
#include "I2c.h"
#include "SsSrv.h"
#include "Uart.h"
#include "Wifi.h"
#include "BlueTooth.h"
#include "DoseRate.h"
#include "TMeas.h"

extern WORD TimerACnt;
extern WORD SleepCnt;
extern INTERFACE currUI;
extern DS1337_QTIME cuTime;
extern QPara Param;
extern QInnerParam InnerParam;
extern char * const SenserName[];
extern BYTE SecondHis;  
extern SYSALARM SysAlarm;
extern char StrTemp[24];
extern METEFACEINFO FaceInfo;


const FACEHANDLE Face[] = 
{
    {MEASU,        MeasuInterKey,      1000},
    {MAIN,         MainMenuKey,        0},
    {QUICK,        QuickMenuSele,      0},
    {BACK,         BackResKey,         0},
    {PASSWD,       PasswordKey,        0},
    {ELECTEST,     ElecTestKey,        1000},
    {PHYTEST,      PhyTestKey,         1000},
    {BACKSET,      BackSetKey,         0},
    {SELRADUNIT,   RadUnitKey,         0},
    {CHGNPSW,      ChngPSWUIKey,       0},
        
    {MODSYSTM,     ModSysTimeKey,      0},
    {RADLIST,      RadSetKey,          0},
    {DETASET,      RadDetailSetKey,   0},
    {VIEWSTORE,    BrowseAllDataKey,  0},
    {DETAIL,       LookDetailKey,      0},
    {DATAMAN,      DataManKey,          0},
    {DATASET,      DataSetKey,          0},
    {EFFIDEMAR,    EffiDemarKey,        0}, 
    {DEMARING,     DemaringKey,         0xFFFF},   // 立刻返回
    {BACKTEST,     BackTestKey,         0xFFFF},
        
    {PARAMSET,     ParamSetUIKey,        0},
    {VIEWDATA,     ViewDataKey,          0},
    {BYTIMEVIEW,   ByTimeViewKey,        0},
    {SHOWBYTIME,   ShowByTimeKey,        0},
    {DETAILBYTIME, LookDetailByTimeKey, 0},
    {LOGO,         LogoFaceKey,           1500},
    {SELFCHECK,    SelfCheckKey,          5000},
    {SSPARAM,      SsParamKey,            0},
    {INBYTE,       InputByteKey,          0},
    {INWORD,       InputWordKey,          0},
        
    //{INDWORD,      InputDwordKey,         0},
    {INFLOAT,      InputFloatKey,         0},
    {INFLOATEX,    FloatExKey,            0},
    {INFLOATUNIT,  FloatUnitKey,          0},
    {SELUNIT,      SelectUnitKey,         0},
    {SSALARM,      SsAlarmKey,            0},
    {HINT,         HintKey,               3000},
    {SENSERWORK,   SsWorkSetKey,         0},
    {DEMARRES,     DemerResKey,          0},
    {TIMESEL,      TimeSelKey,           0},
    
    {METERTIME,    MeterTimeKey,         0},
    {DATADEL,      DataDelKey,           0},
    {VIEWALLLOG,   ViewAllLogKey,        0},
    {OUTSS,        OutSenserSetKey,      0},    
    {INSS,         InSenserSetKey,       0},   
    {COMFIRM,      ComfirmKey,            10000},
    {METERTYPE,    SelectTypeKey,        0},
    {TRIGERTYPE,   SelectTrigerKey,      0},
    {METERMAN,     MeterManKey,           0},
    {INTEST,       InSenserTestKey,      1000},
    {DEVTYPE,      DevTypeKey,            0},

    #ifdef USE_WIFI
    {WIFIMAN,      WifiManKey,             0},
    {WIFISTATUS,   WifiStatusKey,         0},
    {WIFIMODE,     WifiModeKey,           0},
    {WIFIAP,       WifiApKey,              0},
    {WIFISRV,      WifiSrvKey,             0},
    {WIFIDETAIL,   WifiDetailKey,          0},
    #endif
   
    {INCNTFACT,    InCntFactorKey,        0},
    {METERVIEW,    MeterViewKey,          0},
    {SHOWALM,      ShowAlmKey,             0},
    {BLUETOOTHSET, BTSetKey,               0},
    {BTSRV,        BTSenserKey,            2000},

    {RIGHT,        RightKey,               0},
    {MEASRES,      MeasResKey,              0},
    {COMLOG,       ComLogKey,              0},
    {NULLUI,       0,                      0},
};


BOOL DebugMode = FALSE;
//FLASH_IO RunLed;
FLASH_IO AlarmLed;
FLASH_IO BeepCtl;
WORD BkTimer = 0;       // 背光计数
BOOL BkStatus = FALSE;  // 背光开启状态
WORD EventTimer = 0;   // 事件定时器
PRESSKEY GetEvent(WORD ms)
{   
    PRESSKEY key;
    WORD   to;

    to = 0;
    if (InnerParam.MeauExitTime <= 60)
    {
        to = (WORD)InnerParam.MeauExitTime * 1000;
    }
    
    
    while(1)
    {
        SystemTask();
        
        key = GetKey();
        if (key != NONE)
        {
            #ifdef NEW_KEYBOARD
            // 主界面的 UP 键作为背光使能设置，因此不能打开背光
            if (currUI == MEASU)
            {
                if (key != UP)
                {
                    OpenBkLight();
                }
            }
            else
            {
                OpenBkLight();
            }
            #else
            OpenBkLight();
            #endif

            #ifndef NEW_KEYBOARD
            if (currUI == MEASU)
            {
                // 主界面只返回需要的按键
                if ((key == OKKEY) || (key == MODE) || 
                     (key == RETURN) || (key == DOWN)  )
                {
                    EventTimer = 0;  // 计数器清零
                    return key;
                }
            }
            else
            #endif
            {
                EventTimer = 0;  // 计数器清零
                return key;
            }
        }

        if (ms == 0xFFFF)  // 立刻返回
        {
            break; 
        }
        else if (ms == 0)  // 根据参数返回超时
        {
            // to == 0 时不返回
            if (to != 0)
            {
                if (EventTimer >= to)  
                {
                    break;
                }
            }
        }
        else 
        {
            // 使用固定的时间返回
            if (EventTimer >= ms) 
            {
                break;
            }
        }

        // 没事了就休眠
        idle();
    }

    EventTimer = 0;  // 计数器清零
    return TIMEOUT;
}




void Init_Gpio()
{
    #ifndef BATTERY_VER
    P1DIR &= ~BIT1;  //DC_IN
    #endif

    P3DIR |= BIT5;   //P8V_On
    
    P3DIR |= BIT2;   //HV_EN_L
    P7DIR |= BIT3;   //HV_EN_H

    P6DIR |= BIT0;   //PowerOff
    P6DIR &= ~BIT1;   //CHARGE_GREEN

    P5DIR |= BIT0 | BIT1;  // UART_SHDN  | CAN_SHDN
    P2DIR |= BIT7;  // GM Power  ---  高低量程切换  0 : 低 1:高
    #ifdef BATTERY_VER
    P3DIR |= BIT2;  // 高低量程切换  
    #endif

    P7DIR |= BIT6;  // Speaker
}


void Led_Init(void)
{
    #ifdef BATTERY_VER
    // LED_GREEN       -- P1.1
    P1DIR |= BIT1;   
    P1OUT &= ~(BIT1); 
    
    // LED_RED            -- P6.1
    P6DIR |= BIT1;
    P6OUT &= ~(BIT1);
    #endif
    // CHARGE_GREEN  -- P5.6
    // LED_RUN            -- P4.7
    // LED_ALARM        -- P4.6
    //

    P4DIR |= BIT6 | BIT7;
    P4OUT &= ~(BIT6 | BIT7); 
}


// -------Beep  ----------
void BeepOn(void)
{
    P7OUT |= BIT6;
}

void BeepOff(void)
{
    P7OUT &= ~BIT6;
}


void StartBeep(WORD OnTime, WORD OffTime)
{
    BeepCtl.Enable = TRUE;
    BeepCtl.Count = 0;
    BeepCtl.OnTime = OnTime;
    BeepCtl.OffTime = OffTime;
    BeepCtl.On = BeepOn;
    BeepCtl.Off = BeepOff;
    BeepCtl.Status = 1;
    BeepOn();
}

void StopBeep(void)
{
    BeepCtl.Enable = FALSE;
    BeepOff();
}



// -------Run Led ----------
void RunLedOn(void)
{
    #ifdef BATTERY_VER
    P1OUT |= BIT1;
    #else
    P4OUT |= BIT7;
    #endif
}

void RunLedOff(void)
{
    #ifdef BATTERY_VER
    P1OUT &= ~BIT1;
    #else
    P4OUT &= ~BIT7;
    #endif
}

#if 0
void StartRunLed(WORD OnTime, WORD OffTime)
{
    RunLed.Enable = TRUE;
    RunLed.Count = 0;
    RunLed.OnTime = OnTime;
    RunLed.OffTime = OffTime;
    RunLed.On = RunLedOn;
    RunLed.Off = RunLedOff;
    RunLed.Status = 1;
    RunLedOn();
}

void StopRunLed(void)
{
    RunLed.Enable = FALSE;
    RunLedOff();
}
#endif


// -------Alarm Led ----------
void AlarmLedOn(void)
{
    P4OUT |= BIT6;
}

void AlarmLedOff(void)
{
    P4OUT &= ~BIT6;
}


void StartAlarmLed(WORD OnTime, WORD OffTime)
{
    AlarmLed.Enable = TRUE;
    AlarmLed.Count = 0;
    AlarmLed.OnTime = OnTime;
    AlarmLed.OffTime = OffTime;
    AlarmLed.On = AlarmLedOn;
    AlarmLed.Off = AlarmLedOff;
    AlarmLed.Status = 1;
    AlarmLedOn();
}

void StopAlarmLed(void)
{
    AlarmLed.Enable = FALSE;
    AlarmLedOff();
}



void OpenBkLight()
{
    if (InnerParam.pisScreen)
    {
        BK_ON;
        BkTimer = 0;
        BkStatus = TRUE;
    }
}

void CloseBkLight()
{
    BK_OFF;
    BkStatus = FALSE;;
}

void BkLightCtl()
{
    if (InnerParam.BkTimeout != 0)
    {
        if ((BkTimer/1000) > InnerParam.BkTimeout)
        {
            CloseBkLight();
        }
    }
}

void SetBkParam()
{
    InnerParam.pisScreen= !InnerParam.pisScreen; 
    if (InnerParam.pisScreen)
    {
        OpenBkLight();
    }
    else
    {
        CloseBkLight();
    }
}


void ShowTitle()
{
    if (InnerParam.ComLog ==  XI_AN_HLT)
    {
        Show5X7String(20,14,"@Y",0);
        GT_Show16X16Char(28,10,"吊测仪主机",0); 
    }
    else
    {
        if (InnerParam.DevType == DEV_MPRP_S3)
        {
            GT_Show16X16Char(16,10,"长杆剂量率仪",0);  
        }
        else
        {
            #ifndef DG105
            GT_Show16X16Char(4,10,"便携式",0);  
            Show5X7String(53,14,"@Y",0);
            GT_Show16X16Char(60,10,"剂量率仪",0);  
            #else
            GT_Show16X16Char(0,10,"辐射剂量率测量仪",0);  
            #endif
        }
    }
}

void ShowComName()
{
    switch(InnerParam.ComLog)
    {
        case XI_AN_HLT:
            GT_Show12X12Char(6,100,"西安海利特电子设备",0);
            GT_Show12X12Char(12,114,"科技开发有限公司",0);
        break;

        case COM_MTS:
            GT_Show12X12Char(30,100,"成都麦特斯",0);
            GT_Show12X12Char(25,114,"科技有限公司",0);
        break;

        case COM_MW:
            GT_Show12X12Char(38,100,"成都迈为",0);
            GT_Show12X12Char(6,114,"核监测科技有限公司",0);
        break;

        case  XI_AN_262:
            GT_Show12X12Char(38,100,"西安中核",0);
            GT_Show12X12Char(19,114,"核仪器有限公司",0);
        break;
    }
}


void ShowVer(BOOL sTime)
{
    GT_Show12X12Char(20,44,"型号",0);
    memset(StrTemp,0,24);

    if (InnerParam.ComLog ==  XI_AN_HLT)
    {
        sprintf(StrTemp,":HLTM2");
    }
    else
    {
        #ifndef DG105
        if (InnerParam.DevType == DEV_MPRP_S3)
        {
            sprintf(StrTemp,":MNRM-%d",InnerParam.DevType);
        }
        else if (InnerParam.DevType == DEV_MPRP_S2L)
        {
            sprintf(StrTemp,":MPRP-S2L");
            //sprintf(StrTemp,":MPRP-S2D");
        }
        else if (InnerParam.DevType == DEV_MPRP_S2N)
        {
            sprintf(StrTemp,":MPRP-S2N");
        }
        else
        {
            #ifdef BATTERY_VER
                #ifdef NO_BT_WIFI 
                sprintf(StrTemp,":MPRP-S%dD",InnerParam.DevType);
                #else
                sprintf(StrTemp,":MPRP-S%dB",InnerParam.DevType);
                #endif
            #else
            sprintf(StrTemp,":MPRP-S%d",InnerParam.DevType);
            #endif
        }
        #else
        sprintf(StrTemp,":DG105");
        #endif
    }
    
    Show5X7String(46,47,StrTemp,0);
    
    GT_Show12X12Char(20,76,"版本",0);
    #ifdef NEW_KEYBOARD
        Show5X7String(46,60,"(V002R001)",0);    // 产品版本
    #else
        Show5X7String(46,60,"(V001R001)",0);    // 产品版本
    #endif

    #ifdef BATTERY_VER
        #ifdef DEBUG
        Show5X7String(46,79,":V2.8.2D",0);
        #else
        Show5X7String(46,79,":V2.8.2",0);
        #endif
    #else
        #ifdef DEBUG
        Show5X7String(46,79,":V2.1.14D",0);
        #else
        Show5X7String(46,79,":V2.1.14",0);
        #endif
    #endif

    if (sTime)
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%s %s",__DATE__, __TIME__);
        Show5X7String(0,89,StrTemp,0);
    }
}

void ShowTime(BYTE Line)
{
    GetTimer(&cuTime);
    sprintf(StrTemp,"%02d-%02d-%02d %02d:%02d:%02d",
                      cuTime.year,cuTime.month,cuTime.day,
                      cuTime.hour,cuTime.minute,cuTime.second);
    Show5X7String(4,Line,StrTemp,0);  
}

void ShowTime2(BYTE Line)
{
    
    sprintf(StrTemp,"%02d-%02d-%02d %02d:%02d",
                      cuTime.year,cuTime.month,cuTime.day,
                      cuTime.hour,cuTime.minute);
    Show5X7String(10,Line,StrTemp,0);  
}


// 操作提示
void ShowOptHint()
{
    #if 0
    const BYTE Up[] = {0x00,0x00,0x10,0x38,0x7C,0xFE,0x00,0x00};
    const BYTE Dn[] = {0x00,0x00,0xFE,0x7C,0x38,0x10,0x00,0x00};
    const BYTE En[] = {0x10,0x30,0x7C,0x34,0x14,0x44,0x7C,0x00};
    Clear(0,118,128,10);
    Diplay(8,120,8,8,Up,0);
    Diplay(16,120,8,8,Dn,0);
    Show5X7String(24,120,":SELECT",0);  
    
    Diplay(72,120,8,8,En,0);
    Show5X7String(80,120,":QUIT",0);  
    #endif
}

void ShowLogo()
{
    currUI = LOGO;
    Clear_DispBuf();
    
    //开机界面
    ShowTitle();
    ShowVer(FALSE);

    //GT_Show12X12Char(8,102,"正在自检",0); 
    //Show5X7Int(0,0,sizeof(SENSER),0);
    //Show7X8Int(0,10,sizeof(SENSER),0);
    //Show6X12Int(0,20,sizeof(SENSER),0);
    //ShowTime(120);
    ShowComName();
    
    DisplayRefresh();
}


void LogoFaceKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT:  SelfCheck(); break;
    }
}

void testcan()
{
    while(1)
    {
        Sleep(1000);
        FindSenser();
    }
}

#ifdef PWM_BEEP
void testbeep()
{
    //TBPwmInit('A',1,'P','P'); 
    //TBPwmInit('A',1,'P','P'); 
    BYTE key;
    int j=1,k=0;  
    
    j = 1; k = 1;
    while(1)
    {
        Clear(0,0,100,8);
        Show5X7Int(0,0,j,0);
        Show5X7Int(50,0,k,0);
        DisplayRefresh();

        
        P7OUT |= BIT6;
        TBPwmSetPeriod(j);    // 3K    
        //TBPwmSetDuty(1,k);  
        //TBPwmSetDuty(1,k);  
        //TBPwmSetPermill(1,k);
        TBPwmInit('A',1,'P','P'); 

        key = GetEvent(10000);
        if (key==RETURN) j++;
        if (key==OKKEY)  k++;

        BeepOff();
    }
    
    //TBPwmSetPeriod(128);    // 3K    
     
}
#endif

void HV_CTL(BYTE lv)
{
    #ifndef BATTERY_VER
    switch(lv)
    {
        case 0: HVL_OFF;  HVH_OFF;  break;
        case 1: HVL_ON;   HVH_OFF;  break;
        case 2: HVL_OFF;  HVH_ON;   break;
        case 3: HVL_ON;   HVH_ON;   break;
        default: HVL_OFF; HVH_OFF;  break;
    }
    #endif
}


void HwInit()
{
    Init_Gpio();

    #ifdef USE_WIFI
    Wifi_IoInit();
    #endif
    
    Led_Init();
    Can_IoInit();

    BeepOff();
    CAN_EN(1);
    PW_ON;
    P8V_ON;
    GM_LOW;
    UART_OFF;    // 暂时没有使用串口
    WIFI_OFF;
    #ifdef BATTERY_VER
    BT_ON;
    #endif
    HV_CTL(3);
    
    Init_ADC12();

    #ifndef DEBUG
    //PowerOn();    // 改到 boot 中按键开机了
    #endif

    CheckLed();

    DS1337_I2C_Init();
    
    KeyBoard_Init();

    I2C_Init();
    Param_Read();

    if (InnerParam.DevType == DEV_MPRP_S2N)
    {
        InnerParam.InnerSersor = FALSE;
    }
    
    
    memset(&FaceInfo, 0, sizeof(METEFACEINFO));
    //memcpy(&FaceInfo, &Param.MetInfo, sizeof(METEFACEINFO));
    FaceInfo.In_Max = InnerParam.DoseMax;
    FaceInfo.In_Total = InnerParam.DoseTotal;
    
    BK_ON;
        
    Oled_Init();
    Init_GT21L16S2W();  // 字库
    ShowLogo();

    #ifdef UART_DOSE
    InitUart();
    #endif

    #ifdef USE_WIFI
    Wifi_UartInit();
    Wifi_Reset();
    #endif

    BT_HwInit();

    MCP4725_OutVol(MCP4725_HV_ADDR,InnerParam.Fudu);

    BeepOn();

    ADC12_Start();
}

int main(void)
{    
    PRESSKEY Evt;
    BYTE i;
        
    SysInit();

    HwInit();

    RadInit();

    InitAlarm();

    Init_SelfCheck();
    
    //testcan();
    //testbeep();

    Init_Cache();
    Cache_CurrDataBlock();
    Cache_CurrLogBlock();
    //StorageLog(EVENT_STARTUP);

    ClearCounter();
    
    // 初始化逻辑定时器进入主循环
    InitLogicTimer();
    while(1)
    {
        i = 0;
        while(Face[i].Ef != 0)
        {
            if (currUI == Face[i].UiId)
            {
                break;
            }
            i++;
        }
        
        Evt = GetEvent(Face[i].Rt);
        Face[i].Ef(Evt);
    }
}

