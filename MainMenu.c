#include <stdio.h>
#include <string.h>
#include <msp430x552x.h>
#include "system.h"
#include "CommDataDefine.h"

#include "Keyboard.h"
#include "oled.h"

#include "OtherParam.h"
#include "Measinter.h"
#include "RadioSet.h"
#include "DataStore.h"
#include "Background.h"
#include "MainMenu.h"
#include "SsParam.h"
#include "SsAlarm.h"
#include "Senser.h"
#include "Main.h"
#include "SelfCheck.h"
#include "Input.h"
#include "SsSrv.h"
#include "Wifi.h"
#include "mcp4725.h"


extern INTERFACE currUI;
//extern unsigned int TIMERCOUNT;
extern QPara Param;
extern CHECKRES CheckRes;
extern SENSERPARAM ParamTmp;
extern SENSERALARM AlarmTmp;
extern SENSER Senser;
extern SENSERPARAM const *ParamMax;   //  �������ֵ
extern SENSERPARAM const *ParamMin;   //  ������Сֵ
extern BYTE SsParamIndex;
extern BYTE SsParamPage;
extern QInnerParam InnerParam;
extern METEFACEINFO FaceInfo;
extern char StrTemp[24];
extern BYTE SsAlarmIndex;
extern DWORD InSenserCnt;
extern char IsSuperUser;
extern BOOL InSenserEnable;
extern BYTE OutAlarmType;
extern BYTE SsAlarmPage;

static BYTE MainIndex = 1;
static BYTE  MAINMENUCNT = 5;

BYTE Setting_SenserType = SENSER_NONE;
      

void ShowMainMenu1()
{
    
    Clear_DispBuf();
    ShowMeteTitle();
    
    GT_Show16X16Char(32,16,"ϵͳ�˵�",0);    
    Show12X12String(20,36,"�ⲿ̽��������",MainIndex == 1);
    Show12X12String(20,52,"�ڲ�̽��������",MainIndex == 2);
    Show12X12String(20,68,"ϵͳ��������",MainIndex == 3);
    Show12X12String(20,84,"����/��־����",MainIndex == 4);
    #ifdef USE_WIFI
    MAINMENUCNT = 5;
    Show12X12String(20,100,"�����������",MainIndex == 5);
    #else
    MAINMENUCNT = 4;
    #endif
    ShowOptHint();    
    DisplayRefresh();
}


#if 0
void ShowMainMenu2()
{
    MAINMENUCNT = 3;
    Clear_DispBuf();
    ShowMeteTitle();
    
    GT_Show16X16Char(32,16,"ϵͳ�˵�",0);    
    GT_Show12X12Char(20,36,"ϵͳ��������",MainIndex == 1);
    GT_Show12X12Char(20,52,"̽ͷ��������",MainIndex == 2);
    GT_Show12X12Char(20,68,"̽ͷ������ֵ",MainIndex == 3);
    //GT_Show12X12Char(20,65,"̽ͷ����ģʽ",MainIndex == 4);
    ShowOptHint();
    DisplayRefresh();
}


void ShowMainMenu()
{
    // ��ͬ̽ͷ��ʾ��Ӧ�����˵�
    switch (CheckRes.SenserType)
    {
        case SENSER_IN:   ShowMainMenu2();     break;       
        case SENSER_PD:   ShowMainMenu2();     break;   //         0x02    //ˮ�¸�����̽ͷ��PD��
        case SENSER_GM:   ShowMainMenu2();     break;  //         0x03    //ˮ���и�����̽ͷ��GM��
        case SENSER_A_A:  ShowMainMenu1();    break;    //          0x04        ��̽ͷ(����alphy)
        case SENSER_A_B:  ShowMainMenu1();     break;  //        0x14    //��̽ͷ(����beta)
        case SENSER_A_AB: ShowMainMenu1();     break;  //      0x24    //��̽ͷ(�ɲ�alphy+beta)
        case SENSER_B_B:  ShowMainMenu1();    break;   //       0x05    //��̽ͷ(����beta)
        case SENSER_B_A:  ShowMainMenu1();    break;   //       0x15    //��̽ͷ(����alphy)
        case SENSER_B_AB: ShowMainMenu1();    break;   //      0x25    //��̽ͷ(�ɲ�alphy+beta)
        case SENSER_G1:   ShowMainMenu2();    break;   //        0x07    //����̽ͷ1
        case SENSER_G2:   ShowMainMenu2();    break;   //        0x08    //����̽ͷ2
    } 
}
#endif

