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
#include "Senser.h"
#include "Main.h"
#include "Input.h"
#include "SelfCheck.h"
#include "Adc12.h"
#include "Hvadc.h"

extern CHECKRES CheckRes;
extern const SENSERCONTXT SenserContxt[SENSERCOUNT];
extern SENSER Senser;
extern INTERFACE currUI;
extern QPara Param;
extern BYTE Setting_SenserType;

extern INPUTDWORD  InDword;
extern INPUTFLOAT InFloat;
extern INPUTBYTE  InByte;
extern FLOATEX    FloatEx;
extern char IsSuperUser;


BYTE SsParamIndex = 1;
BYTE SsParamPage = 0;
static BYTE SSPARAMCNT = 6;


char * const ParamText[] =
{
    "高压值",        // 1
    "甄别阈值",
    "计数时间",
    "高压误差",
    "甄别阈值2",    // 5
    "甄别阈值3",
    "修正系数1",
    "修正系数2",
    "校准因子",
    //"剂量阈值",     
    "启用核素",    // 10 
    "修正系数3",   // 新增加修正系数3
    "甄别阈值4",
    "平滑时间",
    "低量程A",      // 0x0E
    "低量程B",
    "低量程C",      // 0x10
    "高量程A",     
    "高量程B",
    "高量程C",
    "探头面积",
    "扣除本底"
};

// 水下PD探头参数
char * const ParamText_PD[] =
{
    "高压值",        // 1
    "甄别阈值",
    "计数时间",
    "高压误差",
    "甄别阈值2",    // 5
    "甄别阈值3",
    "修正系数A1",
    "修正系数A2",
    "校准因子",
    //"剂量阈值",     
    "启用核素",    // 0x0A
    "修正系数A3",  // 新增加修正系数3
    "甄别阈值4",
    "平滑时间",
    "修正系数A4",      // 0x0E
    "修正系数B1",
    "修正系数B2",      // 0x10
    "修正系数A",    
    "修正系数B",
    "修正系数C",
    "探头面积",
    "扣除本底"
};



char ParamValStr[20] = {0};
SENSERPARAM ParamTmp;    // 参数临时存储
SENSERPARAM const *ParamMax;   //  参数最大值
SENSERPARAM const *ParamMin;   //  参数最小值

void ShowParamValue(BYTE ParamId, BYTE loc)
{
    if (ParamId == 20)
    {
        Show12X12String(70,20+(loc*16),ParamValStr,0);
    }
    else
    {
        Show5X7String(70,23+(loc*16),ParamValStr,0);
    }
}

// 获取第Index 个参数的位置
BYTE GetValidParamLoc(BYTE Index)
{
    BYTE j,k;
    BYTE i = GetSenserIndex(Setting_SenserType);
    k = 0;
    for (j=0;j<PARAMCOUNT;j++) 
    {
        if (SenserContxt[i].ParamLoc[j] == TRUE)
        {
            if (Index == k) break;
            k++;
        }
    }
    return j;
}

