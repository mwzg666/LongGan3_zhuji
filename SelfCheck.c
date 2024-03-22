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
#include "Eeprom.h"
#include "SelfCheck.h"
#include "Wifi.h"
#include "BlueTooth.h"
#include "DoseRate.h"

//extern WORD TimerACnt;
extern INTERFACE currUI;
extern DS1337_QTIME cuTime;
extern QPara Param;
extern WORD SleepCnt;
extern char * const SenserName[];
//extern FLASH_IO RunLed;
extern FLASH_IO AlarmLed;
extern FLASH_IO BeepCtl;
extern SENSER Senser;
extern QInnerParam InnerParam;
extern BYTE  RadCount;     // ��������
extern RADIO pnuc[25];      //����
extern BYTE  CurRadIndex;   // ����ʹ�ú��ص�����
extern BOOL InSenserEnable;
extern METEFACEINFO FaceInfo;

CHECKRES CheckRes;
BYTE SecondHis = 0;

BOOL HvValid()
{
    #if 0
    WORD Vol = (unsigned int)Get_HV_Vol();
    if ((Vol > Senser.Param.Hv * (1 + Senser.Param.Hd/100)) ||
        ((Vol < Senser.Param.Hv * (1 - Senser.Param.Hd/100))))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
    #else
    return TRUE;
    #endif
}

// No.1
void CheckBat(void)
{
    BeepOff();
    
    Clear_DispBuf();
    ShowTitle();
    ShowVer(FALSE);

            
    CheckRes.BatVol = GetVlotValu(); //Get_Bat_Vol()/(float)1000;
    if(CheckRes.BatVol < 6.4)
    {
        GT_Show12X12Char(8,102,"��ص�ѹ����",0);
        CheckRes.BatSt = FALSE;
        StorageLog(EVENT_BAT_LOW);
    }
    else
    {
        GT_Show12X12Char(8,102,"�����Լ�",0);
        Show5X7String(60,106,".",0);   
        CheckRes.BatSt = TRUE;
    }

    ShowTime(120);  
    SecondHis = cuTime.second;  // �����Լ�ʱ��
    
    DisplayRefresh();

    //ADC12_Start();  //Start AD for HV Check
}

//No.2
void Check_Hv(void)
{
    Clear_DispBuf();
    ShowTitle();
    ShowVer(FALSE);

    GT_Show12X12Char(8,102,"�����Լ�",0);
    Show5X7String(60,106,"..",0); 
    CheckRes.HvSt = TRUE;

    #if 0
    if (InnerParam.DevType == DEV_MPRP_S2L)
    {
        GT_Show12X12Char(8,102,"�����Լ�",0);
        Show5X7String(60,106,"..",0); 
        CheckRes.HvSt = TRUE;
    }
    else
    {
        CheckRes.HvVol = (unsigned int)Get_HV_Vol();
        
        if((CheckRes.HvVol < 450) || (CheckRes.HvVol > 550))
        {
            GT_Show12X12Char(8,102,"��ѹ�쳣",0);
            CheckRes.HvSt = FALSE;
            StorageLog(EVENT_HV_ERR);
            //CloseHv();
        }
        else
        {
            GT_Show12X12Char(8,102,"�����Լ�",0);
            Show5X7String(60,106,"..",0); 
            CheckRes.HvSt = TRUE;
        }
    }
    #endif

    ShowTime(120);
    DisplayRefresh();
}

//No.3
void CheckTimer()
{
    Clear_DispBuf();
    ShowTitle();
    ShowVer(FALSE);

    ShowTime(120);
    
    if (SecondHis == cuTime.second)
    {
        GT_Show12X12Char(8,102,"ʵʱ���쳣",0);
        CheckRes.TimerSt = FALSE;
        StorageLog(EVENT_RTC_ERR);
    }
    else
    {
        GT_Show12X12Char(8,102,"�����Լ�",0);
        Show5X7String(60,106,"...",0); 
        CheckRes.TimerSt = TRUE;
    }

    
    DisplayRefresh();
}

// No.2
void CheckLcd()  // ???
{
    Clear_DispBuf();
    ShowTitle();
    ShowVer(FALSE);

    
    StopAlarmLed();
    
    //if ()
    //{
    //    GT_Show12X12Char(8,102,"Һ���쳣",0);
    //}
    //else
    {
        GT_Show12X12Char(8,102,"�����Լ�",0);
        Show5X7String(60,106,"..",0); 
    }

    ShowTime(120);
    DisplayRefresh();
}


