#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <msp430.h>
#include "system.h"
#include "CommDataDefine.h"

#include "Senser.h"
#include "Mcp2510.h"
#include "Main.h"
#include "SsParam.h"
#include "SsAlarm.h"
#include "Input.h"
#include "MainMenu.h"
#include "Oled.h"
#include "Measinter.h"
#include "SelfCheck.h"
#include "Adc12.h"
#include "SsParam.h"
#include "SsSrv.h"

extern INTERFACE currUI;
extern SENSER Senser;
extern CHECKRES CheckRes;
extern METEFACEINFO FaceInfo;
extern QPara Param;
extern BYTE Setting_SenserType;
extern SENSERPARAM ParamTmp;    // 参数临时存储
extern SENSERPARAM const *ParamMax;   //  参数最大值
extern SENSERPARAM const *ParamMin;   //  参数最小值
extern char StrTemp[24];
extern BATTERY SenserBatery;
extern BOOL DebugMode;
extern BYTE BTErrCode;
extern BYTE ElecTestChannel;

#if 0
void Elec_PdSenser()
{

}

void Elec_GmSenser()
{

}

void Elec_AbSenser()
{

}

void Elec_AabSenser()
{

}

void Elec_BbSenser()
{

}

void Elec_BaSenser()
{

}


void Elec_BabSenser()
{

}

void Elec_G1Senser()
{

}

void Elec_G2Senser()
{

}
#endif

void ShowSenserVer(BYTE x, BYTE y)
{
    BYTE m,s;
    WORD n;
    m = (BYTE)(CheckRes.SwVer & 0xFF);
    s = (BYTE)((CheckRes.SwVer>>8) & 0xFF);
    n = (WORD)((CheckRes.SwVer>>16) & 0xFFFF);
    memset(StrTemp,0,24);
    sprintf(StrTemp,"(V%d.%d.%d)", m,s,n);
    Show5X7String(x,y,StrTemp,0);     // 版本
}

void ShowTestMode(BYTE x, BYTE y)
{
    switch (Senser.WorkSt)
    {
        case 1:   Show12X12String(x,y,"电测试",0);  break;
        case 2:   Show12X12String(x,y,"物理测试",0); break;
        case 3:   Show12X12String(x,y,"辐射测试",0); break;
        default:  Show12X12String(x,y,"状态异常",0); break;
    }
}

void GetTestCps()
{
    BYTE i,j;
    

    if (!HvValid())
    {
        SetAlarm(ALARM_HV_ERR);
    }
    else
    {
        ClrAlarm(ALARM_HV_ERR);
    }
    
    if (GetSenserCounter())
    {
        ClrAlarm(ALARM_SENSER_ERR);
        //GetSenserWork();

        #if 0
        switch (Senser.WorkSt)
        {
            case 1:   Show12X12String(0,16,"电测试",0);  break;
            case 2:   Show12X12String(0,16,"物理测试",0); break;
            case 3:   Show12X12String(0,16,"辐射测试",0); break;
            default:  Show12X12String(0,16,"状态异常",0); break;
        }
        #endif

        
        for (i=0;i<Senser.Contxt->CounterCnt;i++)
        {
            j = GetValidCounterLoc(i);

            switch (i)
            {
                case 0: FaceInfo.OutA_Cps_Rt =  GetCounterVal(j); break;
                case 1: FaceInfo.OutB_Cps_Rt =  GetCounterVal(j); break;
            }
        }
    }
    else
    {
        SetAlarm(ALARM_SENSER_ERR);
    }
}

void Elec_AaSenser()
{
    //GetTestCps();
    ShowTestMode(0,16);

    // 2018.8.20  -- 外部NaI探头没有版本信息
    if (CheckRes.SenserType != SENSER_Y)
    {
        ShowSenserVer(52,19);
    }
    
    //FaceInfo.OutA_Cur = CpsUnit(FaceInfo.OutA_Cps, 
    //                              Param.pnuc[CurRadIndex].Unit);
    ShowCounter(0,30,"",FaceInfo.OutA_Cps_Rt,UNIT_CPS);
}