void ShowSsParamSetUI()
{   
    BYTE i,Loc;
    
    Clear_DispBuf();
    if (Setting_SenserType == SENSER_IN)
    {
        GT_Show16X16Char(8,0,"内部探测器参数",0);
    }
    else
    {
        GT_Show16X16Char(8,0,"外部探测器参数",0);
    }

    // 一页只能显示6个参数, 超过6个要翻页
    if (SSPARAMCNT > PAGE_MENUCOUNT)
    {
        if (SsParamIndex > PAGE_MENUCOUNT)
        {
            SsParamPage = 1;
        }
        else
        {
            SsParamPage = 0;
        }
        
        if (SsParamPage == 0)
        {
            for (i=0;i<PAGE_MENUCOUNT;i++)
            { 
                Loc = GetValidParamLoc(i);
                if (Setting_SenserType == SENSER_PD)
                {
                    Show12X12String(0,20+(i*16),ParamText_PD[Loc],SsParamIndex==i+1);
                }
                else
                {
                    Show12X12String(0,20+(i*16),ParamText[Loc],SsParamIndex==i+1);
                }
                Show5X7String(65,23+(i*16),":",0);
                GetPararmValStr(Loc);
                ShowParamValue(Loc,i);
            }

            Show5X7String(20,20+(i*16),"...",0);
        }
        else
        {
            for (i=PAGE_MENUCOUNT;i<SSPARAMCNT;i++)
            { 
                Loc = GetValidParamLoc(i);
                if (Setting_SenserType == SENSER_PD)
                {
                    Show12X12String(0,20+((i-PAGE_MENUCOUNT)*16),ParamText_PD[Loc],SsParamIndex==i+1);
                }
                else
                {
                    Show12X12String(0,20+((i-PAGE_MENUCOUNT)*16),ParamText[Loc],SsParamIndex==i+1);
                }
                Show5X7String(65,23+((i-PAGE_MENUCOUNT)*16),":",0);
                GetPararmValStr(Loc);
                ShowParamValue(Loc,i-PAGE_MENUCOUNT);
                //Show5X7String(70,23+((i-PAGE_MENUCOUNT)*16),ParamValStr,0);
            }

            GT_Show12X12Char(100,20+((i-PAGE_MENUCOUNT)*16),"保存",SsParamIndex == (SSPARAMCNT+1));
        }
    }
    else
    {
        SsParamPage = 0;
        for (i=0;i<SSPARAMCNT;i++)
        { 
            Loc = GetValidParamLoc(i);
            if (Setting_SenserType == SENSER_PD)
            {
                Show12X12String(0,20+(i*16),ParamText_PD[Loc],SsParamIndex==i+1);
            }
            else
            {
                Show12X12String(0,20+(i*16),ParamText[Loc],SsParamIndex==i+1);
            }
            Show5X7String(65,23+(i*16),":",0);
            GetPararmValStr(Loc);
            ShowParamValue(Loc,i);
            //Show5X7String(70,23+(i*16),ParamValStr,0);
        }

        // 完成按钮
        GT_Show12X12Char(100,20+(i*16),"保存",SsParamIndex == (SSPARAMCNT+1));
    }
    
    ShowOptHint();
    DisplayRefresh();
}

#if 0
    DWORD  Hv; // 0x01+高压值
    DWORD  Z1;  //0x02+甄别器阈值1
    DWORD  Ct;  // 0x03+计数时间
    DWORD  Hd;  //0x04+高压误
    DWORD  Z2;  // 0x05+甄别器阈值2
    DWORD  Z3;  //0x06+甄别器阈值3
    float  S1;  // 0x07+探测器修正系数1
    float  S2;  //0x08+探测器修正系数2
    DWORD  Cr;  // 0x09+校准因子
    DWORD  Hn;  //0x0A+启用核素号
#endif

void InputHv(ReturnFun rf,BYTE Line)
{
    #if 0
    InFloat.Title = "探测器高压";
    InFloat.Spec  = "V";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.Hv);
    InFloat.Max = ParamMax->Hv;
    InFloat.Min = ParamMin->Hv;
    InFloat.Step = 1;
    InFloat.Rf = rf;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 0;
    FloatEx.Max = ParamMax->Hv;
    FloatEx.Min = ParamMin->Hv;
    FloatEx.Val = &(ParamTmp.Hv);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = rf; //InterSsParamSetUI;
    InputFloatEx();
    #endif
}

void InputZ1(ReturnFun rf, BYTE Line)
{
    #if 0
    InFloat.Title = "甄别器阈值";
    InFloat.Spec  = "mV";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.Z1);
    InFloat.Max = ParamMax->Z1;
    InFloat.Min = ParamMin->Z1;
    InFloat.Step = 10;
    InFloat.Rf = rf;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 0;
    FloatEx.Max = ParamMax->Z1;
    FloatEx.Min = ParamMin->Z1;
    FloatEx.Val = &(ParamTmp.Z1);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = rf; //InterSsParamSetUI;
    InputFloatEx();
    #endif
}

void InputCt(BYTE Line)
{
    #if 0
    InFloat.Title = "探头计数时间";
    InFloat.Spec  = "ms";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.Ct);
    InFloat.Max = ParamMax->Ct;
    InFloat.Min = ParamMin->Ct;
    InFloat.Step = 100;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 0;
    FloatEx.Max = ParamMax->Ct;
    FloatEx.Min = ParamMin->Ct;
    FloatEx.Val = &(ParamTmp.Ct);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}

void InputHd(BYTE Line)
{
    #if 0
    InFloat.Title = "探头高压误差";
    InFloat.Spec  = "%";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.Hd);
    InFloat.Max = ParamMax->Hd;
    InFloat.Min = ParamMin->Hd;
    InFloat.Step = 1;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 3;
    FloatEx.Bn = 0;
    FloatEx.Max = ParamMax->Hd;
    FloatEx.Min = ParamMin->Hd;
    FloatEx.Val = &(ParamTmp.Hd);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}