void InterMainMenu()
{
    currUI = MAIN;

    #ifdef INIT_OPT_LOC
    MainIndex = 1;
    #endif
    ShowMainMenu1();
}

void MainMenuKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  EnterMeasuInter(); break;
        
        case MODE:
        {  
            
        }
        break;
        
        case UP:
        {
            if(--MainIndex < 1)
            {
                MainIndex = MAINMENUCNT;
            }
            ShowMainMenu1();
        }
        break;
        
        case DOWN:
        {
            if(++MainIndex > MAINMENUCNT)
            {
                MainIndex = 1;
            }
            ShowMainMenu1();
        }
        break;
        
        case OKKEY:
        {
            switch (MainIndex)
            {
                case 1:  OutSenserSetUI();      break;
                case 2:  InSenserSetUI();       break;
                case 3:  InterParamSetUI();    break;
                case 4:  InterDataManUI();      break;

                #ifdef USE_WIFI
                case 5:  EnterWifiMan();        break;
                #endif
                
                #if 0
                case 1:  InterParamSetUI();   break;
                
                case 2:  
                {
                    memcpy(&ParamTmp,&Senser.Param,sizeof(ParamTmp));
                    InterSsParamSetUI(); 
                    break;
                }
                
                case 3:  
                {
                    memcpy(&AlarmTmp,&Senser.Alarm,sizeof(AlarmTmp));
                    InterSsAlarmSetUI(); 
                    break;
                }
                
                case 4:  InterSsWorkSetUI();   break;
                case 5:  InterBackSetUI();  break;
                case 6:  InterRadSetMenu();  break;
                //case 7:  InterDataManUI();  break;
                #endif
            }

        }
        break;
    }

    
}




// out senser
char OutSenserIndex = 1;
BYTE OutMenuCount = 5;
void ShowOutSsUI()
{
    if ((!HaveRadSet()) || ( Senser.Contxt->AlarmCnt == 0))
    {
        PopWindow(18,20,90,86,"�ⲿ̽����");
        GT_Show12X12Char(26,32,"��������",OutSenserIndex==1);
        GT_Show12X12Char(26,46,"������ֵ",OutSenserIndex==2); 
        GT_Show12X12Char(26,60,"ǿ�Ʊ��ײ���",OutSenserIndex==3);
        GT_Show12X12Char(26,74,"���ع���",OutSenserIndex==4); 
        GT_Show12X12Char(26,88,"̽����ά��",OutSenserIndex==5); 
        OutMenuCount = 5;
    }
    else
    {
        PopWindow(18,20,90,72,"�ⲿ̽����");
        GT_Show12X12Char(26,32,"��������",OutSenserIndex==1);
        GT_Show12X12Char(26,46,"ǿ�Ʊ��ײ���",OutSenserIndex==2);
        GT_Show12X12Char(26,60,"���ع���",OutSenserIndex==3); 
        GT_Show12X12Char(26,74,"̽����ά��",OutSenserIndex==4); 
        OutMenuCount = 4;
    }
    //ShowOptHint();
    DisplayRefresh();
}

void OutSenserSetUI()
{
    if (CheckRes.SenserType == SENSER_NONE)
    {
        ShowHint("�����̽����", InterMainMenu);
        return;
    }

    if (GetAlarm(ALARM_SENSER_ERR))
    {
        ShowHint("���̽�����쳣", InterMainMenu);
        return;
    }

    #ifdef INIT_OPT_LOC
    OutSenserIndex = 1;
    #endif
    
    currUI = OUTSS;
    ShowOutSsUI();
}

void StartSetOutAlarm(BYTE Type)
{
    memcpy(&AlarmTmp,&Senser.Alarm,sizeof(AlarmTmp));
    Setting_SenserType = CheckRes.SenserType;
    SsAlarmIndex = 1;
    SsAlarmPage = 0;
    OutAlarmType = Type;
    InterSsAlarmSetUI(); 
}

void OutSenserSetKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterMainMenu(); break;
        
        case MODE:
        {  
            
        }
        break;
        
        case UP:
        {
            if(--OutSenserIndex < 1)
            {
                OutSenserIndex = OutMenuCount;
            }
            ShowOutSsUI();
        }
        break;
        
        case DOWN:
        {
            if(++OutSenserIndex > OutMenuCount)
            {
                OutSenserIndex = 1;
            }
            ShowOutSsUI();
        }
        break;
        
        case OKKEY:
        {
            switch (OutSenserIndex)
            {
                case 1:  
                {
                    memcpy(&ParamTmp,&Senser.Param,sizeof(ParamTmp));
                    ParamMax = Senser.ParamMax;
                    ParamMin = Senser.ParamMin;
                    Setting_SenserType = CheckRes.SenserType;
                    SsParamIndex = 1;
                    SsParamPage = 0;
                    InterSsParamSetUI(); 
                    break;
                }
                
                case 2:  
                {
                    if (OutMenuCount == 5)
                    {
                        StartSetOutAlarm(NOR);
                    }
                    else
                    {
                        InterBackSetUI();
                    }
                    break;
                }
                
                case 3:
                {
                    if (OutMenuCount == 5)
                    {
                        InterBackSetUI();
                    }
                    else
                    {
                        InterRadSetMenu();
                    }
                    break;
                }
                
                case 4:  
                {
                    if (OutMenuCount == 5)
                    {
                        InterRadSetMenu();
                    }
                    else
                    {
                        InterSsWorkSetUI();
                    }
                        
                    break;
                }
                case 5:  InterSsWorkSetUI();    break;
            }
        }
        break;
    }

    
}


void MeterManUI();

void ClearTotolCounter()
{
    FaceInfo.In_Total = 0;
    ShowHint("�ۼƼ�������ɹ�",MeterManUI);
}

void ClearMaxCounter()
{
    FaceInfo.In_Max = 0;
    ShowHint("�ۼƼ�������ɹ�",MeterManUI);
}


char MeterManIndex = 1;
void MeterManUI()
{
    currUI = METERMAN;
    Clear_DispBuf();
    ShowMeteTitle();

    //�ڲ�Y�������ǵ���ʷ���ֵ
    Show12X12String(0,16,"�ڲ�̽����������",0);
    Show_InCounterSm(0,32,FaceInfo.In_Max);
    Show12X12String(100,42,"���",MeterManIndex==1);

    ShowLine(62);

    // �ۼƼ���
    Show12X12String(0,70,"�ڲ�̽�����ۼƼ���",0);
    Show_InTotal(0,86,FaceInfo.In_Total);
    Show12X12String(100,96,"���",MeterManIndex==2);

    ShowOptHint();
    DisplayRefresh();
}

void MeterManKey(PRESSKEY key)
{
    switch (key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterMainMenu(); break;
        
        case MODE:
        {  
            
        }
        break;
        
        case UP:
        {
            if(--MeterManIndex < 1)
            {
                MeterManIndex = 2;
            }
            MeterManUI();
        }
        break;
        
        case DOWN:
        {
            if(++MeterManIndex > 2)
            {
                MeterManIndex = 1;
            }
            MeterManUI();
        }
        break;
        
        case OKKEY:
        {
            switch (MeterManIndex)
            {
                case 1:  
                {
                    ShowComfirm("�Ƿ�Ҫ�����ʷ������?",ClearMaxCounter,MeterManUI);
                    break;
                }
                
                case 2:  
                {
                    ShowComfirm("�Ƿ�Ҫ����ۼƼ���?",ClearTotolCounter,MeterManUI);
                    break;
                }

                
            }
        }
        break;
    }
}


char InSenserTestTime = 120;
DWORD TotalCnt = 0;
void InSenserTestMode()
{
    //static DWORD Tt = 0;

    currUI = INTEST;
    Clear_DispBuf();
    ShowMeteTitle();

    //�ڲ�Y�������ǵ���ʷ���ֵ
    Show12X12String(0,12,"�ڲ�̽����ά��",0);

    //InSenserCnt = (Tt++)*10000;
    FaceInfo.In_Cps = InSenserCnt; //Get_InCounter();   // CPS
    ShowCounter(0,28,"",FaceInfo.In_Cps,UNIT_CPS);

    TotalCnt += InSenserCnt;
    memset(StrTemp,0,24);
    sprintf(StrTemp,"�ۼƼ���:%lu",TotalCnt);
    Show12X12String(0,82,StrTemp,0);
    
    #if 0
    InSenserTestTime--;
    memset(StrTemp,0,24);
    sprintf(StrTemp,"ʣ��ʱ��:%ds",InSenserTestTime);
    Show12X12String(0,98,StrTemp,0);
    #endif
    
    Show12X12String(0,114,"�����������",0);
    //ShowStatusBar();
    DisplayRefresh();

    #if 0
    if (InSenserTestTime == 0)
    {
        EnterMeasuInter();
    }
    #endif
}