// No.4 -- ����WIFI �ˣ���Ϊ����
void CheckWifi()
{
    Clear_DispBuf();
    ShowTitle();
    ShowVer(FALSE);


    #ifdef USE_WIFI
    if (Wifi_AtCmd_Test())
    {
        GT_Show12X12Char(8,102,"�����Լ�",0);
        Show5X7String(60,106,"....",0); 
        CheckRes.WifiSt = TRUE;
        if (!Param.Wifi.Enable)
        {
            Wifi_ShutDown();
        }
        else
        {
            InitWifi();
        }
    }
    else
    {
        GT_Show12X12Char(8,102,"���������쳣",0);
        CheckRes.WifiSt = FALSE;
        Wifi_ShutDown();
    }
    #else  // BlueTooth
    
    GT_Show12X12Char(8,102,"�����Լ�",0);
    Show5X7String(60,106,"....",0); 
    
    #ifdef NO_BT_WIFI 
    CheckRes.WifiSt = FALSE;    
    #else
    if (BT_Init())
    {
        CheckRes.WifiSt = TRUE;
    }
    else
    {
        //GT_Show12X12Char(8,102,"���������쳣",0);
        CheckRes.WifiSt = FALSE;
    }
    #endif
    
    #endif

    ShowTime(120);
    DisplayRefresh();
}


void CheckCan()
{
    Clear_DispBuf();
    ShowTitle();
    ShowVer(FALSE);
    
    if (!CanBus_Init())
    {
        CheckRes.CanSt = FALSE;
        Show12X12String(8,102,"CanBus��ʼ��ʧ��",0);   
    }
    else
    {
        CheckRes.CanSt = TRUE;
        GT_Show12X12Char(8,102,"�����Լ�",0);
        Show5X7String(60,106,".....",0); 
    }

    ShowTime(120);
    DisplayRefresh();
}


#if 0
void CheckHzChip()
{
    Clear_DispBuf();
    ShowTitle();
    ShowVer();
    
    //if ()
    //{
    //    GT_Show12X12Char(8,102,"�ֿ��쳣",0);
    //}
    //else
    {
        GT_Show12X12Char(8,102,"�����Լ�",0);
        Show5X7String(60,106,".....",0); 
    }

    ShowTime(120);
    DisplayRefresh();
}
#endif

// No.6
void CheckEeprom()
{
    Clear_DispBuf();
    ShowTitle();
    ShowVer(FALSE);
    
    if (!Eeprom_Test())
    {
        CheckRes.E2St = FALSE;
        GT_Show12X12Char(8,102,"�ڲ��洢�쳣",0);
        StorageLog(EVENT_E2_ERR);
    }
    else
    {
        CheckRes.E2St = TRUE;
        GT_Show12X12Char(8,102,"�����Լ�",0);
        Show5X7String(60,106,"......",0); 
    }

    ShowTime(120);
    DisplayRefresh();
}



//No.7 --- �ⲿ̽ͷ

// �������ı���ֵд�����ر�����ֵ��
void GetAlarmThres()
{
    BYTE id;
    id = GetThresId();
    pnuc[CurRadIndex].AlarmThres = GetAlarmValEx(id-1);
}