void InputZ2(BYTE Line)
{
    #if 0
    InFloat.Title = "甄别器阈值2";
    InFloat.Spec  = "mV";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.Z2);
    InFloat.Max = ParamMax->Z2;
    InFloat.Min = ParamMin->Z2;
    InFloat.Step = 10;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 0;
    FloatEx.Max = ParamMax->Z2;
    FloatEx.Min = ParamMin->Z2;
    FloatEx.Val = &(ParamTmp.Z2);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}


void InputZ3(BYTE Line)
{
    #if 0
    InFloat.Title = "甄别器阈值3";
    InFloat.Spec  = "mV";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.Z3);
    InFloat.Max = ParamMax->Z3;
    InFloat.Min = ParamMin->Z3;
    InFloat.Step = 10;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 0;
    FloatEx.Max = ParamMax->Z3;
    FloatEx.Min = ParamMin->Z3;
    FloatEx.Val = &(ParamTmp.Z3);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}

void InputZ4(BYTE Line)
{
    #if 0
    InFloat.Title = "甄别器阈值4";
    InFloat.Spec  = "mV";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.Z4);
    InFloat.Max = ParamMax->Z4;
    InFloat.Min = ParamMin->Z4;
    InFloat.Step = 10;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 0;
    FloatEx.Max = ParamMax->Z4;
    FloatEx.Min = ParamMin->Z4;
    FloatEx.Val = &(ParamTmp.Z4);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}



void InputPt(BYTE Line)
{
    #if 0
    InFloat.Title = "平滑时间";
    InFloat.Spec  = "s";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.Pt);
    InFloat.Max = ParamMax->Pt;
    InFloat.Min = ParamMin->Pt;
    InFloat.Step = 1;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 3;
    FloatEx.Bn = 0;
    FloatEx.Max = ParamMax->Pt;
    FloatEx.Min = ParamMin->Pt;
    FloatEx.Val = &(ParamTmp.Pt);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}


void InputLA(BYTE Line)
{
    #if 1
    if (!IsSuperUser) 
    {
        ShowHint("没有权限",InterSsParamSetUI);
        return;
    }
    #endif
    
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(ParamTmp.LA);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
}

void InputLB(BYTE Line)
{
    #if 1
    if (!IsSuperUser) 
    {
        ShowHint("没有权限",InterSsParamSetUI);
        return;
    }
    #endif
    
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(ParamTmp.LB);
    FloatEx.x = 60;
    FloatEx.y = 20 + (Line % 6) * 16;;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
}

void InputLC(BYTE Line)
{
    #if 0
    if (!IsSuperUser) 
    {
        ShowHint("没有权限",InterSsParamSetUI);
        return;
    }
    #endif
    
    FloatEx.Bp = 1;
    FloatEx.Bn = 2;
    FloatEx.Max = 9.99;
    FloatEx.Min = 0.01;
    FloatEx.Val = &(ParamTmp.LC);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;;
    
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
}


void InputHA(BYTE Line)
{
    #if 1
    if (!IsSuperUser) 
    {
        ShowHint("没有权限",InterSsParamSetUI);
        return;
    }
    #endif
    
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(ParamTmp.HA);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
}

void InputHB(BYTE Line)
{
    #if 1
    if (!IsSuperUser) 
    {
        ShowHint("没有权限",InterSsParamSetUI);
        return;
    }
    #endif
    
    FloatEx.Bp = 2;
    FloatEx.Bn = 6;
    FloatEx.Max = 99.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(ParamTmp.HB);
    FloatEx.x = 60;
    FloatEx.y = 20 + (Line % 6) * 16;;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
}

void InputHC(BYTE Line)
{
    #if 0
    if (!IsSuperUser) 
    {
        ShowHint("没有权限",InterSsParamSetUI);
        return;
    }
    #endif
    
    FloatEx.Bp = 1;
    FloatEx.Bn = 2;
    FloatEx.Max = 9.99;
    FloatEx.Min = 0.01;
    FloatEx.Val = &(ParamTmp.HC);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;;
    
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
}


void InputSs(BYTE Line)
{
    FloatEx.Bp = 4;
    FloatEx.Bn = 0;
    FloatEx.Max = 9999.9;
    FloatEx.Min = 0.0;
    FloatEx.Val = &(ParamTmp.Ss);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
}