void Elec_GmSenser()
{
    //GetTestCps();
    ShowTestMode(0,16);

    if (ElecTestChannel == 0)
    {
        Show5X7String(40,19,"L",0);
    }
    else
    {
        Show5X7String(40,19,"H",0);
    }
    
    // 2018.8.20  -- 外部NaI探头没有版本信息
    if (CheckRes.SenserType != SENSER_Y)
    {
        ShowSenserVer(52,19);
    }
    
    //FaceInfo.OutA_Cur = CpsUnit(FaceInfo.OutA_Cps, 
    //                              Param.pnuc[CurRadIndex].Unit);
    ShowCounter(0,30,"",FaceInfo.OutA_Cps_Rt,UNIT_CPS);
}


void Elec_ABSenser()
{
    //GetTestCps();
    ShowTestMode(0,16);

    // 2018.8.20  -- 外部NaI探头没有版本信息
    if (CheckRes.SenserType != SENSER_Y)
    {
        ShowSenserVer(52,19);
    }
    //FaceInfo.OutA_Cur = CpsUnit(FaceInfo.OutA_Cps, 
    //                              Param.pnuc[CurRadIndex].Unit);
    ShowCounterMid(6,24,"@A",FaceInfo.OutA_Cps_Rt,0);
    ShowCounterMid(0,48,"@B@Y",FaceInfo.OutB_Cps_Rt,UNIT_CPS);
}



void EndElecTest()
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        MCP2510_WakeUp();
    }
    
    if (SetSenserWork(3,0))
    {        
        Senser.WorkSt = 3;
        ShowHint("电测试完成",InterMainMenu);
    }
    else
    {
        ShowHint("电测试失败",InterMainMenu);
    }
}

void SwChannel()
{
    if (ElecTestChannel == 0)
    {
        ElecTestChannel = 1;
        SetSenserWork(1,1);
    }
    else
    {
        ElecTestChannel = 0;
        SetSenserWork(1,0);
    }
}



BYTE ElecTestTime = 60;
void ShowElecTestUI()
{
    //static BYTE time = 60;
    //char str[10] = {0};
    GetTestCps();
    
    Clear_DispBuf();

    ShowMeteTitle();
    
    #if 1
    switch (CheckRes.SenserType)
    {
        case SENSER_IN:   Elec_AaSenser();     break;
        case SENSER_PD:   Elec_AaSenser();     break;   //         0x02    //水下高量程探头（PD）
        case SENSER_GM:   Elec_GmSenser();     break;  //         0x03    //水下中高量程探头（GM）
        case SENSER_A_A:  Elec_AaSenser();     break;    //          0x05        α探头(仅测alphy)
        case SENSER_A_B:  Elec_AaSenser();     break;  //        0x14    //α探头(仅测beta)
        case SENSER_A_AB: Elec_ABSenser();    break;  //      0x24    //α探头(可测alphy+beta)
        case SENSER_B_B:  Elec_AaSenser();     break;   //       0x05    //β探头(仅测beta)
        case SENSER_B_A:  Elec_AaSenser();     break;   //       0x15    //β探头(仅测alphy)
        case SENSER_B_AB: Elec_ABSenser();    break;   //      0x25    //β探头(可测alphy+beta)
        case SENSER_G1:   Elec_AaSenser();     break;   //        0x07    //地面探头1
        case SENSER_G2:   Elec_AaSenser();     break;   //        0x08    //地面探头2
        case SENSER_NONE: Elec_AaSenser();    break;   //   无外接探头
    }  
    #else
    if ((CheckRes.SenserType == SENSER_A_AB) ||
        (CheckRes.SenserType == SENSER_B_AB))
    {
        Elec_ABSenser();
    }
    else
    {
        Elec_AaSenser();
    }
    #endif

    //ElecTestTime --;
    //memset(StrTemp,0,24);
    //Show12X12String(0,82,"剩余时间",0);
    //sprintf(StrTemp,":%ds",ElecTestTime);
    //Show5X7String(54,85,StrTemp,0);
    //ShowSenserVer(24,82);
    Show12X12String(0,98,"任意键结束测试",0);
    
    
    ShowStatusBar();
    
    DisplayRefresh();
    
    //ADC12_Start();


    //if (ElecTestTime == 0)
    //{
    //    EndElecTest();
    //}

    //SaveMeteData();
}



void StartElecTest()
{
    currUI = ELECTEST;
    ElecTestTime = 60;
    ShowElecTestUI();
}