void CheckSenser()
{
    Clear_DispBuf();
    ShowTitle();
    ShowVer(FALSE);
    ShowTime(120);


    if ((CheckRes.CanSt == TRUE) || (CheckRes.WifiSt == TRUE))
    {
        GT_Show12X12Char(8,102,"�����Լ�",0);
        Show5X7String(60,106,".......",0); 
        DisplayRefresh();

        CheckRes.SenserType = FindSenser();
    }
    else
    {
        CheckRes.SenserType = SENSER_NONE;
    }
        
       
    if (CheckRes.SenserType == SENSER_NONE)
    {
        Clear(8,102,128,12);
        GT_Show12X12Char(8,102,"�����̽ͷ",0);
        DisplayRefresh();
        
        CloseHv();

        #ifndef UART_DOSE
        P8V_OFF;
        #endif
        
        CAN_EN(0);
    }
    else
    {
        InitSenser();
        
        if (!GetSenserParam())
        {
            Clear(8,102,112,12);
            GT_Show12X12Char(8,102,"��ȡ̽ͷ����ʧ��",0);
            DisplayRefresh();
            Sleep(1000);
            CheckRes.SenserSt = FALSE;
            return;
        }

        Show5X7String(60,106,"........",0); 
        DisplayRefresh();

        if (!GetSenserAlarm())
        {
            Clear(8,102,112,12);
            GT_Show12X12Char(8,102,"��ȡ������ֵʧ��",0);
            DisplayRefresh();
            Sleep(1000);
            CheckRes.SenserSt = FALSE;
            return;
        }

        // ά���������ݿ�
        if (HaveRadSet())
        {
            if (!HndRadDataBase())
            {
                Clear(8,102,112,12);
                GT_Show12X12Char(8,102,"��ȡ��������ʧ��",0);
                DisplayRefresh();
                Sleep(1000);
                CheckRes.SenserSt = FALSE;
                return;
            }
                
            // �������ı���ֵд�����ر�����ֵ��
            // ��ģʽ�в���Ҫ��
            // GetAlarmThres();

            // ��ǰ����
            CurRadIndex = GetCurRadIndex();
            InnerParam.RadioSelIndex = CurRadIndex;
            //AyncAlarmThres(float Thres)
            
        }
        
        Show5X7String(60,106,".........",0); 
        DisplayRefresh();

        

        if (NeedHv())
        {
            OpenHv((WORD)Senser.Param.Hv);
        }
        else
        {
            CloseHv();
        }
        //Sleep(500);
        //ADC12_Start();

        
        //Show5X7String(60,106,"..........",0); 
    }

    CheckRes.SenserSt = TRUE;
    
}




void CheckLed()
{
    //StartRunLed(1000,1000);
    //RunLedOn();
    
    StartAlarmLed(200,200);
}

