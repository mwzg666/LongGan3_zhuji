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
#include "SsAlarm.h"
#include "Senser.h"
#include "Main.h"
#include "Input.h"
#include "SelfCheck.h"



extern CHECKRES CheckRes;
extern const SENSERCONTXT SenserContxt[SENSERCOUNT];
extern SENSER Senser;
extern INTERFACE currUI;
extern QPara Param;
extern INPUTDWORD InDword;
extern INPUTFLOAT InFloat;
extern BYTE Setting_SenserType;
extern INPUTEXT InEx;
extern FLOATEX  FloatEx;
extern char FloatUnitIndex;
extern char * const UnitStr[];
extern BYTE  RadCount;     // ��������
extern RADIO pnuc[25];      //����
extern BYTE  CurRadIndex;   // ����ʹ�ú��ص�����

BYTE SsAlarmIndex = 1;
BYTE SsAlarmPage = 0;

BYTE SSALARMCNT = 0;
BYTE OutAlarmType = NOR; 


char AlarmValStr[16] = {0};
SENSERALARM AlarmTmp;

char * const AlarmText[] =
{
    "@AԤ��",        //(@A)   ----1
    "@A����",        //(@A)
    "@A�ͱ���",      //(@A)
    "@A�߱���",      //(@A)
    "@BԤ��",        //(@B)   --- 5
    "@B����",        //(@B)  
    #ifdef FOR_TEMP
    "@B�ͱ���",      //(@B)
    "@B�߱���",      //(@B)
    #else
    "@B�ͱ���",      //(@B)
    "@B�߱���",      //(@B)
    #endif
    "@A���ر���",    // (@A)
    "@B���ر���",    //(@B)  --- 0x0A
    "һ������",      //              0x0B
    "��������",      //             0x0C
    "��������",      //            0x0D
    "�ۼ�Ԥ��",      //           0x0D
    "�ۼƱ���",      //           0x0E
    "������",        //           0x10   
};


// LP ̽ͷ|| ABY̽ͷ
char * const AlarmText1[] =
{
    "Ԥ����ֵ",        //(@A)   ----1
    "������ֵ",        //(@A)
    "�ͱ���",      //(@A)
    "�߱���",      //(@A)
    "",        //(@B)   --- 5
    "",        //(@B)  
    "",      //(@B)
    "",      //(@B)
    "",    // (@A)
    "",    //(@B)  --- 0x0A
    "������Ԥ��",      //              0x0B
    "�����ʱ���",      //             0x0C
    "",      //            0x0D
    "",      //           0x0D
    "",      //           0x0E
    "",        //           0x10   
};


char * InnerUnit[] = 
{
    #ifdef UNIT_GY
    "uGy/h",
    "mGy/h",
    "Gy/h"
    #else
    "uSv/h",
    "mSv/h",
    "Sv/h"
    #endif
};

char * MxUnit[] = 
{
    #ifdef UNIT_GY
    "uGy",
    "mGy",
    "Gy"
    #else
    "uSv",
    "mSv",
    "Sv"
    #endif
};


char * OutUnit[] = 
{
    "CPS",
    "Bq",
    "Bq/cm@2"
};



void ShowSsAlarmSetUI()
{
    //const unsigned char Sign[] = {0x00,0x04,0x1C,0x7C,0x1C,0x04,0x00,0x00};
    
    BYTE i,Loc;
        
    Clear_DispBuf();
    
    if (Setting_SenserType == SENSER_IN)
    {
        GT_Show16X16Char(8,0,"�ڲ�̽��������",0);
    }
    else
    {
        if (OutAlarmType == MIX)
        {
            GT_Show16X16Char(8,0,"��ϳ�������ֵ",0);
        }
        else
        {
            GT_Show16X16Char(8,0,"�ⲿ̽��������",0);
        }
    }

    #if 1
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
                Show12X12String(0,20+(i*16),AlarmText[Loc],SsAlarmIndex==i+1);
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
                Show12X12String(0,20+((i-PAGE_MENUCOUNT)*16),AlarmText[Loc],SsAlarmIndex==i+1);
                Show5X7String(65,23+((i-PAGE_MENUCOUNT)*16),":",0);
                GetAlarmValStr(Loc);
                Show5X7String(70,23+((i-PAGE_MENUCOUNT)*16),AlarmValStr,0);
            }

            
            GT_Show12X12Char(100,20+((i-PAGE_MENUCOUNT)*16),"����",SsAlarmIndex == (SSALARMCNT+1));
        }
    }
    else
    {
        SsAlarmPage = 0;
        for (i=0;i<SSALARMCNT;i++)
        { 
            Loc = GetValidAlarmLoc(i);

            if ( (CheckRes.SenserType == SENSER_LP)   ||
                  (CheckRes.SenserType == SENSER_ABY) ||
                  (CheckRes.SenserType == SENSER_PD)  ||
                  (CheckRes.SenserType == SENSER_GM)
                )
            {
                Show12X12String(0,20+(i*16),AlarmText1[Loc],SsAlarmIndex==i+1);
            }
            else
            {
                Show12X12String(0,20+(i*16),AlarmText[Loc],SsAlarmIndex==i+1);
            }
            Show5X7String(65,23+(i*16),":",0);
            GetAlarmValStr(Loc);
            Show5X7String(70,23+(i*16),AlarmValStr,0);
        }

        // ���水ť
        GT_Show12X12Char(100,20+(i*16),"����",SsAlarmIndex == (SSALARMCNT+1));
    }

    #else
    
    for (i=0;i<SSALARMCNT;i++)
    {
        Loc = GetValidAlarmLoc(i);

        #if 1
        Show12X12String(0,20+(i*16),AlarmText[Loc],SsAlarmIndex==i+1);
        #else
        // ���á����桢ˮ��̽ͷ����ʾ����a/b
        if ( (CheckRes.SenserType == SENSER_IN) ||
              (CheckRes.SenserType == SENSER_PD) ||
              (CheckRes.SenserType == SENSER_GM) || 
              (Setting_SenserType == SENSER_IN)
           )
        {
            if (Loc < 2)
            {
            
                // ֻ��ǰ������������������a/b
                Show12X12String(0,20+(i*16),AlarmText[Loc+11],SsAlarmIndex==i+1);
            }
            else
            {
                Show12X12String(0,20+(i*16),AlarmText[Loc],SsAlarmIndex==i+1);
            }
            
        }
        else if ( (CheckRes.SenserType == SENSER_G1) || 
                   (CheckRes.SenserType == SENSER_G2) )
        {
            Show12X12String(0,20+(i*16),AlarmText[Loc+6],SsAlarmIndex==i+1);
        }
        else
        {
            Show12X12String(0,20+(i*16),AlarmText[Loc],SsAlarmIndex==i+1);
        }
        #endif

        // ����ֵ
        Show5X7String(52,23+(i*16),":",0);
        GetAlarmValStr(Loc);
        Show5X7String(58,23+(i*16),AlarmValStr,0);
    }

    

    // ��ɰ�ť
    GT_Show12X12Char(100,20+(i*16),"����",SsAlarmIndex == (SSALARMCNT+1));

    #endif

    
    ShowOptHint();
    DisplayRefresh();
}