void ElecTestKey(PRESSKEY key)
{
    switch (key)
    {
        case TIMEOUT: ShowElecTestUI(); break;
        case MODE:    SwChannel();        break;

        // 任意键返回
        case POWER:    
        case OKKEY:    
        case UP:
        case DOWN:
        
        case RETURN:
            EndElecTest();
        break;
        
    }
}




#if 0
void Phy_PdSenser()
{

}

void Phy_GmSenser()
{

}

void Phy_AbSenser()
{

}

void Phy_AabSenser()
{

}

void Phy_BbSenser()
{

}

void Phy_BaSenser()
{

}


void Phy_BabSenser()
{

}

void Phy_G1Senser()
{

}

void Phy_G2Senser()
{

}
#endif

void Phy_AaSenser()
{
    //GetTestCps();
    ShowTestMode(0,11);

    // 2018.8.20  -- 外部NaI探头没有版本信息
    if (CheckRes.SenserType != SENSER_Y)
    {
        ShowSenserVer(52,14);
    }
    
    //ShowCounterMid(0,22,"",FaceInfo.OutA_Cps_Rt,UNIT_CPS);
    Show_InCounterMid(0,22,FaceInfo.OutA_Cps_Rt);
}


void Phy_ABYSenser()
{
    //GetTestCps();
    ShowTestMode(0,16);

    // 2018.8.20  -- 外部NaI探头没有版本信息
    if (CheckRes.SenserType != SENSER_Y)
    {
        ShowSenserVer(52,19);
    }
    
    ShowCounter(0,30,"",FaceInfo.OutA_Cps_Rt,UNIT_CPS);
}

void Phy_ABSenser()
{
    //GetTestCps();
    ShowTestMode(0,16);
    ShowSenserVer(52,19);

    ShowCounterSm(6,36,"@A",FaceInfo.OutA_Cps_Rt,UNIT_CPS);
    ShowCounterSm(0,48,"@B@Y",FaceInfo.OutB_Cps_Rt,UNIT_CPS);
}


static BYTE PhyTestIndex = 1;
static BOOL ParamEdit = FALSE;
void ShowPhyTestUI()
{
    //char str[16] = {0};
    GetTestCps();
    
    Clear_DispBuf();

    ShowMeteTitle();
    
    #if 0
    switch (CheckRes.SenserType)
    {
        //case SENSER_IN:   Elec_InSerser();     break;
        case SENSER_PD:   Phy_PdSenser();     break;   //         0x02    //水下高量程探头（PD）
        case SENSER_GM:   Phy_GmSenser();     break;  //         0x03    //水下中高量程探头（GM）
        case SENSER_A_A:  Phy_AaSenser();    break;    //          0x05        α探头(仅测alphy)
        case SENSER_A_B:  Phy_AbSenser();     break;  //        0x14    //α探头(仅测beta)
        case SENSER_A_AB: Phy_AabSenser();     break;  //      0x24    //α探头(可测alphy+beta)
        case SENSER_B_B:  Phy_BbSenser();    break;   //       0x05    //β探头(仅测beta)
        case SENSER_B_A:  Phy_BaSenser();    break;   //       0x15    //β探头(仅测alphy)
        case SENSER_B_AB: Phy_BabSenser();    break;   //      0x25    //β探头(可测alphy+beta)
        case SENSER_G1:   Phy_G1Senser();    break;   //        0x07    //地面探头1
        case SENSER_G2:   Phy_G2Senser();    break;   //        0x08    //地面探头2
        //case SENSER_NONE: Elec_InSerser();    break;   //   无外接探头
    }  
    #else
    
    #endif

    if (CheckRes.SenserType == SENSER_ABY)
    {
        Phy_ABYSenser();
        //ShowSenserVer(24,82);
        Show12X12String(30,98,"结束测试",PhyTestIndex==1);
    }
    else
    {
        if ((CheckRes.SenserType == SENSER_A_AB) ||
            (CheckRes.SenserType == SENSER_B_AB))
        {
            Phy_ABSenser();
        }
        else
        {
            Phy_AaSenser();
        }
        memset(StrTemp,0,24);
        sprintf(StrTemp,":%0.0fv",Senser.Param.Hv);
        Show12X12String(0,70,"探测器高压",PhyTestIndex==1);
        Show5X7String(66,73,StrTemp,0);

        memset(StrTemp,0,24);
        sprintf(StrTemp,":%0.0fmv",Senser.Param.Z1);
        Show12X12String(0,84,"甄别器阈值",PhyTestIndex==2);
        Show5X7String(66,87,StrTemp,0);
        
        Show12X12String(30,98,"结束测试",PhyTestIndex==3);
    }
    
    ShowStatusBar();
    
    DisplayRefresh();
    
    //ADC12_Start();

}



