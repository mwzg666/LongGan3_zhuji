#include <stdio.h>
#include <string.h>

#include <msp430x552x.h>

#include "system.h"
#include "CommDataDefine.h"

#include "Keyboard.h"
#include "oled.h"
#include "QuickMenu.h"
#include "VerifPassword.h"
#include "DataStore.h"
#include "Measinter.h"
#include "Main.h"
#include "Input.h"
#include "SelfCheck.h"
#include "SsAlarm.h"
#include "Mcp2510.h"
#include "TMeas.h"


extern INTERFACE currUI;
extern QPara Param;
extern QInnerParam InnerParam;
extern METEFACEINFO FaceInfo;
extern RADIO pnuc[25];
extern BYTE  CurRadIndex;
extern CHECKRES CheckRes;
extern BYTE     RadMeasMode;   // ��ϳ�����ģʽ��0:Mix 1:�� 2����

char QuickIndex = 1;
BYTE QUICKMENUCNT = 3;

void InterQuickMenu()
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        MCP2510_WakeUp();
    }

    
    CanSleep = FALSE;
    CpuHighSpeed();
        
    QuickIndex = 1;
    currUI = QUICK;
   
    Clear(0, 10, 128, 118);
    RefreshQuickMenu();

    if (InnerParam.Keytone)
    {
        // �ð���������
        Sleep(100);
    }

    // �������״̬, ��ֹ���ص�ʱ���󱨾�
    DisableAllAlarm();
}


#if 0
void RefreshQuickMenu()
{  
    //ShowBatPower();
    //GT_Show16X16Char(32,8,"��ݹ���",0);
    
    PopWindow(20,26,88,80,"��ݹ���");
    
    GT_Show12X12Char(30,40,"������",0);
    Show5X7String(68,43,"[   ]",0);
    if(InnerParam.Sound)
    {
        GT_Show12X12Char(76,40,"��",QuickIndex == 1);
    }
    else 
    {
        GT_Show12X12Char(76,40,"��",QuickIndex == 1);
        
    }

    
    GT_Show12X12Char(30,56,"������",0);
    Show5X7String(68,59,"[   ]",0);
    if(InnerParam.Keytone)
    {
        GT_Show12X12Char(76,56,"��",QuickIndex == 2);
    }
    else 
    {
        GT_Show12X12Char(76,56,"��",QuickIndex == 2);
    }


    GT_Show12X12Char(30,72,"����",0);
    Show5X7String(68,75,"[   ]",0);
    if(InnerParam.pisScreen)
    {
        GT_Show12X12Char(76,72,"��",QuickIndex == 3);
    }
    else 
    {
        GT_Show12X12Char(76,72,"��",QuickIndex == 3);
    }


    
    GT_Show12X12Char(30,88,"ϵͳ���˵�",QuickIndex == 4);

    
    
    ShowOptHint();
    DisplayRefresh();
    
}

void QuickMenuSele(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT:  case POWER: EnterMeasuInter();  break;
        case RETURN:   EnterMeasuInter();  break;
        
        case DOWN:
        {
            if(++QuickIndex > 4)
            {
                QuickIndex = 1;
            }
            
            RefreshQuickMenu();
        }
        break;
        
        case UP:
        {
            if(--QuickIndex < 1)
            {
                QuickIndex = 4;
            }
            
            RefreshQuickMenu();
        }
        break;

        case MODE:   // ��ģʽ���л�����
        {
            switch (QuickIndex)
            {
                case 1: InnerParam.Sound = !InnerParam.Sound;        RefreshQuickMenu(); break;
                case 2: InnerParam.Keytone = !InnerParam.Keytone;    RefreshQuickMenu(); break;
                case 3: SetBkParam();                      RefreshQuickMenu(); break;
            }

            break;
        }
        
        case OKKEY:
        {            
            if(QuickIndex == 4) //��������������
            {
                InterPasswordUI();
            }
        }
        break;

        
    }
}
#else
void MeterView();
void RedoAlarm();
extern SYSALARM SysAlarm;
extern BYTE Setting_SenserType;
extern BYTE SSALARMCNT;
extern const SENSERCONTXT SenserContxt[SENSERCOUNT];
extern BYTE SsAlarmIndex;
extern BYTE SsAlarmPage;
extern char * const AlarmText[];
extern char AlarmValStr[16];
extern SENSERALARM AlarmTmp;
extern SENSER Senser;