void InSenserTestKey(PRESSKEY key)
{
    switch (key)
    {
        case TIMEOUT: InSenserTestMode(); break;

        // ���������
        case POWER:   EnterMeasuInter(); break;  
        case OKKEY:    
        case UP:
        case DOWN:
        case MODE:
        case RETURN:
            InterMainMenu();
            break;
        
    }
}

//����У׼����
extern FLOATEX     FloatEx;
extern INPUTBYTE   InByte;
extern INPUTWORD   InWord;

void InCounterFactor();




void InputLa()
{
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(InnerParam.La);
    FloatEx.x = 10;
    FloatEx.y = 46;
    FloatEx.Rf = InCounterFactor;
    InputFloatEx();
}

void InputLb()
{
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(InnerParam.Lb);
    FloatEx.x = 10;
    FloatEx.y = 56;
    FloatEx.Rf = InCounterFactor;
    InputFloatEx();
}

void InputLc(BYTE x, BYTE y)
{
    FloatEx.Bp = 1;
    FloatEx.Bn = 2;
    FloatEx.Max = 9.99;
    FloatEx.Min = 0.01;
    FloatEx.Val = &(InnerParam.Lc);
    FloatEx.x = x;
    FloatEx.y = y;
    
    FloatEx.Rf = InCounterFactor;
    InputFloatEx();
}


void InputHa()
{
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(InnerParam.Ha);
    FloatEx.x = 10;
    FloatEx.y = 82;
    FloatEx.Rf = InCounterFactor;
    InputFloatEx();
}

void InputHb()
{
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(InnerParam.Hb);
    FloatEx.x = 10;
    FloatEx.y = 92;
    FloatEx.Rf = InCounterFactor;
    InputFloatEx();
}

void InputHc(BYTE x, BYTE y)
{
    FloatEx.Bp = 1;
    FloatEx.Bn = 2;
    FloatEx.Max = 9.99;
    FloatEx.Min = 0.01;
    FloatEx.Val = &(InnerParam.Hc);
    FloatEx.x = x;
    FloatEx.y = y;
    
    FloatEx.Rf = InCounterFactor;
    InputFloatEx();
}


void InputYa()
{
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(InnerParam.Ya);
    FloatEx.x = 10;
    FloatEx.y = 48;
    FloatEx.Rf = InCounterFactor;
    InputFloatEx();
}

void InputYb()
{
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(InnerParam.Yb);
    FloatEx.x = 10;
    FloatEx.y = 58;
    FloatEx.Rf = InCounterFactor;
    InputFloatEx();
}

void InputYc(BYTE x, BYTE y)
{
    FloatEx.Bp = 1;
    FloatEx.Bn = 2;
    FloatEx.Max = 9.99;
    FloatEx.Min = 0.01;
    FloatEx.Val = &(InnerParam.Yc);
    FloatEx.x = x;
    FloatEx.y = y;
    
    FloatEx.Rf = InCounterFactor;
    InputFloatEx();
}


void UpdateFudu()
{
    MCP4725_OutVol(MCP4725_HV_ADDR,InnerParam.Fudu);
    InCounterFactor();
}

void InputFudu()
{
    InWord.Title = "������ֵ";
    InWord.Spec  = "mV";
    InWord.Max = 3600;
    InWord.Min = 0;
    InWord.Step = 10;
    InWord.Rf = UpdateFudu;
    InWord.Val = &InnerParam.Fudu;
    InputWord();
}


BYTE FACTORCNT = 5;
char CntFacterIndex = 1;
void ShowFactor();
void ShowFactorNor();
void ShowFactorAdmin();

void InCounterFactor()
{
    currUI = INCNTFACT;

    ShowFactor();
}

void ShowFactor()
{
    if (IsSuperUser)
    {
        ShowFactorAdmin();
    }
    else
    {
        ShowFactorNor();
    }
}