void UpdateParam()
{
    if (ParamEdit == TRUE)
    {
        ParamEdit = FALSE;
        
        if ((ParamTmp.Hv != Senser.Param.Hv) || 
            (ParamTmp.Z1 != Senser.Param.Z1))
        {
            SaveParam(StartPhyTest);
        }
    }
}

void StartPhyTest()
{
    UpdateParam();
    
    currUI = PHYTEST;
    PhyTestIndex = 1;

    
    ShowPhyTestUI();
}


void EndPhyTest()
{
    if (SetSenserWork(3,0))
    {      
        Senser.WorkSt = 3;
        ShowHint("物理测试完成",InterMainMenu);
    }
    else
    {
        ShowHint("物理测试失败",InterMainMenu);
    }
}

void SetHv()
{
    memcpy(&ParamTmp,&Senser.Param,sizeof(ParamTmp));
    ParamMax = Senser.ParamMax;
    ParamMin = Senser.ParamMin;
    Setting_SenserType = CheckRes.SenserType;
    InputHv(StartPhyTest,3);

    ParamEdit = TRUE;
}

void SetZ1()
{
    memcpy(&ParamTmp,&Senser.Param,sizeof(ParamTmp));
    ParamMax = Senser.ParamMax;
    ParamMin = Senser.ParamMin;
    Setting_SenserType = CheckRes.SenserType;
    InputZ1(StartPhyTest,4);

    ParamEdit = TRUE;
}



void PhyTestKey(PRESSKEY key)
{
    switch (key)
    {
        case TIMEOUT:  ShowPhyTestUI(); break;
        case POWER:    break;

        case UP:
        {
            if (--PhyTestIndex < 1)
            {
                PhyTestIndex = 3;
            }
            ShowPhyTestUI();
            break;
        }

        case DOWN:
        {
            if (++PhyTestIndex > 3)
            {
                PhyTestIndex = 1;
            }
            ShowPhyTestUI();
            break;

        }
        
        case OKKEY:    
        {
            if (CheckRes.CommInterface == COMM_CAN)
            {
                MCP2510_WakeUp();
            }
            
            if (CheckRes.SenserType == SENSER_ABY)
            {
                EndPhyTest();
            }
            else
            {
                switch (PhyTestIndex)
                {
                    case 1: SetHv(); break;
                    case 2: SetZ1(); break;
                    case 3: EndPhyTest(); break;
                }
            }
            break;
        }
        
    }
}


// 长杆维护界面
void ShowLpVersion()
{
    Show12X12String(0,12,"探头维护",0);
    ShowSenserVer(52,15);

    #if 0
    memset(StrTemp,0,24);
    sprintf(StrTemp,"(V%d)", CheckRes.SwVer);
    Show5X7String(52,15,StrTemp,0);     // 版本
    #endif
}

void ShowLpBat(BYTE y)
{
    Show12X12String(0,y,"电池电压: ",0);
    if (!BTGetBattary(1))
    {
        //CErr ++;
        Show12X12String(62,y,"异常",0);
    }
    else
    {
        //CErr = 0;
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%dmV-%d%%",
                         SenserBatery.Voltage,SenserBatery.percent);
        
        Show5X7String(62,y+3,StrTemp,0);
    }
}