void InputS1(BYTE Line)
{
    #if 0
    InFloat.Title = "探头修正系数1";
    InFloat.Spec  = "";
    InFloat.Bits  = 2;
    InFloat.Val = &(ParamTmp.S1);
    InFloat.Max = ParamMax->S1;
    InFloat.Min = ParamMin->S1;
    InFloat.Step = 0.01;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 2;
    FloatEx.Max = ParamMax->S1;
    FloatEx.Min = ParamMin->S1;
    FloatEx.Val = &(ParamTmp.S1);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}

void InputS2(BYTE Line)
{
    #if 0
    InFloat.Title = "探头修正系数2";
    InFloat.Spec  = "";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.S2);
    InFloat.Max = ParamMax->S2;
    InFloat.Min = ParamMin->S2;
    InFloat.Step = 100;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 2;
    FloatEx.Max = ParamMax->S2;
    FloatEx.Min = ParamMin->S2;
    FloatEx.Val = &(ParamTmp.S2);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}

void InputCr(BYTE Line)
{
    #if 0
    InFloat.Title = "校准因子";
    InFloat.Spec  = "";
    InFloat.Bits  = 2;
    InFloat.Val = &(ParamTmp.Cr);
    InFloat.Max = ParamMax->Cr;
    InFloat.Min = ParamMin->Cr;
    InFloat.Step = 0.01;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 2;
    FloatEx.Max = ParamMax->Cr;
    FloatEx.Min = ParamMin->Cr;
    FloatEx.Val = &(ParamTmp.Cr);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}

void InputCr_Ex()
{
    FloatEx.Bp = 2;
    FloatEx.Bn = 2;
    FloatEx.Max = 10;
    FloatEx.Min = 0.1;
    FloatEx.x = 70;

    switch (Setting_SenserType)
    {
        case SENSER_IN: FloatEx.y = 32;  break;
        case SENSER_A_A: FloatEx.y = 80; break;
        default : FloatEx.y = 70; break;
    }
    
    
    FloatEx.Val = &(ParamTmp.Cr);
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
}

void InputS3(BYTE Line)
{
    #if 0
    InFloat.Title = "探头修正系数3";
    InFloat.Spec  = "";
    InFloat.Bits  = 0;
    InFloat.Val = &(ParamTmp.S3);
    InFloat.Max = ParamMax->S3;
    InFloat.Min = ParamMin->S3;
    InFloat.Step = 100;
    InFloat.Rf = InterSsParamSetUI;
    InputFloat();
    #else
    FloatEx.Bp = 4;
    FloatEx.Bn = 2;
    FloatEx.Max = ParamMax->S3;
    FloatEx.Min = ParamMin->S3;
    FloatEx.Val = &(ParamTmp.S3);
    FloatEx.x = 70;
    FloatEx.y = 20 + (Line % 6) * 16;
    FloatEx.Rf = InterSsParamSetUI;
    InputFloatEx();
    #endif
}


void InputHn()
{
    InByte.Title = "启用核素号";
    InByte.Spec  = "";
    InByte.Val = &(ParamTmp.Hn);
    InByte.Max = ParamMax->Hn;
    InByte.Min = ParamMin->Hn;
    InByte.Step = 1;
    InByte.Rf = InterSsParamSetUI;
    InputByte();
}

void InputBk(BYTE Index)
{
    if (ParamTmp.Bk)
    {
        ParamTmp.Bk = 0;
    }
    else
    {
        ParamTmp.Bk = 1;
    }
    ShowSsParamSetUI();
    //ShowComfirm(char * Hint,ReturnFun Rf_Ok,ReturnFun Rf_Cancel)
}


