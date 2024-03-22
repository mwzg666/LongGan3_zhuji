#include <stdio.h>
#include <string.h>

#include <msp430x552x.h>

#include "system.h"
#include "CommDataDefine.h"
#include "Input.h"

#include "TMeas.h"
#include "Measinter.h"
#include "Oled.h"
#include "Main.h"
#include "QuickMenu.h"

BOOL   bTimerMeas = FALSE;
BYTE   MeasTime = 0;


extern INPUTBYTE   InByte;
extern QInnerParam InnerParam;
extern INTERFACE currUI;
extern char StrTemp[24];
extern METEFACEINFO FaceInfo;

void StartTimerMeas()
{
    bTimerMeas = TRUE;
    MeasTime = InnerParam.MeasTime;
    EnterMeasuInter();
}

// 定时测量
void EnterTimerMeas()
{
    InByte.Title = "定时测量时间";
    InByte.Spec  = "S";
    InByte.Max = 120;
    InByte.Min = 10;
    InByte.Step = 10;
    InByte.Rf = StartTimerMeas;
    InByte.Val = &InnerParam.MeasTime;
    InputByte();
}

BYTE MeasIndex = 1;
void ShowMeasRes()
{
    currUI = MEASRES;

    Clear_DispBuf();
    ShowMeteTitle();
    
    
    PopWindow(15,28,98,80,"测量结果");

    memset(StrTemp,0,24);
    sprintf(StrTemp," @A:%.1f CPS", FaceInfo.OutA_Cur_Rt);
    Show12X12String(20,44,StrTemp,0);

    memset(StrTemp,0,24);
    sprintf(StrTemp,"@B@Y:%.1f CPS", FaceInfo.OutB_Cur_Rt);
    Show12X12String(20,64,StrTemp,0);
    
    //Show12X12String(30,74,"清累计剂量",0);
    Show12X12String(30,90,"保存",MeasIndex == 1);
    Show12X12String(70,90,"结束",MeasIndex == 2);
    
    ShowOptHint();
    DisplayRefresh();
}


void MeasResKey(PRESSKEY  key)
{
    switch(key)
    {
        case TIMEOUT:  
        case POWER:  
        case RETURN:   EnterMeasuInter();  break;
        
        case DOWN:
        {
            if (MeasIndex == 1) 
            {
                MeasIndex = 2; 
                ShowMeasRes();
            }
            break;   
        }
        
        case UP:    
        {
            if (MeasIndex == 2) 
            {
                MeasIndex = 1;
                ShowMeasRes();
            }
            break;
        }
        
        case MODE:   break;
       
        case OKKEY:  
        {
            switch (MeasIndex)
            {
                case 1: 
                {
                    bTimerMeas = TRUE;
                    MeasTime = InnerParam.MeasTime;
                    SaveDose(); 
                    break;  
                }
                
                case 2: 
                {
                    bTimerMeas = FALSE;
                    EnterMeasuInter();
                    break;
                }
            }
        }
        break;
    }
}