#if 1
void LpDebugMode()
{
    static DWORD ErrCnt = 0;
    static BYTE  CErr = 0, AlmCnt = 0;    // 连续通信错误

    Clear_DispBuf();

    ShowMeteTitle();

    ShowLpVersion();

    #if 1
    Show12X12String(0,26,"电池电压: ",0);
    if (!BTGetBattary(1))
    {
        CErr ++;
        Show12X12String(62,26,"异常",0);
    }
    else
    {
        CErr = 0;
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%dmV-%d%%",
                         SenserBatery.Voltage,SenserBatery.percent);
        
        Show5X7String(62,29,StrTemp,0);
    }
    #endif


    if (!GetSenserCounter())
    {
        CErr ++;
        
        Show12X12String(0,40,"通信异常",0);
        
        memset(StrTemp,0,24);
        sprintf(StrTemp,"Err:%d", BTErrCode);
        Show5X7String(0,60,StrTemp,0);
        ErrCnt++;
    }
    else
    {
        CErr = 0;
        
        // 底量程usv/h
        Show5X7String(0,40,"L:",0);
        Show_InCounterSm(12,40,Senser.Conter.C1);
        // 底量程CPS
        Show5X7String(0,52,"L:",0);
        ShowCounterSm(12,52,"",Senser.Conter.P1,UNIT_CPS);
        
        // 高量程usv/h
        Show5X7String(0,64,"H:",0);
        Show_InCounterSm(12,64,Senser.Conter.C2);

        // 高量程CPS
        Show5X7String(0,76,"H:",0);
        ShowCounterSm(12,76,"",Senser.Conter.P2,UNIT_CPS); 
    }


    if (CErr >= 4)
    {
        AlmCnt ++;
        CErr = 0;
    }
    memset(StrTemp,0,24);
    sprintf(StrTemp,"ErrCnt:%ld AlmCnt:%d", ErrCnt,AlmCnt);
    Show5X7String(0,88,StrTemp,0);
    
    Show12X12String(0,116,"任意键结束测试",0);
    DisplayRefresh();

}
#endif


BYTE LpSrvFace = 0;
WORD ErrCnt = 0;


void Face_LpSrv()
{
    ShowLpBat(28);
        
    if (LpSrvFace == 0)
    {
        Show5X7String(30,44,"Low Channel",0);
        // 底量程usv/h
        //Show_InCounterMid(0,32,Senser.Conter.C1);
        // 底量程CPS
        ShowCounter(0,60,"",Senser.Conter.P1,UNIT_CPS);
    }
    else
    {
        Show5X7String(30,44,"High Channel",0);
        // 高量程usv/h
        //Show_InCounterMid(0,32,Senser.Conter.C2);
        // 高量程CPS
        ShowCounter(0,60,"",Senser.Conter.P2,UNIT_CPS); 
    }
}

void Face_LcSrv()
{
    ShowCounterMid(0,20," @A",Senser.Conter.P1,0);
    ShowCounterMid(0,60,"@B@Y",Senser.Conter.P2,UNIT_CPS); 
}


void LpNormalMode()
{
    #if 1
    BOOL ret = GetSenserCounter();
    #endif
    
    Clear_DispBuf();

    ShowMeteTitle();

    ShowLpVersion();

    #if 1
    //BTGetBattary(1);
    //if (!GetSenserCounter())
    if (!ret)
    {
        // 错误了不刷新
        Show12X12String(0,40,"通信异常",0);
        ErrCnt ++;
        StorageLog(ALARM_EVT_SENSER_ERR);
    }
    else
    #endif
    {

        switch(CheckRes.SenserType)
        {
        
            case SENSER_LC:   Face_LcSrv(); break;
            case SENSER_LP:   Face_LpSrv(); break; 
        }
    }

    memset(StrTemp,0,24);
    sprintf(StrTemp,"Err:%d",ErrCnt);
    Show5X7String(0,100,StrTemp,0);
    Show12X12String(0,116,"确认键结束测试",0);
    DisplayRefresh();
}

void SwLpSrvFace()
{
    if (LpSrvFace == 0)
    {
        LpSrvFace = 1;
    }
    else
    {
        LpSrvFace = 0;
    }
    BTGmSw(LpSrvFace);
    
    LpNormalMode();
}


void ShowBTSrvUI()
{
    
    #if 1
    if (DebugMode)
    {
        LpDebugMode();
    }
    else
    {
        LpNormalMode();
    }
    #else
    LpNormalMode();
    #endif

}

void EnterBTSenserSrv()
{
    currUI = BTSRV;
    ErrCnt = 0;
    ShowBTSrvUI();
}


void BTSenserKey(PRESSKEY key)
{
    switch (key)
    {
        case TIMEOUT:  ShowBTSrvUI(); break;

        case OKKEY:
        case RETURN:   
        {
            BTGmSw(0); 
            InterMainMenu(); 
            break;
        }

        case MODE:     SwLpSrvFace();     break;
    }
}