void GetPararmValStr(BYTE Index)
{
    memset(ParamValStr,0,20);
    switch(Index)
    {
        case 0: sprintf(ParamValStr,"%0.0fv",   ParamTmp.Hv);   break;
        case 1: sprintf(ParamValStr,"%0.0fmv",  ParamTmp.Z1);   break;
        case 2: sprintf(ParamValStr,"%0.0fms",  ParamTmp.Ct);   break;
        case 3: sprintf(ParamValStr,"%0.2f%%",  ParamTmp.Hd);   break;
        case 4: sprintf(ParamValStr,"%0.0fmv",  ParamTmp.Z2);   break;
        case 5: sprintf(ParamValStr,"%0.0fmv",  ParamTmp.Z3);   break;
        case 6: sprintf(ParamValStr,"%0.2f",    ParamTmp.S1);   break;
        case 7: sprintf(ParamValStr,"%0.2f",    ParamTmp.S2);   break;
        case 8: sprintf(ParamValStr,"%0.2f",    ParamTmp.Cr);   break;
        case 9: sprintf(ParamValStr,"%d",       ParamTmp.Hn);   break;
        case 10: sprintf(ParamValStr,"%0.2f",    ParamTmp.S3);   break;  
        case 11: sprintf(ParamValStr,"%0.0fmv",    ParamTmp.Z4);   break;  

        case 12: sprintf(ParamValStr,"%0.0fs",  ParamTmp.Pt);   break;  
        
        case 13: // sprintf(ParamValStr,"%0.2f",   ParamTmp.LA);  break;
        {
            if ( (IsSuperUser)  || 
                 (Setting_SenserType == SENSER_PD) ||
                 (Setting_SenserType == SENSER_GM)
                )
            {
                sprintf(ParamValStr,"%0.6f",   ParamTmp.LA);
            }
            else 
            {
                sprintf(ParamValStr,"---");
            }
        }
        break;
        
        case 14:  // sprintf(ParamValStr,"%0.6f",   ParamTmp.LB);   break;
        {
            if ( (IsSuperUser) || 
                 (Setting_SenserType == SENSER_PD) ||
                 (Setting_SenserType == SENSER_GM)
                )
            {
                sprintf(ParamValStr,"%0.6f",   ParamTmp.LB);
            }
            else 
            {
                sprintf(ParamValStr,"---");
            }
        }
        break;
        
        case 15: sprintf(ParamValStr,"%0.2f",   ParamTmp.LC);   break;
        
        case 16: //sprintf(ParamValStr,"%0.2f",   ParamTmp.HA);   break;
        {
            if ( (IsSuperUser)  || 
                 (Setting_SenserType == SENSER_PD) ||
                 (Setting_SenserType == SENSER_GM)
               )
            {
                sprintf(ParamValStr,"%0.6f",   ParamTmp.HA);
            }
            else 
            {
                sprintf(ParamValStr,"---");
            }
        }
        break;
        
        case 17: //sprintf(ParamValStr,"%0.6f",   ParamTmp.HB);   break;
        {
            if ( (IsSuperUser)  || 
                 (Setting_SenserType == SENSER_PD) ||
                 (Setting_SenserType == SENSER_GM)
               )
            {
                sprintf(ParamValStr,"%0.6f",   ParamTmp.HB);
            }
            else 
            {
                sprintf(ParamValStr,"---");
            }
        }
        break;
        
        case 18: sprintf(ParamValStr,"%0.2f",   ParamTmp.HC);        break;
        case 19: sprintf(ParamValStr,"%0.0fcm@2",   ParamTmp.Ss);    break;
        case 20: sprintf(ParamValStr,"%s",   ParamTmp.Bk?"是":"否"); break;
        
    }
}


float GetPararmVal(BYTE Index)
{
    float ret;
    switch(Index)
    {
        case 0: ret = ParamTmp.Hv;           break;
        case 1: ret = 2500 - ParamTmp.Z1;   break;
        case 2: ret = ParamTmp.Ct;           break;
        case 3: ret = ParamTmp.Hd;           break;
        case 4: ret = 2500 - ParamTmp.Z2;   break;
        case 5: ret = 2500 - ParamTmp.Z3;   break;
        case 6: ret = ParamTmp.S1;           break;
        case 7: ret = ParamTmp.S2;           break;
        case 8: ret = ParamTmp.Cr;           break;
        //case 9: ret = ParamTmp.Hn;   break;  // 启用核素不再这里处理了
        case 10: ret = ParamTmp.S3;          break;
        case 11: ret = 2500 - ParamTmp.Z4;  break;
        case 12: ret = ParamTmp.Pt;          break;
        case 13: ret = ParamTmp.LA;          break;
        case 14: ret = ParamTmp.LB;          break;
        case 15: ret = ParamTmp.LC;          break;
        case 16: ret = ParamTmp.HA;          break;
        case 17: ret = ParamTmp.HB;          break;
        case 18: ret = ParamTmp.HC;          break;
        case 19: ret = ParamTmp.Ss;          break;
        case 20: ret = ParamTmp.Bk;          break;
    }
    return ret;
}