void ShowAlarmThres()
{
    BYTE i,Loc;

    currUI = SHOWALM;
    
    if (CheckRes.SenserType == SENSER_NONE)
    {
        Setting_SenserType = SENSER_IN;
        memcpy(&AlarmTmp,&Param.InSenser.Alarm,sizeof(AlarmTmp));
    }
    else
    {
        Setting_SenserType = CheckRes.SenserType;
        memcpy(&AlarmTmp,&Senser.Alarm,sizeof(AlarmTmp));
    }
    
    i = GetSenserIndex(Setting_SenserType);
    SSALARMCNT = SenserContxt[i].AlarmCnt;
    
    
        
    Clear_DispBuf();
    
    GT_Show16X16Char(40,0,"����ֵ",0);
    

    if (SSALARMCNT > PAGE_MENUCOUNT)
    {
        if (SsAlarmIndex > PAGE_MENUCOUNT)
        {
            SsAlarmPage = 1;
        }
        else
        {
            SsAlarmPage = 0;
        }
        
        if (SsAlarmPage == 0)
        {
            for (i=0;i<PAGE_MENUCOUNT;i++)
            { 
                Loc = GetValidAlarmLoc(i);
                Show12X12String(0,20+(i*16),AlarmText[Loc],0);
                Show5X7String(65,23+(i*16),":",0);
                GetAlarmValStr(Loc);
                Show5X7String(70,23+(i*16),AlarmValStr,0);
            }

            
            Show5X7String(20,20+(i*16),"...",0);
        }
        else
        {
            for (i=PAGE_MENUCOUNT;i<SSALARMCNT;i++)
            { 
                Loc = GetValidAlarmLoc(i);
                Show12X12String(0,20+((i-PAGE_MENUCOUNT)*16),AlarmText[Loc],0);
                Show5X7String(65,23+((i-PAGE_MENUCOUNT)*16),":",0);
                GetAlarmValStr(Loc);
                Show5X7String(70,23+((i-PAGE_MENUCOUNT)*16),AlarmValStr,0);
            }

            
            Show12X12String(50,116,"OK������",0);
        }
    }
    else
    {
        SsAlarmPage = 0;
        for (i=0;i<SSALARMCNT;i++)
        { 
            Loc = GetValidAlarmLoc(i);
            Show12X12String(0,20+(i*16),AlarmText[Loc],0);
            Show5X7String(65,23+(i*16),":",0);
            GetAlarmValStr(Loc);
            Show5X7String(70,23+(i*16),AlarmValStr,0);
        }


        Show12X12String(50,116,"OK������",0);
    }

    
    
    ShowOptHint();
    DisplayRefresh();
}


void ShowAlmKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT:  case POWER: EnterMeasuInter();  break;
        case RETURN:   EnterMeasuInter();  break;
        
        case DOWN:
        {
            if (SSALARMCNT > PAGE_MENUCOUNT)
            {
                if (SsAlarmIndex == 1)
                {
                    SsAlarmIndex += PAGE_MENUCOUNT;
                }
            }
            ShowAlarmThres();
        }
        break;
        
        case UP:
        {
            SsAlarmIndex = 1;
            ShowAlarmThres();
        }
        break;

        
        
        case OKKEY:  EnterMeasuInter(); break;

        
    }
}


// ���ۼƼ���
void ClearTotol()
{
    FaceInfo.In_Total = 0;
    if (CheckRes.SenserType == SENSER_LP)
    {
        if (!BTClearTotol())
        {
            ShowHint("�ۼƼ������ʧ��",EnterMeasuInter);
            return;
        }
    }
    ShowHint("�ۼƼ�������ɹ�",EnterMeasuInter);
}


// ��������
void ClearMax()
{
    FaceInfo.In_Max = 0;
    FaceInfo.OutA_Cps_Max = 0;
    FaceInfo.OutB_Cps_Max = 0;
    //FaceInfo.OutC_Cps_Max = 0;
    FaceInfo.OutA_Cur_Max = 0;
    FaceInfo.OutB_Cur_Max = 0;
    if (CheckRes.SenserType == SENSER_LP)
    {
        if (!BTClearMax())
        {
            ShowHint("�ۼƼ������ʧ��",EnterMeasuInter);
            return;
        }
    }
    ShowHint("����������ɹ�",EnterMeasuInter);
}



void QMHndS2()
{
    if (CheckRes.SenserType  == SENSER_LC)
    {
        EnterTimerMeas();
    }
    else
    {
        ClearMax(); 
    }
}


void QMHndS3()
{
    //if ((CheckRes.SenserType == SENSER_A_A) || (CheckRes.SenserType == SENSER_B_B))
    if (HaveRadSet() || (CheckRes.SenserType == SENSER_LC))
    {
        //InterPasswordUI();
    }
    else
    {
        ClearTotol();
    }
}

void SaveDose()
{
    StorageRtData(); 
    ShowHint("��ǰֵ����ɹ�",EnterMeasuInter);
    //EnterMeasuInter();
}


void LcQuickMenu()
{
    PopWindow(20,36,88,64,"��ݹ���");
        
    Show12X12String(30,50,"���浱ǰֵ",QuickIndex == 1);
    Show12X12String(30,66,"��ʱ����",QuickIndex == 2);
    Show12X12String(30,82,"ϵͳ���˵�",QuickIndex == 3);
    QUICKMENUCNT = 3;
}