BOOL SelfCheckOk()
{
    if ((CheckRes.BatSt == FALSE) 
        || (CheckRes.SenserSt == FALSE) 
        // || (CheckRes.WifiSt == FALSE) 
        || (CheckRes.TimerSt == FALSE)
        || (CheckRes.E2St == FALSE))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void ShowCheckRes(void)
{
    currUI = SELFCHECK;
    
    Clear_DispBuf();
    //ShowTitle();
    
    GT_Show16X16Char(32,0,"�Լ���",0);
    
    Show12X12String(0,20,"�ⲿ̽����",0);
    Show5X7String(65,23,":",0);
    if (CheckRes.SenserType == SENSER_NONE)
    {
        Show12X12String(71,20,"��",0);
    }
    else
    {
        Show12X12String(71,20,SenserName[CheckRes.SenserType],0);

        if (CheckRes.SenserType == SENSER_Y)
        {
            InSenserEnable = TRUE;
            // NaI ̽ͷֻʹ���ڲ�������   -- 2019.3.6
            FaceInfo.Gm = 1;
        }
        else
        {
            // ����̽ͷ��ֹ�ڲ�̽ͷ
            InSenserEnable = FALSE;
        }
    }

    Show12X12String(0,35,"�ڲ�̽����",0);

    if (!InnerParam.InnerSersor)
    {
        InSenserEnable = FALSE;
    }
                
    if (InSenserEnable)
    {
        Show5X7String(65,38,":@Y",0);
    }
    else
    {
        Show5X7String(65,38,":",0);
        if (InnerParam.InnerSersor)
        {
            Show12X12String(71,35,"����",0);
        }
        else
        {
            Show12X12String(71,35,"��",0);
        }
    }
    
    
    GT_Show12X12Char(0,50,"��ص�ѹ",0);
    Show5X7String(52,53,":",0);
    if (CheckRes.BatSt)
    {
        GT_Show12X12Char(58,50,"����",0);
    }
    else
    {
        GT_Show12X12Char(58,50,"�쳣",1);
    }

    
    
    #if 1
    GT_Show12X12Char(0,65,"��������",0);
    Show5X7String(52,68,":",0);

    if (CheckRes.CommInterface == COMM_CAN)
    {
        GT_Show12X12Char(58,65,"����",0);
    }
    else
    {
        if (CheckRes.WifiSt)
        {
            GT_Show12X12Char(58,65,"����",0);
        }
        else
        {
            GT_Show12X12Char(58,65,"��",1);
        }
    }
    #else
        #if 0  // ��ѹ
        GT_Show12X12Char(0,65,"��ѹ���",0);
        Show5X7String(52,68,":",0);
        if (CheckRes.HvSt)
        {
            GT_Show12X12Char(58,65,"����",0);
        }
        else
        {
            GT_Show12X12Char(58,65,"�쳣",1);
        }
        #else
        GT_Show12X12Char(0,65,"���ݲ���",0);
        Show5X7String(52,68,":",0);
        if (CheckRes.E2St)
        {
            GT_Show12X12Char(58,65,"����",0);
        }
        else
        {
            GT_Show12X12Char(58,65,"�쳣",1);
        }
        #endif

    #endif
    
    
    
    GT_Show12X12Char(0,80,"ʵʱ��",0);
    Show5X7String(39,83,":",0);
    if (CheckRes.TimerSt)
    {
        GT_Show12X12Char(45,80,"����",0);
    }
    else
    {
        GT_Show12X12Char(45,80,"�쳣",1);
    }

    if (!SelfCheckOk())
    {
        GT_Show16X16Char(32,96,"�Լ�ʧ��",1);

        if (CheckRes.E2St == FALSE)
        {
            GT_Show12X12Char(0,116,"���Ĳ����쳣",0);
        }
        else
        {
            GT_Show12X12Char(10,116,"�����������ϵͳ",0);
        }
    }
    else
    {
        GT_Show16X16Char(32,100,"�Լ�ɹ�",0);
    }

    DisplayRefresh();

    //StopRunLed();
    //RunLedOn();
    //StartRunLed(1000,1000);
    StopAlarmLed();

    // ʼ�մ��ڲ�̽ͷ
    #if 0
    Pkur_Init();
    #else
    if (InSenserEnable)
    {
        GetCounter();   // �嶨ʱ��
        Init_InSenser();

        //if (CheckRes.SenserType == SENSER_Y)
        //{
        //    GM_HIGH;
        //}
    }
    #endif

    // ̽ͷ�仯��Ҫ�������
    if (NeedSaveParam())
    {
        WriteParamToFlash();
    }

    ADC12_Start();
}


BOOL NeedBackTest()
{
    #if 0
    if ((CheckRes.SenserType == SENSER_NONE) ||    // �ڲ�̽ͷ�ޱ���
        (CheckRes.SenserType == SENSER_Y)     ||   // NaI û�б���
        (CheckRes.SenserType == SENSER_LP))        // ���� 
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
    #else
    return Senser.Gnd.Valid;
    #endif
}

void InitCounter()
{
    GetCounter();
    //memset(&FaceInfo, 0, sizeof(METEFACEINFO));
    //memcpy(&FaceInfo, &Param.MetInfo, sizeof(METEFACEINFO));
    //ClearCounter();
    InitArr();
}

void SelfCheckKey(PRESSKEY Key)
{
    BK_OFF;
    
    switch(Key)
    {
        case TIMEOUT:  
        {
            if (SelfCheckOk()) 
            {
                if (NeedBackTest())
                {
                    StartBackTest();
                }
                else
                {
                    InitCounter();
                    EnterMeasuInter(); 
                }
            }
            // else  �ȴ���������
            
            break;
        }
        
        case OKKEY:    
        {
            if (CheckRes.E2St == TRUE)
            {
                if (NeedBackTest()
                     && (CheckRes.SenserSt == TRUE))
                {
                    StartBackTest();
                }
                else
                {
                    InitCounter();
                    
                    EnterMeasuInter(); 
                }
            }
            break;
        }
    }
}

void SelfCheck(void)
{
    static BYTE Cnt = 0;
    
    if (Cnt < 10) Cnt ++; 
    
    switch (Cnt)
    {
        case 1: CheckBat();    break;
        case 2: CheckLcd();     break;
        case 3: CheckTimer();   break;
        case 4: CheckWifi();    break;
        case 5: CheckCan();     break;
        case 6: CheckEeprom();  break;
        case 7: CheckSenser();  break;
        case 8: ShowCheckRes(); break;
    }
    
}

void Init_SelfCheck()
{
    memset((BYTE *)&CheckRes,0,sizeof(CheckRes));
    CheckRes.SenserType = SENSER_NONE;
    CheckRes.SenserSt = TRUE;
}
// SelfCheck <<<----------------------------------