void InputParam(BYTE Index)
{
     BYTE Loc;
     Loc = GetValidParamLoc(Index);
     switch (Loc)
     {
        case 0: InputHv(InterSsParamSetUI,Index); break;
        case 1: InputZ1(InterSsParamSetUI,Index);  break;
        case 2: InputCt(Index); break;
        case 3: InputHd(Index); break;
        case 4: InputZ2(Index); break;
        case 5: InputZ3(Index); break;
        case 6: InputS1(Index); break;
        case 7: InputS2(Index); break;
        case 8: InputCr_Ex();    break;
        case 9: InputHn();        break;
        case 10: InputS3(Index);  break;
        case 11: InputZ4(Index); break;

        case 12: InputPt(Index); break;
        case 13: InputLA(Index); break;
        case 14: InputLB(Index); break;
        case 15: InputLC(Index); break;
        case 16: InputHA(Index); break;
        case 17: InputHB(Index); break;
        case 18: InputHC(Index); break;
        
        case 19: InputSs(Index);  break;
        case 20: InputBk(Index);  break;
     }
}


void SaveParam(ReturnFun Rf)
{
    if (Setting_SenserType == SENSER_IN)
    {
        memcpy(&Param.InSenser.Param,&ParamTmp,sizeof(ParamTmp));
        //memcpy(&Param.InSenser,&Senser,sizeof(Senser));
        PopHint("正在更新参数");
        WriteParamToFlash();
        ShowHint("参数更新成功", Rf);
        StorageLog(EVENT_MOD_INPARAM);
    }
    else
    {
        PopHint("正在更新参数");
        
        //  将参数下发到探测器
        if (!SetSenserParam())
        {
            ShowHint("参数设置失败", Rf);
            return;
        }
        
        if (!SaveSenserParam())
        {
            ShowHint("参数保存失败", Rf);
            return;
        }
        
        // 启用新的参数
        memcpy(&Senser.Param,&ParamTmp,sizeof(ParamTmp));
        //Out_HV_Val((WORD)Senser.Param.Hv); 

        if (NeedHv())
        {
            OpenHv((WORD)Senser.Param.Hv);
        }
        else
        {
            CloseHv();
        }
        
        WriteParamToFlash();
        
        
        //Sleep(200);      // 等高压稳定后开启AD
        //ADC12_Start();  // 启用参数后要重新开启AD
        
        ShowHint("参数更新成功", Rf);
        StorageLog(EVENT_MOD_OUTPARAM);
        
    }

    
}


void InterSsParamSetUI()
{
    BYTE i;
    currUI = SSPARAM;
    
    i = GetSenserIndex(Setting_SenserType);
    SSPARAMCNT = SenserContxt[i].ParmCnt;
    

    #if 0
    // 特殊处理: AB探头的核素启用参数不显示在这里
    // 不在这里处理了，在参数总表中设置
    if ((CheckRes.SenserType == SENSER_A_A) ||
         (CheckRes.SenserType == SENSER_A_B) ||
         (CheckRes.SenserType == SENSER_A_AB) ||
         (CheckRes.SenserType == SENSER_B_A) ||
         (CheckRes.SenserType == SENSER_B_B) ||
         (CheckRes.SenserType == SENSER_B_AB))
    {
        SSPARAMCNT -= 1;
    }
    #endif
    
    ShowSsParamSetUI();
}


BOOL ParamValid()
{
    if (Setting_SenserType == SENSER_IN)
    {
        
    }
    else
    {
        
    }

    return TRUE;
}

void SsParamKey(PRESSKEY  key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter();  break;
        case RETURN:  InterMainMenu();                 break;
        case OKKEY:   
        {
            if (SsParamIndex == (SSPARAMCNT+1))
            {
                if (ParamValid())
                {
                    SsParamIndex = 1;
                    SaveParam(InterMainMenu);
                }
                else
                {
                    ShowHint("参数非法",InterSsParamSetUI);
                }
            }
            else
            {
                InputParam(SsParamIndex-1);     
            }
        }
        break;

        case UP:
        {
            if(--SsParamIndex < 1)
            {
                SsParamIndex = SSPARAMCNT+1;
            }
            ShowSsParamSetUI();
        }
        break;
        
        case DOWN:
        {
            if(++SsParamIndex > (SSPARAMCNT+1))
            {
                SsParamIndex = 1;
            }
            ShowSsParamSetUI();
        }
        break;
    }
}