void ShowFactorNor()
{
    Clear_DispBuf();
    ShowMeteTitle();

    GT_Show16X16Char(8,12,"�ڲ�̽��������",0);

    if (InnerParam.DevType == DEV_MPRP_S2L)
    {
        FACTORCNT = 2;
        if (CntFacterIndex > FACTORCNT)
        {
            CntFacterIndex = 1;
        }
        Show12X12String(0,34,"����У׼����",0);
        
        memset(StrTemp,0,24);
        sprintf(StrTemp,"C:%.2f",InnerParam.Yc);
        Show5X7String(0,52,StrTemp,CntFacterIndex==1);

        Show12X12String(0,70,"������ֵ:",0);
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%dmV",InnerParam.Fudu);
        Show5X7String(60,73,StrTemp,CntFacterIndex==2);
    }
    else
    {
        FACTORCNT = 3;
        Show12X12String(0,34,"������У׼����",0);
        memset(StrTemp,0,24);
        sprintf(StrTemp,"C:%.2f",InnerParam.Lc);
        Show5X7String(0,52,StrTemp,CntFacterIndex==1);

        Show12X12String(0,70,"������У׼����",0);
        memset(StrTemp,0,24);
        sprintf(StrTemp,"C:%.2f",InnerParam.Hc);
        Show5X7String(0,88,StrTemp,CntFacterIndex==2);

        Show12X12String(0,106,"������ֵ:",0);
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%dmV",InnerParam.Fudu);
        Show5X7String(60,109,StrTemp,CntFacterIndex==3);
    }

    ShowOptHint();
    DisplayRefresh();
}


void ShowFactorAdmin()
{
    Clear_DispBuf();
    ShowMeteTitle();

    GT_Show16X16Char(8,12,"�ڲ�̽��������",0);

    if (InnerParam.DevType == DEV_MPRP_S2L)
    {
        FACTORCNT = 4;
        if (CntFacterIndex > FACTORCNT)
        {
            CntFacterIndex = 1;
        }
        Show12X12String(0,32,"����У׼����",0);
        
        memset(StrTemp,0,24);
        sprintf(StrTemp,"A:%.6f",InnerParam.Ya);
        Show5X7String(0,48,StrTemp,CntFacterIndex==1);

        memset(StrTemp,0,24);
        sprintf(StrTemp,"B:%.6f",InnerParam.Yb);
        Show5X7String(0,58,StrTemp,CntFacterIndex==2);

        memset(StrTemp,0,24);
        sprintf(StrTemp,"C:%.2f",InnerParam.Yc);
        Show5X7String(0,68,StrTemp,CntFacterIndex==3);

        Show12X12String(0,78,"������ֵ:",0);
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%dmV",InnerParam.Fudu);
        Show5X7String(60,81,StrTemp,CntFacterIndex==4);
    }
    else
    {
        FACTORCNT = 7;
        Show12X12String(0,30,"������У׼����",0);
        
        memset(StrTemp,0,24);
        sprintf(StrTemp,"A:%.6f",InnerParam.La);
        Show5X7String(0,46,StrTemp,CntFacterIndex==1);

        memset(StrTemp,0,24);
        sprintf(StrTemp,"B:%.6f",InnerParam.Lb);
        Show5X7String(0,56,StrTemp,CntFacterIndex==2);

        memset(StrTemp,0,24);
        sprintf(StrTemp,"C:%.2f",InnerParam.Lc);
        Show5X7String(80,56,StrTemp,CntFacterIndex==3);

        Show12X12String(0,66,"������У׼����",0);
        memset(StrTemp,0,24);
        sprintf(StrTemp,"A:%.6f",InnerParam.Ha);
        Show5X7String(0,82,StrTemp,CntFacterIndex==4);

        memset(StrTemp,0,24);
        sprintf(StrTemp,"B:%.6f",InnerParam.Hb);
        Show5X7String(0,92,StrTemp,CntFacterIndex==5);

        memset(StrTemp,0,24);
        sprintf(StrTemp,"C:%.2f",InnerParam.Hc);
        Show5X7String(80,92,StrTemp,CntFacterIndex==6);

        Show12X12String(0,102,"������ֵ:",0);
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%dmV",InnerParam.Fudu);
        Show5X7String(60,105,StrTemp,CntFacterIndex==7);
    }

    ShowOptHint();
    DisplayRefresh();
}

void InCntFactorKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterMainMenu(); break;
        
        case MODE:
        {  
            
        }
        break;
        
        case UP:
        {
            if(--CntFacterIndex < 1)
            {
                CntFacterIndex = FACTORCNT;
            }
            ShowFactor();
        }
        break;
        
        case DOWN:
        {
            if(++CntFacterIndex > FACTORCNT)
            {
                CntFacterIndex = 1;
            }
            ShowFactor();
        }
        break;
        
        case OKKEY:
        {
            if (IsSuperUser)
            {
                if (InnerParam.DevType == DEV_MPRP_S2L)
                {
                    switch (CntFacterIndex)
                    {
                        case 1:  InputYa(); break;
                        case 2:  InputYb(); break;
                        case 3:  InputYc(10,68); break;   
                        case 4:  InputFudu(); break;
                    }
                }
                else
                {
                    switch (CntFacterIndex)
                    {
                        case 1:  InputLa(); break;
                        case 2:  InputLb(); break;
                        case 3:  InputLc(90,56); break;    
                        case 4:  InputHa(); break;
                        case 5:  InputHb(); break;
                        case 6:  InputHc(90,92); break;    
                        case 7:  InputFudu(); break;
                    }
                }
            }
            else
            {
                if (InnerParam.DevType == DEV_MPRP_S2L)
                {
                    switch (CntFacterIndex)
                    {
                        //case 1:  InputYa(); break;
                        //case 2:  InputYb(); break;
                        case 1:  InputYc(10,48); break;   
                        case 2:  InputFudu(); break;
                    }
                }
                else
                {
                    switch (CntFacterIndex)
                    {
                        //case 1:  InputLa(); break;
                        //case 2:  InputLb(); break;
                        case 1:  InputLc(10,48); break;    
                        //case 4:  InputHa(); break;
                        //case 5:  InputHb(); break;
                        case 2:  InputHc(10,86); break;    
                        case 3:  InputFudu(); break;
                    }
                }
            }
        }
        break;
    }
}




// in senser 
#define INSSMENUCNT  4
char InSenserIndex = 1;
void ShowInSsUI()
{
    Clear(0, 10, 128, 118);
    PopWindow(18,28,90,80,"�ڲ�̽����");
    
    GT_Show12X12Char(30,40,"��������",  InSenserIndex == 1);
    GT_Show12X12Char(30,56,"������ֵ",  InSenserIndex == 2); 
    GT_Show12X12Char(30,72,"�����ʹ���",InSenserIndex == 3);
    GT_Show12X12Char(30,88,"ά��ģʽ",  InSenserIndex == 4);
    //GT_Show12X12Char(30,104,"����У׼����",  InSenserIndex == 5);
    //ShowOptHint();
    DisplayRefresh();
}


void InSenserSetUI()
{
    if (!InSenserEnable)
    {
        ShowHint("�ڲ�̽��������", InterMainMenu);
        return;
    }
    
    if (GetAlarm(ALARM_INSENSER_ERR))
    {
        ShowHint("�ڲ�̽�����쳣", InterMainMenu);
        return;
    }

    #ifdef INIT_OPT_LOC
    InSenserIndex = 1;
    #endif
    currUI = INSS;
    ShowInSsUI();
}


void InSenserSetKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterMainMenu(); break;
        
        case MODE:
        {  
            
        }
        break;
        
        case UP:
        {
            if(--InSenserIndex < 1)
            {
                InSenserIndex = INSSMENUCNT;
            }
            ShowInSsUI();
        }
        break;
        
        case DOWN:
        {
            if(++InSenserIndex > INSSMENUCNT)
            {
                InSenserIndex = 1;
            }
            ShowInSsUI();
        }
        break;
        
        case OKKEY:
        {
            switch (InSenserIndex)
            {
                case 1:  
                {
                    #if 0
                    memcpy(&ParamTmp,&Param.InSenser.Param,sizeof(ParamTmp));
                    ParamMax = Param.InSenser.ParamMax;
                    ParamMin = Param.InSenser.ParamMin;
                    Setting_SenserType = SENSER_IN;
                    SsParamIndex = 1;
                    SsParamPage = 0;
                    InterSsParamSetUI(); 
                    #else
                    InCounterFactor();
                    #endif
                    break;
                }
                
                case 2:  
                {
                    if (CheckRes.SenserType == SENSER_Y)
                    {
                        ShowHint("��ǰģʽ�ù��ܽ���", InterMainMenu);
                        return;
                    }

                    memcpy(&AlarmTmp,&Param.InSenser.Alarm,sizeof(AlarmTmp));
                    Setting_SenserType = SENSER_IN;
                    SsAlarmIndex = 1;
                    OutAlarmType = NOR;
                    InterSsAlarmSetUI(); 
                    break;
                }

                case 3:
                {
                    if (CheckRes.SenserType == SENSER_Y)
                    {
                        ShowHint("��ǰģʽ�ù��ܽ���", InterMainMenu);
                        return;
                    }
                    
                    MeterManUI();
                    break;
                }

                case 4:
                {
                    InSenserTestTime = 120;
                    TotalCnt = 0;
                    InSenserTestMode();
                    break;
                }

                
            }
        }
        break;
    }
}