void InterSsAlarmSetUI()
{
    currUI = SSALARM;
    
    //#ifdef INIT_OPT_LOC
    //SsAlarmIndex = 1;
    //#endif
    
    BYTE i = GetSenserIndex(Setting_SenserType);
    SSALARMCNT = SenserContxt[i].AlarmCnt;
    ShowSsAlarmSetUI();
}

// ��ȡ��Index ��������λ��-- ��0 ��ʼ
BYTE GetValidAlarmLoc(BYTE Index)
{
    BYTE j,k;
    BYTE i = GetSenserIndex(Setting_SenserType);
    k = 0;
    for (j=0;j<ALARMCOUNT;j++)  
    {
        if (SenserContxt[i].AlarmLoc[j] == TRUE)
        {
            if (Index == k) break;
            k++;
        }
    }
    return j;
}

BYTE GetValidAlarmLocEx(BYTE SsType, BYTE Index)
{
    BYTE j,k;
    BYTE i = GetSenserIndex(SsType);
    k = 0;
    for (j=0;j<ALARMCOUNT;j++)  
    {
        if (SenserContxt[i].AlarmLoc[j] == TRUE)
        {
            if (Index == k) break;
            k++;
        }
    }
    return j;
}

float GetAlarmValEx(BYTE Loc)
{
    float ret;
    switch(Loc)
    {
        case 0: ret = Senser.Alarm.A1;   break;
        case 1: ret = Senser.Alarm.A2;   break;
        case 2: ret = Senser.Alarm.Al;   break;
        case 3: ret = Senser.Alarm.Ah;   break;
        case 4: ret = Senser.Alarm.B1;   break;
        case 5: ret = Senser.Alarm.B2;   break;
        case 6: ret = Senser.Alarm.Bl;   break;
        case 7: ret = Senser.Alarm.Bh;   break;
        case 8: ret = Senser.Alarm.Ac;   break;
        case 9: ret = Senser.Alarm.Bc;   break;
        
        case 10: ret = Senser.Alarm.Y1;   break;
        case 11: ret = Senser.Alarm.Y2;   break;
        case 12: ret = Senser.Alarm.Y3;   break;
        case 13: ret = Senser.Alarm.Ma;   break;
        case 14: ret = Senser.Alarm.Mx;   break;
        case 15: ret = (float)Senser.Alarm.Zu;   break;
    }
    return ret;
}



float GetAlarmVal(BYTE Loc)
{
    float ret;
    switch(Loc)
    {
        case 0: ret = AlarmTmp.A1;   break;
        case 1: ret = AlarmTmp.A2;   break;
        case 2: ret = AlarmTmp.Al;   break;
        case 3: ret = AlarmTmp.Ah;   break;
        case 4: ret = AlarmTmp.B1;   break;
        case 5: ret = AlarmTmp.B2;   break;
        case 6: ret = AlarmTmp.Bl;   break;
        case 7: ret = AlarmTmp.Bh;   break;
        case 8: ret = AlarmTmp.Ac;   break;
        case 9: ret = AlarmTmp.Bc;   break;
        case 10: ret = AlarmTmp.Y1;   break;
        case 11: ret = AlarmTmp.Y2;   break;
        case 12: ret = AlarmTmp.Y3;   break;
        case 13: ret = AlarmTmp.Ma;   break;
        case 14: ret = AlarmTmp.Mx;   break;
        case 15: ret = (float)AlarmTmp.Zu;   break;
    }
    return ret;
}

void GetInnerAlarmStr(float Val)
{
    if (Val >= 1000000)
    {
        sprintf(AlarmValStr,"%0.2fSv/h",Val/1000000);
    }
    else if (Val >= 1000)
    {
        sprintf(AlarmValStr,"%0.2fmSv/h",Val/1000);
    }
    else
    {
        sprintf(AlarmValStr,"%0.2fuSv/h",Val);
    }
}

void GetOutAlarmStr(float Val, BYTE Unit)
{
    
    #if 0
    if ( (CheckRes.SenserType == SENSER_G1)  ||
         (CheckRes.SenserType == SENSER_G2) 
        )
    {
        if(Senser.Param.Hn == 0)  // ��ϳ�
        {
            sprintf(AlarmValStr,"%0.0f%s",Val,"CPS");
        }
        else
        {
            sprintf(AlarmValStr,"%0.2f%s",Val,
                                   UnitStr[pnuc[CurRadIndex].Unit]);
        }
        
    }
    else
    {
        sprintf(AlarmValStr,"%0.0f%s",Val,
                                   UnitStr[pnuc[CurRadIndex].Unit]);
    }
    #else
    // ���ݵ�λ����
    // ������ת����
    //static float r;
    //r = CpsUnit(Val, Unit);
    if(Unit == UNIT_Bq_cm2)
    {
        sprintf(AlarmValStr,"%0.2f%s",Val,UnitStr[Unit]);
    }
    else
    {
        sprintf(AlarmValStr,"%0.0f%s",Val,UnitStr[Unit]);
    }
    #endif
}