void NorQuickMenu()
{
    PopWindow(20,28,88,80,"��ݹ���");

    Show12X12String(30,42,"���浱ǰֵ",QuickIndex == 1);
    Show12X12String(30,58,"������ֵ",QuickIndex == 2);
    Show12X12String(30,74,"���ۼƼ���",QuickIndex == 3);
    Show12X12String(30,90,"ϵͳ���˵�",QuickIndex == 4);
    QUICKMENUCNT = 4;
}

void RadQuickMenu()
{
    PopWindow(20,28,88,80,"��ݹ���");
        
    Show12X12String(30,42,"���浱ǰֵ",QuickIndex == 1);
    Show12X12String(30,58,"������ֵ",QuickIndex == 2);

    if (RadMeasMode == RAD_MODE_MIX)
    {
        Show12X12String(30,74,"���ģʽ",        QuickIndex == 3);
    }
    else if (RadMeasMode == RAD_MODE_A)
    {
        Show12X12String(30,74,"@Aģʽ",        QuickIndex == 3);
    }
    else
    {
        Show12X12String(30,74,"@B@Yģʽ",        QuickIndex == 3);
    }

    Show12X12String(30,90,"ϵͳ���˵�",QuickIndex == 4);
    QUICKMENUCNT = 4;
}

void RefreshQuickMenu()
{   
    //if ((CheckRes.SenserType == SENSER_A_A) || (CheckRes.SenserType == SENSER_B_B))
    if (HaveRadSet())
    {
        RadQuickMenu();
    }
    else
    {
        if (CheckRes.SenserType  == SENSER_LC)
        {
            LcQuickMenu();
        }
        else
        {
            NorQuickMenu();
        }
    }
    

    ShowOptHint();
    DisplayRefresh();
}

void QuickMenuSele(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT:  case POWER: EnterMeasuInter();  break;
        case RETURN:   EnterMeasuInter();  break;
        
        case DOWN:
        {
            if(++QuickIndex > QUICKMENUCNT)
            {
                QuickIndex = 1;
            }
            
            RefreshQuickMenu();
        }
        break;
        
        case UP:
        {
            if(--QuickIndex < 1)
            {
                QuickIndex = QUICKMENUCNT;
            }
            
            RefreshQuickMenu();
        }
        break;

        case MODE:   // ��ģʽ��
        {
            if (QuickIndex == 3)
            {
                RadMeasMode++;
                if (RadMeasMode > RAD_MODE_B)
                {
                    RadMeasMode = RAD_MODE_MIX;
                }
                RefreshQuickMenu();
            }
        }
        break;
        
        case OKKEY:
        {   
            switch (QuickIndex)
            {
                //case 1: ShowAlarmThres();    break;
                case 1: SaveDose();         break;
                case 2: QMHndS2();          break;
                case 3: QMHndS3();          break;
                case 4: InterPasswordUI(); break;
            }
        }
        break;

        
    }
}



void MeterView()
{
    currUI = METERVIEW;
    Clear_DispBuf();
    ShowMeteTitle();

    Show12X12String(0,10,"�ڲ�̽����������",0);
    //�ڲ�Y�������ǵ���ʷ���ֵ
    Show12X12String(0,26,"���:",0);
    Show_InCounterSm(32,26,FaceInfo.In_Max);
    // �ۼƼ���
    Show12X12String(0,42,"�ۼ�:",0);
    Show_InTotal(32,42,FaceInfo.In_Total);
    
    ShowLine(58);

    Show12X12String(0,62,"�ⲿ̽����������",0);

    if (CheckRes.SenserType == SENSER_NONE)
    {
        Show12X12String(36,86,"δ����",0);
    }
    else
    {
        Show12X12String(0,78,"@A���:",0);
        ShowCounterSm(40,78,"",FaceInfo.OutA_Cur_Max,pnuc[CurRadIndex].Unit);
        
        Show12X12String(0,94,"@B���:",0);
        ShowCounterSm(40,94,"",FaceInfo.OutB_Cur_Max,pnuc[CurRadIndex].Unit);
    }

    Show12X12String(0,116,"�����������",0);

    ShowOptHint();
    DisplayRefresh();
}

void MeterViewKey(PRESSKEY key)
{
    EnterMeasuInter();
}

void RedoAlm()
{
    SysAlarm.MaskB1 = 0xFFFFFFFF;
    SysAlarm.MaskB2 = 0xFFFFFFFF;
    //EnableAllAlarm();
    EnterMeasuInter();
}

void RedoAlarm()
{
    if ((SysAlarm.MaskB1 != 0xFFFFFFFF) || (SysAlarm.MaskB2 != 0xFFFFFFFF))
    {
        ShowComfirm("�Ƿ�Ҫ�ָ����б���?",RedoAlm,EnterMeasuInter);
    }
    else
    {
        ShowHint("û����Ҫ�ָ��ı���",EnterMeasuInter);
    }
}


#endif