void GetInnerMxStr(float Val)
{
    if (Val >= 1000000)
    {
        sprintf(AlarmValStr,"%0.2fSv",Val/1000000);
    }
    else if (Val >= 1000)
    {
        sprintf(AlarmValStr,"%0.2fmSv",Val/1000);
    }
    else
    {
        sprintf(AlarmValStr,"%0.2fuSv",Val);
    }
}


BOOL IsValidAlarm(BYTE id)
{
    #if 1
    return TRUE;
    #else
    if (Senser.Param.Hn == 0) // ��ϳ�
    {
        if ( (id == ALMID_A1) ||  (id == ALMID_A2) || (id == ALMID_AL) || (id == ALMID_AH) ||
              (id == ALMID_B1) ||  (id == ALMID_B2) || (id == ALMID_BL) || (id == ALMID_BH) )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        if (pnuc[CurRadIndex].Channel == CHANNEL_A) // ��
        {
            if ( (id == ALMID_AC)  || (id == ALMID_AL) || (id == ALMID_AH) )
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else  // ��
        {
            if ( (id == ALMID_BC)  ||  (id == ALMID_BL) || (id == ALMID_BH) )
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        
    }
    #endif
}


void GetA1Str()
{
    if ( (Setting_SenserType == SENSER_IN) ||
          (Setting_SenserType == SENSER_PD) ||
          (Setting_SenserType == SENSER_GM)
        )
    {
        GetInnerAlarmStr(AlarmTmp.A1);
    }
    else
    {
        if (IsValidAlarm(ALMID_A1))
        {
            GetOutAlarmStr(AlarmTmp.A1,UNIT_CPS);
        }
        else
        {
            sprintf(AlarmValStr,"%s","---");
        }
    }
}

void GetA2Str()
{
    if ( (Setting_SenserType == SENSER_IN) ||
          (Setting_SenserType == SENSER_PD) ||
          (Setting_SenserType == SENSER_GM)
        )
    {
        GetInnerAlarmStr(AlarmTmp.A2);   
    }
    else
    {
        if (IsValidAlarm(ALMID_A2))
        {
            GetOutAlarmStr(AlarmTmp.A2,UNIT_CPS);
        }
        else
        {
            sprintf(AlarmValStr,"%s","---");
        }
    }
}

void GetAlStr()
{
    if (IsValidAlarm(ALMID_AL))
    {
        GetOutAlarmStr(AlarmTmp.Al,UNIT_CPS);
    }
    else
    {
        sprintf(AlarmValStr,"%s","---");
    }
}

void GetAhStr()
{
    if (IsValidAlarm(ALMID_AH))
    {
        GetOutAlarmStr(AlarmTmp.Ah,UNIT_CPS);
    }
    else
    {
        sprintf(AlarmValStr,"%s","---");
    }
}

void GetB1Str()
{
    if (IsValidAlarm(ALMID_B1))
    {
        GetOutAlarmStr(AlarmTmp.B1,UNIT_CPS);
    }
    else
    {
        sprintf(AlarmValStr,"%s","---");
    }
}

void GetB2Str()
{
    if (IsValidAlarm(ALMID_B2))
    {
        GetOutAlarmStr(AlarmTmp.B2,UNIT_CPS);
    }
    else
    {
        sprintf(AlarmValStr,"%s","---");
    }
}

void GetBlStr()
{
    if (IsValidAlarm(ALMID_BL))
    {
        GetOutAlarmStr(AlarmTmp.Bl,UNIT_CPS);
    }
    else
    {
        sprintf(AlarmValStr,"%s","---");
    }
}

void GetBhStr()
{
    if (IsValidAlarm(ALMID_BH))
    {
        GetOutAlarmStr(AlarmTmp.Bh,UNIT_CPS);
    }
    else
    {
        sprintf(AlarmValStr,"%s","---");
    }
}

void GetAcStr()
{
    //static BYTE id;
    if (IsValidAlarm(ALMID_AC))
    {
        //id = GetCurRadIndex();
        GetOutAlarmStr(AlarmTmp.Ac,pnuc[CurRadIndex].Unit);
    }
    else
    {
        sprintf(AlarmValStr,"%s","---");
    }
}

void GetBcStr()
{
    //static BYTE id;
    if (IsValidAlarm(ALMID_BC))
    {
        //id = GetCurRadIndex();
        GetOutAlarmStr(AlarmTmp.Bc,pnuc[CurRadIndex].Unit);
    }
    else
    {
        sprintf(AlarmValStr,"%s","---");
    }
}

void GetZuStr()
{
    sprintf(AlarmValStr,"%d%%",AlarmTmp.Zu);
}


void GetAlarmValStr(BYTE Index)
{
    memset(AlarmValStr,0,16);
    switch(Index)
    {
        case 0:  GetA1Str();     break;       
        case 1:  GetA2Str();     break;
        case 2:  GetAlStr();     break;
        case 3:  GetAhStr();     break;
        case 4:  GetB1Str();     break;
        case 5:  GetB2Str();     break;
        case 6:  GetBlStr();     break;
        case 7:  GetBhStr();     break;
        case 8:  GetAcStr();     break;
        case 9:  GetBcStr();     break;
        case 10: GetInnerAlarmStr(AlarmTmp.Y1); break;
        case 11: GetInnerAlarmStr(AlarmTmp.Y2); break;
        case 12: GetInnerAlarmStr(AlarmTmp.Y3); break;
        case 13: GetInnerMxStr(AlarmTmp.Ma);    break;
        case 14: GetInnerMxStr(AlarmTmp.Mx);    break;
        case 15: GetZuStr();                     break;
    }
}




void SetA1()
{
    switch (InEx.CurrUnit)
    {
        case 0: 
        {
            *InEx.RetVal = InEx.InputVal;
            break;
        }

        case 1: 
        {
            *InEx.RetVal = InEx.InputVal * 1000;
            break;
        }

        case 2: 
        {
            *InEx.RetVal = InEx.InputVal * 1000000;
            break;
        }

    }

    InterSsAlarmSetUI();
}


// �������뵥λ
void InputA1_Ext()
{
    InEx.Rf_Ok     = SetA1;
    InEx.Rf_Cancel = InterSsAlarmSetUI;
    InEx.UnitCount = 3;
    InEx.UnitList = InnerUnit;
    InEx.RetVal = &(AlarmTmp.A1);
    
    if (AlarmTmp.A1 >= 1000000)
    {
        InEx.CurrUnit = 2;
        InEx.InputVal = AlarmTmp.A1/1000000;
    }
    else if (AlarmTmp.A1 >= 1000)
    {
        InEx.CurrUnit = 1;
        InEx.InputVal = AlarmTmp.A1/1000;
    }
    else
    {
        InEx.CurrUnit = 0;
        InEx.InputVal = AlarmTmp.A1;
    }

    if (  (Setting_SenserType == SENSER_IN) ||
          (Setting_SenserType == SENSER_PD) ||
          (Setting_SenserType == SENSER_GM)
        )
    {
        InEx.Title = "����Ԥ����ֵ";
    }
    else
    {
        InEx.Title = "������ֵ(@A)";
    }
    
    FloatUnitIndex = 1;
    InputFloatUnit();
}


void InputA1_Out(BYTE Line)
{    
    #if 0
    InFloat.Title = "������ֵ(@A)";
    InFloat.Spec  = "CPS";
    
    InFloat.Bits = 0;
    InFloat.Val = &(AlarmTmp.A1);
    InFloat.Max = 100000;
    InFloat.Min = 0;
    InFloat.Step = 10;
    InFloat.Rf = InterSsAlarmSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 6;
    FloatEx.Bn = 0;
    FloatEx.Max = 999999;
    FloatEx.Min = 0;
    FloatEx.Val = &(AlarmTmp.A1);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsAlarmSetUI;
    InputFloatEx();
    #endif
}


void InputA1(BYTE Line)
{
    if ( (Setting_SenserType == SENSER_IN) ||
          (Setting_SenserType == SENSER_PD) ||
          (Setting_SenserType == SENSER_GM)
        )
    {
        InputA1_Ext();
    }
    else
    {
        if(IsValidAlarm(ALMID_A1))
        {
            InputA1_Out(Line);
        }
        else
        {
            ShowHint("��Ч����",InterSsAlarmSetUI);
        }
    }
}



void SetA2()
{
    switch (InEx.CurrUnit)
    {
        case 0: 
        {
            *InEx.RetVal = InEx.InputVal;
            break;
        }

        case 1: 
        {
            *InEx.RetVal = InEx.InputVal * 1000;
            break;
        }

        case 2: 
        {
            *InEx.RetVal = InEx.InputVal * 1000000;
            break;
        }

    }

    InterSsAlarmSetUI();
}


// �������뵥λ
void InputA2_Ext()
{
    InEx.Rf_Ok     = SetA2;
    InEx.Rf_Cancel = InterSsAlarmSetUI;
    InEx.UnitCount = 3;
    InEx.UnitList = InnerUnit;
    InEx.RetVal = &(AlarmTmp.A2);
    
    if (AlarmTmp.A2 >= 1000000)
    {
        InEx.CurrUnit = 2;
        InEx.InputVal = AlarmTmp.A2/1000000;
    }
    else if (AlarmTmp.A2 >= 1000)
    {
        InEx.CurrUnit = 1;
        InEx.InputVal = AlarmTmp.A2/1000;
    }
    else
    {
        InEx.CurrUnit = 0;
        InEx.InputVal = AlarmTmp.A2;
    }

    if ( (Setting_SenserType == SENSER_IN) ||
          (Setting_SenserType == SENSER_PD) ||
          (Setting_SenserType == SENSER_GM)
        )
    {
        InEx.Title = "����������ֵ";
    }
    else
    {
        InEx.Title = "2������(@A)��ֵ";
    }
    
    FloatUnitIndex = 1;
    InputFloatUnit();
}

void InputA2_CPS(BYTE Line)
{
    #if 0
    InFloat.Title = "2��������ֵ(@A)";
    InFloat.Spec  = "CPS";
    
    
    InFloat.Bits = 0;
    InFloat.Val = &(AlarmTmp.A2);
    InFloat.Max = 100000;
    InFloat.Min = 0;
    InFloat.Step = 10;
    InFloat.Rf = InterSsAlarmSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 6;
    FloatEx.Bn = 0;
    FloatEx.Max = 999999;
    FloatEx.Min = 0;
    FloatEx.Val = &(AlarmTmp.A2);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsAlarmSetUI;
    InputFloatEx();
    #endif
}


void InputA2(BYTE Line)
{
    if ( (Setting_SenserType == SENSER_IN) ||
          (Setting_SenserType == SENSER_PD) ||
          (Setting_SenserType == SENSER_GM)
        )
    {
        InputA2_Ext();
    }
    else
    {
        if(IsValidAlarm(ALMID_A2))
        {
            InputA2_CPS(Line);
        }
        else
        {
            ShowHint("��Ч����",InterSsAlarmSetUI);
        }
    }
}

void InputAL(BYTE Line)
{
    if(IsValidAlarm(ALMID_AL))
    {
        #if 0
        InFloat.Title = "�ͱ�����ֵ(@A)";
        InFloat.Spec  = "CPS";
        InFloat.Bits = 0;
        InFloat.Val = &(AlarmTmp.Al);
        InFloat.Max = 100000;
        InFloat.Min = 0;
        InFloat.Step = 10;
        InFloat.Rf = InterSsAlarmSetUI;
        InputFloat();
        #else
        FloatEx.Bp = 6;
        FloatEx.Bn = 0;
        FloatEx.Max = 999999;
        FloatEx.Min = 0;
        FloatEx.Val = &(AlarmTmp.Al);
        FloatEx.x = 70;
        FloatEx.y = 20 + (Line % 6) * 16;
        FloatEx.Rf = InterSsAlarmSetUI;
        InputFloatEx();
        #endif
    }
    else
    {
        ShowHint("��Ч����",InterSsAlarmSetUI);
    }
        
    
}

void InputAh(BYTE Line)
{
    if(IsValidAlarm(ALMID_AH))
    {
        #if 0
        InFloat.Title = "�߱�����ֵ(@A)";
        InFloat.Spec  = "CPS";
        InFloat.Bits = 0;
        InFloat.Val = &(AlarmTmp.Ah);
        InFloat.Max = 100;
        InFloat.Min = 0;
        InFloat.Step = 1;
        InFloat.Rf = InterSsAlarmSetUI;
        InputFloat();
        #else
        FloatEx.Bp = 6;
        FloatEx.Bn = 0;
        FloatEx.Max = 999999;
        FloatEx.Min = 0;
        FloatEx.Val = &(AlarmTmp.Ah);
        FloatEx.x = 70;
        FloatEx.y = 20 + (Line % 6) * 16;
        FloatEx.Rf = InterSsAlarmSetUI;
        InputFloatEx();
        #endif
    }
    else
    {
        ShowHint("��Ч����",InterSsAlarmSetUI);
    }
}

void InputB1(BYTE Line)
{
    if(IsValidAlarm(ALMID_B1))
    {
        #if 0
        InFloat.Title = "1��������ֵ(@B)";
        InFloat.Spec  = "CPS";
        InFloat.Bits = 0;
        InFloat.Val = &(AlarmTmp.B1);
        InFloat.Max = 100000;
        InFloat.Min = 0;
        InFloat.Step = 10;
        InFloat.Rf = InterSsAlarmSetUI;
        InputFloat();
        #else
        FloatEx.Bp = 6;
        FloatEx.Bn = 0;
        FloatEx.Max = 999999;
        FloatEx.Min = 0;
        FloatEx.Val = &(AlarmTmp.B1);
        FloatEx.x = 70;
        FloatEx.y = 20 + (Line % 6) * 16;
        FloatEx.Rf = InterSsAlarmSetUI;
        InputFloatEx();
        #endif
    }
    else
    {
        ShowHint("��Ч����",InterSsAlarmSetUI);
    }
}


void InputB2(BYTE Line)
{
    if(IsValidAlarm(ALMID_B2))
    {
        #if 0
        InFloat.Title = "2��������ֵ(@B)";
        InFloat.Spec  = "CPS";
        InFloat.Bits = 0;
        InFloat.Val = &(AlarmTmp.B2);
        InFloat.Max = 100000;
        InFloat.Min = 0;
        InFloat.Step = 10;
        InFloat.Rf = InterSsAlarmSetUI;
        InputFloat();
        #else
        FloatEx.Bp = 6;
        FloatEx.Bn = 0;
        FloatEx.Max = 999999;
        FloatEx.Min = 0;
        FloatEx.Val = &(AlarmTmp.B2);
        FloatEx.x = 70;
        FloatEx.y = 20 + (Line % 6) * 16;
        FloatEx.Rf = InterSsAlarmSetUI;
        InputFloatEx();
        #endif
    }
    else
    {
        ShowHint("��Ч����",InterSsAlarmSetUI);
    }
}

void InputBL(BYTE Line)
{
    if(IsValidAlarm(ALMID_BL))
    {
        #if 0
        #ifdef FOR_TEMP
        InFloat.Title = "�ͱ�����ֵ(@B)";
        #else
        InFloat.Title = "�ͱ�����ֵ";
        #endif
        InFloat.Spec  = "CPS";
        InFloat.Bits = 0;
        InFloat.Val = &(AlarmTmp.Bl);
        InFloat.Max = 100000;
        InFloat.Min = 0;
        InFloat.Step = 5;
        InFloat.Rf = InterSsAlarmSetUI;
        InputFloat();
        #else
        FloatEx.Bp = 6;
        FloatEx.Bn = 0;
        FloatEx.Max = 999999;
        FloatEx.Min = 0;
        FloatEx.Val = &(AlarmTmp.Bl);
        FloatEx.x = 70;
        FloatEx.y = 20 + (Line % 6) * 16;
        FloatEx.Rf = InterSsAlarmSetUI;
        InputFloatEx();
        #endif
    }
    else
    {
        ShowHint("��Ч����",InterSsAlarmSetUI);
    }
}

void InputBh(BYTE Line)
{
    if(IsValidAlarm(ALMID_BH))
    {
        #if 0
        #ifdef FOR_TEMP
        InFloat.Title = "�߱�����ֵ(@B)";
        #else
        InFloat.Title = "�߱�����ֵ";
        #endif
        
        InFloat.Spec  = "CPS";
        InFloat.Bits = 0;
        InFloat.Val = &(AlarmTmp.Bh);
        InFloat.Max = 100000;
        InFloat.Min = 0;
        InFloat.Step = 5;
        InFloat.Rf = InterSsAlarmSetUI;
        InputFloat();
        #else
        FloatEx.Bp = 6;
        FloatEx.Bn = 0;
        FloatEx.Max = 999999;
        FloatEx.Min = 0;
        FloatEx.Val = &(AlarmTmp.Bh);
        FloatEx.x = 70;
        FloatEx.y = 20 + (Line % 6) * 16;
        FloatEx.Rf = InterSsAlarmSetUI;
        InputFloatEx();
        #endif
    }
    else
    {
        ShowHint("��Ч����",InterSsAlarmSetUI);
    }
}

void InputAcEx()
{
    if (pnuc[CurRadIndex].Unit == UNIT_CPS)
    {
        FloatEx.Bp = 3;
        FloatEx.Bn = 0;
        FloatEx.Max = 300;
    }
    else if (pnuc[CurRadIndex].Unit == UNIT_Bq)
    {
        FloatEx.Bp = 3;
        FloatEx.Bn = 0;
        FloatEx.Max = 300;
    }
    else  // Bq_cm2
    {
        FloatEx.Bp = 3;
        FloatEx.Bn = 2;
        FloatEx.Max = 3;
    }
    FloatEx.Min = 0;
    FloatEx.Val = &(AlarmTmp.Ac);
    FloatEx.x = 60;
    FloatEx.y = 64;
    FloatEx.Rf = InterSsAlarmSetUI;
    InputFloatEx();
}


void InputAcBase()
{
    InFloat.Title = "���ر���(@A)";
    InFloat.Spec  = UnitStr[pnuc[CurRadIndex].Unit];
    InFloat.Bits = 0;
    InFloat.Val = &(AlarmTmp.Ac);
    InFloat.Max = 1000;
    InFloat.Min = 0;
    InFloat.Step = 1;
    InFloat.Rf = InterSsAlarmSetUI;
    InputFloat();
}


void InputAc(BYTE Line)
{
    if(IsValidAlarm(ALMID_AC))
    {
        InputAcEx();
    }
    else
    {
        ShowHint("��Ч����",InterSsAlarmSetUI);
    }
}


extern float AlarmThresTemp;


void SetBc()
{
    AlarmTmp.Bc = ToCps(AlarmThresTemp,pnuc[CurRadIndex].Unit);
    InterSsAlarmSetUI();
}

void InputBcEx()
{
    AlarmThresTemp = CpsUnit(AlarmTmp.Bc,pnuc[CurRadIndex].Unit);
        
    if (pnuc[CurRadIndex].Unit == UNIT_CPS)
    {
        FloatEx.Bp = 3;
        FloatEx.Bn = 0;
        FloatEx.Max = 300;
        
    }
    else if (pnuc[CurRadIndex].Unit == UNIT_Bq)
    {
        FloatEx.Bp = 3;
        FloatEx.Bn = 0;
        FloatEx.Max = 300;
    }
    else  // Bq_cm2
    {
        FloatEx.Bp = 3;
        FloatEx.Bn = 2;
        FloatEx.Max = 3;
    }
    FloatEx.Min = 0;
    FloatEx.Val = &(AlarmThresTemp);
    FloatEx.x = 60;
    FloatEx.y = 64;
    FloatEx.Rf = SetBc;
    InputFloatEx();
}


void InputBcBase()
{
    InFloat.Title = "���ر���(@B)";
    InFloat.Spec  = UnitStr[pnuc[CurRadIndex].Unit];
    InFloat.Bits = 0;
    InFloat.Val = &(AlarmTmp.Bc);
    InFloat.Max = 1000;
    InFloat.Min = 0;
    InFloat.Step = 1;
    InFloat.Rf = InterSsAlarmSetUI;
    InputFloat();
}


void InputBc(BYTE Line)
{
    if(IsValidAlarm(ALMID_BC))
    {
        InputBcEx();
    }
    else
    {
        ShowHint("��Ч����",InterSsAlarmSetUI);
    }
}

void InputMx_A()
{
    InFloat.Title = "�ۼƼ�����ֵ";
    InFloat.Spec  = "uSv";
    InFloat.Bits  = 0;
    InFloat.Val = &(AlarmTmp.Mx);
    InFloat.Max = 100000;
    InFloat.Min = 100;
    InFloat.Step = 10;
    InFloat.Rf = InterSsAlarmSetUI;
    InputFloat();
}


void SetMa()
{
    switch (InEx.CurrUnit)
    {
        case 0: 
        {
            *InEx.RetVal = InEx.InputVal;
            break;
        }

        case 1: 
        {
            *InEx.RetVal = InEx.InputVal * 1000;
            break;
        }

        case 2: 
        {
            *InEx.RetVal = InEx.InputVal * 1000000;
            break;
        }

    }

    InterSsAlarmSetUI();
}


// �������뵥λ
void InputMa_Ext()
{
    InEx.Rf_Ok     = SetMa;
    InEx.Rf_Cancel = InterSsAlarmSetUI;
    InEx.UnitCount = 3;
    InEx.UnitList = MxUnit;
    InEx.RetVal = &(AlarmTmp.Ma);
    
    if (AlarmTmp.Ma >= 1000000)
    {
        InEx.CurrUnit = 2;
        InEx.InputVal = AlarmTmp.Ma/1000000;
    }
    else if (AlarmTmp.Ma >= 1000)
    {
        InEx.CurrUnit = 1;
        InEx.InputVal = AlarmTmp.Ma/1000;
    }
    else
    {
        InEx.CurrUnit = 0;
        InEx.InputVal = AlarmTmp.Ma;
    }

    InEx.Title = "�ۼ�Ԥ����ֵ";
    
    
    FloatUnitIndex = 1;
    InputFloatUnit();
}

void SetY3()
{
    switch (InEx.CurrUnit)
    {
        case 0: 
        {
            *InEx.RetVal = InEx.InputVal;
            break;
        }

        case 1: 
        {
            *InEx.RetVal = InEx.InputVal * 1000;
            break;
        }

        case 2: 
        {
            *InEx.RetVal = InEx.InputVal * 1000000;
            break;
        }

    }

    InterSsAlarmSetUI();
}


// �������뵥λ
void InputY3_Ext()
{
    InEx.Rf_Ok     = SetY3;
    InEx.Rf_Cancel = InterSsAlarmSetUI;
    InEx.UnitCount = 3;
    InEx.UnitList = InnerUnit;
    InEx.RetVal = &(AlarmTmp.Y3);
    
    if (AlarmTmp.Y3 >= 1000000)
    {
        InEx.CurrUnit = 2;
        InEx.InputVal = AlarmTmp.Y3/1000000;
    }
    else if (AlarmTmp.Y3 >= 1000)
    {
        InEx.CurrUnit = 1;
        InEx.InputVal = AlarmTmp.Y3/1000;
    }
    else
    {
        InEx.CurrUnit = 0;
        InEx.InputVal = AlarmTmp.Y3;
    }

    InEx.Title = "��������";
    
    
    FloatUnitIndex = 1;
    InputFloatUnit();
}


void SetMx()
{
    switch (InEx.CurrUnit)
    {
        case 0: 
        {
            *InEx.RetVal = InEx.InputVal;
            break;
        }

        case 1: 
        {
            *InEx.RetVal = InEx.InputVal * 1000;
            break;
        }

        case 2: 
        {
            *InEx.RetVal = InEx.InputVal * 1000000;
            break;
        }

    }

    InterSsAlarmSetUI();
}


// �������뵥λ
void InputMx_Ext()
{
    InEx.Rf_Ok     = SetMx;
    InEx.Rf_Cancel = InterSsAlarmSetUI;
    InEx.UnitCount = 3;
    InEx.UnitList = MxUnit;
    InEx.RetVal = &(AlarmTmp.Mx);
    
    if (AlarmTmp.Mx >= 1000000)
    {
        InEx.CurrUnit = 2;
        InEx.InputVal = AlarmTmp.Mx/1000000;
    }
    else if (AlarmTmp.Mx >= 1000)
    {
        InEx.CurrUnit = 1;
        InEx.InputVal = AlarmTmp.Mx/1000;
    }
    else
    {
        InEx.CurrUnit = 0;
        InEx.InputVal = AlarmTmp.Mx;
    }

    InEx.Title = "�ۼƱ�����ֵ";
    
    
    FloatUnitIndex = 1;
    InputFloatUnit();
}


void SetY1()
{
    switch (InEx.CurrUnit)
    {
        case 0:   // uSv/h
        {
            *InEx.RetVal = InEx.InputVal;
            break;
        }

        case 1:  // mSv/h
        {
            *InEx.RetVal = InEx.InputVal * 1000;
            break;
        }

        case 2:  // Sv/h
        {
            *InEx.RetVal = InEx.InputVal * 1000000;
            break;
        }

    }

    InterSsAlarmSetUI();
}


// �������뵥λ
void InputY1_Ext()
{
    InEx.Rf_Ok     = SetY1;
    InEx.Rf_Cancel = InterSsAlarmSetUI;
    InEx.UnitCount = 3;
    InEx.UnitList = InnerUnit;
    InEx.RetVal = &(AlarmTmp.Y1);
    
    if (AlarmTmp.Y1 >= 1000000)
    {
        InEx.CurrUnit = 2;
        InEx.InputVal = AlarmTmp.Y1/1000000;
    }
    else if (AlarmTmp.Y1 >= 1000)
    {
        InEx.CurrUnit = 1;
        InEx.InputVal = AlarmTmp.Y1/1000;
    }
    else
    {
        InEx.CurrUnit = 0;
        InEx.InputVal = AlarmTmp.Y1;
    }

    if (CheckRes.SenserType == SENSER_LP)
    {
        InEx.Title = "������Ԥ��";
    }
    else
    {
        InEx.Title = "һ������";
    }
    
    FloatUnitIndex = 1;
    InputFloatUnit();
}


void SetY2()
{
    switch (InEx.CurrUnit)
    {
        case 0: 
        {
            *InEx.RetVal = InEx.InputVal;
            break;
        }

        case 1: 
        {
            *InEx.RetVal = InEx.InputVal * 1000;
            break;
        }

        case 2: 
        {
            *InEx.RetVal = InEx.InputVal * 1000000;
            break;
        }

    }

    InterSsAlarmSetUI();
}


// �������뵥λ
void InputY2_Ext()
{
    InEx.Rf_Ok     = SetY2;
    InEx.Rf_Cancel = InterSsAlarmSetUI;
    InEx.UnitCount = 3;
    InEx.UnitList = InnerUnit;
    InEx.RetVal = &(AlarmTmp.Y2);
    
    if (AlarmTmp.Y2 >= 1000000)
    {
        InEx.CurrUnit = 2;
        InEx.InputVal = AlarmTmp.Y2/1000000;
    }
    else if (AlarmTmp.Y2 >= 1000)
    {
        InEx.CurrUnit = 1;
        InEx.InputVal = AlarmTmp.Y2/1000;
    }
    else
    {
        InEx.CurrUnit = 0;
        InEx.InputVal = AlarmTmp.Y2;
    }

    if (CheckRes.SenserType == SENSER_LP)
    {
        InEx.Title = "�����ʱ���";
    }
    else
    {
        InEx.Title = "��������";
    }
    
    
    FloatUnitIndex = 1;
    InputFloatUnit();
}


extern INPUTBYTE   InByte;
void InputZu(BYTE Line)
{
    InByte.Title = "����������";
    InByte.Spec  = "%";
    InByte.Max = 100;
    InByte.Min = 1;
    InByte.Step = 1;
    InByte.Rf = InterSsAlarmSetUI;
    InByte.Val = &AlarmTmp.Zu;
    InputByte();
}


void InputAlarm(BYTE Index)
{
     BYTE Loc;
     Loc = GetValidAlarmLoc(Index);
     switch (Loc)
     {
        case 0: InputA1(Index); break;        
        case 1: InputA2(Index); break;
        case 2: InputAL(Index); break;
        case 3: InputAh(Index); break;
        case 4: InputB1(Index); break;
        case 5: InputB2(Index); break;
        case 6: InputBL(Index); break;
        case 7: InputBh(Index); break;
        case 8: InputAc(Index); break;
        case 9: InputBc(Index); break;
        case 10: InputY1_Ext(); break;
        case 11: InputY2_Ext(); break;
        case 12: InputY3_Ext(); break;
        case 13: InputMa_Ext(); break;
        case 14: InputMx_Ext(); break;
        case 15: InputZu(Index);break;
     }
}



// ͬ�����ر���
void AyncRadAlarm()
{
    //static BYTE id;
    if (Senser.Param.Hn != 0)
    {
        //id = GetCurRadIndex();
        //if (GetCurChannel() == CHANNEL_A)
        if (pnuc[CurRadIndex].Channel == CHANNEL_A)
        {
            pnuc[CurRadIndex].AlarmThres = Senser.Alarm.Ac;
        }
        else
        {
            pnuc[CurRadIndex].AlarmThres = Senser.Alarm.Bc;
        }
    }
}


// У�����
BOOL VerifyInAlarm()
{
    if ((AlarmTmp.Y1 <= 0.1) || (AlarmTmp.Y2 <= 0.1) || (AlarmTmp.Y3 <= 0.1))
    {
        return FALSE;
    }
    
    if (AlarmTmp.Y1 >= AlarmTmp.Y2)
    {
        return FALSE;
    }

    if (AlarmTmp.Y2 >= AlarmTmp.Y3)
    {
        return FALSE;
    }

    if (AlarmTmp.Y1 >= AlarmTmp.Y3)
    {
        return FALSE;
    }

    if (AlarmTmp.Y3 >= 1000000)
    {
        return FALSE;
    }

    if (AlarmTmp.Ma >= AlarmTmp.Mx)
    {
        return FALSE;
    }

    if (AlarmTmp.Mx >= 10000000)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VerifyLpAlarm()
{
    if (AlarmTmp.Y1 >= AlarmTmp.Y2)
    {
        return FALSE;
    }

    if (AlarmTmp.Y2 >= 10000000)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VerifyBABAlarm()
{
    if (AlarmTmp.A1 >= AlarmTmp.A2)
    {
        return FALSE;
    }

    if (AlarmTmp.Al >= AlarmTmp.Ah)
    {
        return FALSE;
    }

    if (AlarmTmp.B1 >= AlarmTmp.B2)
    {
        return FALSE;
    }

    if (AlarmTmp.Bl >= AlarmTmp.Bh)
    {
        return FALSE;
    }

    return TRUE;
}


BOOL VerifyABYAlarm()
{
    if (AlarmTmp.A1 >= AlarmTmp.A2)
    {
        return FALSE;
    }

    #if 0
    if (AlarmTmp.Al >= AlarmTmp.Ah)
    {
        return FALSE;
    }
    #endif
    
    return TRUE;
}


BOOL VerifyNaIAlarm()
{
    if ((AlarmTmp.Y1 <= 0.01) || (AlarmTmp.Y2 <= 0.01))
    {
        return FALSE;
    }
    
    if (AlarmTmp.Y1 >= AlarmTmp.Y2)
    {
        return FALSE;
    }

    if (AlarmTmp.Y2 >= 10000000)
    {
        return FALSE;
    }

    #if 0
    if (AlarmTmp.Y2 >= AlarmTmp.Y3)
    {
        return FALSE;
    }

    if (AlarmTmp.Y1 >= AlarmTmp.Y3)
    {
        return FALSE;
    }

    
    if (AlarmTmp.Y3 > 10000000)
    {
        return FALSE;
    }
    #endif

    if (AlarmTmp.Ma >= AlarmTmp.Mx)
    {
        return FALSE;
    }

    if (AlarmTmp.Mx >= 10000000)
    {
        return FALSE;
    }

    return TRUE;
}



BOOL VerifySenserAlarm()
{
    BOOL ret = TRUE;
    
    
    return ret;
}


// ���ر������õ�ǰһ��ҳ��
void AlarmSetRetrun()
{
    if (OutAlarmType == NOR)
    {
        InterMainMenu(); 
    }
    else
    {
        InterRadDetailSetMenu();
    }
}

void SaveAlarm()
{
    // У������ĺϷ���
    if (!VerifySenserAlarm())
    {
        ShowHint("������������", AlarmSetRetrun);
        return;
    }
        
    if (Setting_SenserType == SENSER_IN)
    {
        memcpy(&Param.InSenser.Alarm,&AlarmTmp,sizeof(AlarmTmp));
        //memcpy(&Param.InSenser,&Senser,sizeof(Senser));
        PopHint("���ڱ������");
        WriteParamToFlash();
        
        ShowHint("�������óɹ�", AlarmSetRetrun);
        StorageLog(EVENT_MOD_INALARM);
    }
    else
    {
        PopHint("���ڱ������");

        //  �������·���̽����
        if (!SetSenserAlarm())
        {
            ShowHint("��������ʧ��", AlarmSetRetrun);
        }
        else
        {
            SaveSenserParam();

            // �����µĲ���
            memcpy(&Senser.Alarm,&AlarmTmp,sizeof(AlarmTmp));
            //AyncRadAlarm();
            //WriteParamToFlash();   // �ⲿ̽ͷ�Ĳ������Բ��ñ���
            
            ShowHint("�������óɹ�", AlarmSetRetrun);
            StorageLog(EVENT_MOD_OUTALARM);
        }
    }
}


BOOL AlarmValid()
{
    BOOL ret = TRUE;
    switch (Setting_SenserType)
    {
        case SENSER_IN:  ret = VerifyInAlarm();  break;
        case SENSER_Y:   ret = VerifyNaIAlarm(); break;
        case SENSER_LP:  ret = VerifyLpAlarm();  break;
        case SENSER_B_AB:ret = VerifyBABAlarm();  break;
        case SENSER_ABY: ret = VerifyABYAlarm();  break;
        #if 0
        case SENSER_PD:   ret = VerifyNaIAlarm();    break;
        case SENSER_GM:   ret = VerifyNaIAlarm();    break;
        case SENSER_A_A:  ret = VerifyNaIAlarm();    break;
        case SENSER_A_B:  ret = VerifyNaIAlarm();    break;
        case SENSER_A_AB: ret = VerifyNaIAlarm();    break;
        case SENSER_B_B:  ret = VerifyNaIAlarm();    break;
        case SENSER_B_A:  ret = VerifyNaIAlarm();    break;
        case SENSER_B_AB: ret = VerifyNaIAlarm();    break;
        case SENSER_G1:   ret = VerifyNaIAlarm();    break;
        case SENSER_G2:   ret = VerifyNaIAlarm();    break;
        #endif
    }

    return ret;
}


void SsAlarmKey(PRESSKEY  key)
{
    switch(key)
    {
        case TIMEOUT: 
        case POWER:   EnterMeasuInter(); SsAlarmIndex = 1;       break;

        case RETURN:  
        {
            if (OutAlarmType == NOR)
            {
                InterMainMenu(); 
            }
            else
            {
                InterRadDetailSetMenu();
            }
            SsAlarmIndex = 1;      
            break;
        }

        case OKKEY:   
        {
            if (SsAlarmIndex == (SSALARMCNT+1))
            {
                // ���
                //  �������·���̽����
                if (AlarmValid())
                {
                    SsAlarmIndex = 1;
                    SaveAlarm();
                }
                else
                {
                    ShowHint("�����Ƿ�",InterSsAlarmSetUI);
                }
                
                //InterMainMenu();
            }
            else
            {
                InputAlarm(SsAlarmIndex-1);     
            }
        }
        break;

        case UP:
        {
            if(--SsAlarmIndex < 1)
            {
                SsAlarmIndex = SSALARMCNT+1;
            }
            ShowSsAlarmSetUI();
        }
        break;
        
        case DOWN:
        {
            if(++SsAlarmIndex > (SSALARMCNT+1))
            {
                SsAlarmIndex = 1;
            }
            ShowSsAlarmSetUI();
        }
        break;
    }
}

