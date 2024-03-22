#include "system.h"
#include <msp430x552x.h>
#include "RadioSet.h"
#include "string.h"
#include "oled.h"
#include <stdio.h>

#include "QuickMenu.h"
#include "MainMenu.h"
#include "Measinter.h"
#include "Senser.h"
#include "Input.h"
#include "Main.h"
#include "SelfCheck.h"
#include "SsAlarm.h"

//extern unsigned int TIMERCOUNT;
extern INTERFACE currUI;
extern QPara Param;
extern QInnerParam InnerParam;
extern SENSER Senser;
extern INPUTWORD InWord;
extern INPUTDWORD InDword;
extern INPUTFLOAT InFloat;
extern FLOATEX     FloatEx;
extern BYTE CanRecBuf[14][8];
extern CHECKRES CheckRes;
extern char * const UnitStr[];
extern char StrTemp[24];
extern BYTE OutAlarmType;

BYTE  CurRadIndex = 0;   // 正在使用核素在核素库中的索引
BYTE  RadCount = 0;      // 核素数量(除混合场外)
RADIO pnuc[25];      //核素


char * const NucName[]=
{
    "MIX",
    "Sr90",            // 1
    "Cs137",
    "Pu239",
    "Am241",
    "H3",              // 5
    "C14",
    "Na22",
    "P32",
    "Ti/Sc44",
    "Ca45",           // 10
    "Co58",
    "Co60",
    "Zn65",
    "Ge/a68",
    "Y90",           // 15
    "I131",
    "Au198",
    "Hg203",
    "Tl204",
    "Po210",          // 20
    "U235",
    "U238",
    "Pu238",
    "Cm244"
};

void RadInit()
{
    BYTE i;
    memset(&pnuc,0,sizeof(pnuc));
    
    pnuc[0].Valid = 1;  // 混合场必须有效
    pnuc[0].index = 0;  //MIX
    pnuc[0].Channel = CHANNEL_MIX;
    pnuc[0].AlarmThres = 50;
    pnuc[0].Efficiency = 0;
    pnuc[0].Unit = UNIT_CPS;


    for (i=1;i<25;i++)
    {
        pnuc[i].Unit = UNIT_Bq_cm2;
    }
}


// 获取当前核素的通道类型
BYTE GetCurChannel()
{
    BYTE i;
    for (i=1;i<=RadCount;i++)
    {
        if (Senser.Param.Hn == pnuc[i].index)
        {
            return pnuc[i].Channel;
        }
    }

    return 0xFF;
}

// 获取当前核素的索引
BYTE GetCurRadIndex()
{
    BYTE i;
    for (i=0;i<=RadCount;i++)
    {
        if (Senser.Param.Hn == pnuc[i].index)
        {
            return i;
        }
    }

    return 0xFF;
}




void ShowChannel(BYTE x, BYTE y, BYTE Channel)
{
    if(Channel == CHANNEL_A)
    {
        //显示 阿尔法
        char alfa[1] = {0};
        ShowSpecChar(x,y,alfa,0,1); 
    }
    else if(Channel == CHANNEL_B)
    {
        //显示贝塔&伽马
        char BeitaGama[2] = {1,2};
        ShowSpecChar(x,y,BeitaGama,0,2); 
    }
    else if(Channel == CHANNEL_MIX) // MIX
    {
        //显示 阿尔法 贝塔 伽马
        //char alfaBeitaGama[3] = {0,1,2};
        //ShowSpecChar(110,7,alfaBeitaGama,0,3);
        
        // 混合场根据探头显示通道
        if ((CheckRes.SenserType == SENSER_A_A) ||
            (CheckRes.SenserType == SENSER_B_A))
        {
            char ChannelStr[1] = {0};
            ShowSpecChar(x,y,ChannelStr,0,1); 
        }
        else if (  (CheckRes.SenserType == SENSER_A_AB) ||
                    (CheckRes.SenserType == SENSER_B_AB) ||
                    (CheckRes.SenserType == SENSER_ABY)  ||
                    (CheckRes.SenserType == SENSER_G1)   ||
                    (CheckRes.SenserType == SENSER_G2)  )
        {
            char ChannelStr[3] = {0,1,2};
            ShowSpecChar(x,y,ChannelStr,0,3); 
        }
        else
        {
            char ChannelStr[2] = {1,2};
            ShowSpecChar(x,y,ChannelStr,0,2); 
        }
    }
}

#define RADLINES  14    // 一屏显示14行核素
int RadSetSel = 0;
int PageNum = 0;
//BYTE SelectedRad = 0;
int CntRadio = 0;
int RadioStart = 0;


void ShowNucDetail(BYTE i)
{
    GT_Show16X16Char(0,4,"核素",0);
    GT_Show8X16Ascii(32,4,":",0);
    
    if (Senser.Param.Hn == pnuc[i].index)
    {   
        // 在当前核素前面显示一个*
        Show5X7String(40,7,"*",0);    
        GT_Show8X16Ascii(48,4,NucName[pnuc[i].index],0);    
    }
    else
    {
        GT_Show8X16Ascii(40,4,NucName[pnuc[i].index],0);    
    }
    
    ShowChannel(110,7,pnuc[i].Channel);
    
    Show12X12String(0,26,"当前效率",0);
    if(pnuc[i].Efficiency < 0.0001)
    {
        Show5X7String(70,29,": FF%",0);
    }
    else 
    {
        //char streffici[8] = "";
        memset(StrTemp,0,24);
        sprintf(StrTemp,": %.0f%%",pnuc[i].Efficiency*100);
        Show5X7String(70,29,StrTemp,0);        
    }
    
    ShowLine(44);
}

// line : 显示的行,  index : 核素索引
void ShowRad(BYTE line, BYTE index)
{
    // 显示核素名
    if (Senser.Param.Hn == pnuc[index].index)
    {   
        // 在当前核素前面显示一个*
        Show5X7String(0,13+(line%RADLINES)*8,"*",0);
        Show5X7String(6,13+(line%RADLINES)*8,NucName[pnuc[index].index],0);
    }
    else
    {
        Show5X7String(0,13+(line%RADLINES)*8,NucName[pnuc[index].index],0);
    }
    
    // 显示所属探头
    ShowChannel(48, 13+(line%RADLINES)*8, pnuc[index].Channel);


    
    // 显示阈值
    if ( ( (CheckRes.SenserType == SENSER_G1)    || 
            (CheckRes.SenserType == SENSER_G2)    ||
            (CheckRes.SenserType == SENSER_ABY)   ||
            (CheckRes.SenserType == SENSER_A_AB)  ||
            (CheckRes.SenserType == SENSER_B_AB)
          ) && (index == 0)   // 混合场
        )
    {
        Show5X7String(72,13+(line%RADLINES)*8,"...",0);
    }
    else
    {
        memset(StrTemp,0,24);
        if (pnuc[index].Unit == UNIT_CPS)
        {
            sprintf(StrTemp,"%0.0f%s",CpsUnit(pnuc[index].AlarmThres, 
                                                pnuc[index].Unit),
                                       UnitStr[pnuc[index].Unit]);    
        }
        else
        {
            sprintf(StrTemp,"%0.1f%s",CpsUnit(pnuc[index].AlarmThres, 
                                                pnuc[index].Unit),
                                       UnitStr[pnuc[index].Unit]);   
        }
        Show5X7String(72,13+(line%RADLINES)*8,StrTemp,0);
    }
    
}

void ShowRadSetUI()
{
    Clear_DispBuf();
    
    GT_Show12X12Char(0,0,"核素",0);
    GT_Show12X12Char(40,0,"通道",0);
    GT_Show12X12Char(76,0,"阈值",0);

    
    RefreshRadSet();
}


void InterRadSetMenu()
{
    if (!HaveRadSet())
    {
        ShowHint("该探头无核素",InterMainMenu);
        return;
    }
    
    currUI = RADLIST;
    ShowRadSetUI();
}


void RefreshRadSet()
{
    // 这里要根据探头显示对应的核素
    Clear(0,13,127,115);

    #if 0
    if ((CheckRes.SenserType == SENSER_G1) ||
         (CheckRes.SenserType == SENSER_G2))
    {
        CntRadio = 24;
        RadioStart = 1;
    }
    else if ((CheckRes.SenserType == SENSER_A_A) ||
         (CheckRes.SenserType == SENSER_B_A))
    {   
        // A核素
        CntRadio = 7;
        RadioStart = 18;
    }
    else
    {
        // BY核素
        CntRadio = 17;
        RadioStart = 1;
    }
    #else
    CntRadio = RadCount;
    RadioStart = 1;
    #endif

    
    if (PageNum == 0)
    {
        ShowRad(0,0);// 第一页要显示混合场
        for(int i = PageNum*RADLINES+1; i < ((PageNum+1)*RADLINES)&&(i <= CntRadio); i++)
        {
            ShowRad(i,RadioStart+i-1);
        }
    }
    else
    {
        for(int i = PageNum*RADLINES; i < ((PageNum+1)*RADLINES)&&(i <= CntRadio); i++)
        {
            ShowRad(i,RadioStart+i-1);
        }
    }
    
    RadSetMoveCursor();
}

void RadSetMoveCursor()
{
    const unsigned char Sign[] = {0x00,0x04,0x1C,0x7C,0x1C,0x04,0x00,0x00};
    Clear(120,13,8,115);
    Diplay(120,13+RadSetSel*8,8,8,Sign,0);
    DisplayRefresh();
}

void RadSetKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter();        break;
        case RETURN:  InterMainMenu();          break;
        case OKKEY:   InterRadDetailSetMenu(); break;
        
        //case MODE: ; break;
              
        case DOWN:
        {
           
            if(PageNum*RADLINES+RadSetSel < CntRadio)
            {
                if(++RadSetSel >= RADLINES)
                {
                    RadSetSel = 0;
                    PageNum++;
                    RefreshRadSet();
                }
                RadSetMoveCursor();
            }
            else
            {
                PageNum = 0;
                RadSetSel = 0;
                RefreshRadSet();
                RadSetMoveCursor();
            }
           
        }
        break;
        
        case UP:
        {
            
            if(--RadSetSel < 0)
            {
                if(PageNum > 0)
                {
                    RadSetSel = RADLINES-1;
                    PageNum--;    
                }
                else if(PageNum <= 0)
                {
                    PageNum = CntRadio/RADLINES;
                    RadSetSel = CntRadio%RADLINES;
                }
				RefreshRadSet();
            }
            RadSetMoveCursor();   
        
        }
        break;
    }
}




// 选择核素单位
char RadUnitIndex = 1;
void RefreshRadUnit()
{
    BYTE i;
    PopWindow(72,54,44,9*3+4,"");
    for (i=0;i<3;i++)
    {
        Show5X7String(78,58+i*9,UnitStr[i+1], RadUnitIndex == i+1);
    }
    DisplayRefresh();
}

void SelectRadUnit()
{
    currUI = SELRADUNIT;
    RadUnitIndex = pnuc[InnerParam.RadioSelIndex].Unit;
    RefreshRadUnit();
}

void RadUnitKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterRadDetailSetMenu();       break;

        case UP:
        {
            if (--RadUnitIndex < 1)
            {
                RadUnitIndex = 3;
            }
            RefreshRadUnit();
        }
        break;
        
        case DOWN:
        {
            if (++RadUnitIndex > 3)
            {
                RadUnitIndex = 1;
            }
            RefreshRadUnit();
        }
        break;
        
        case OKKEY:
        {
            pnuc[InnerParam.RadioSelIndex].Unit = RadUnitIndex;
            SetRadUnit(RadUnitIndex);
            InterRadDetailSetMenu();
        }
        break;
        
    }
}






//核素详细设置界面
char RadDetailIndex = 1;
char IsUseThisRad = 1;
int SelAlarmThres = 0;     
float AlarmThresTemp = 0.0;  // 报警阈值
float RadActiveTemp = 0.0;

void InterRadDetailSetMenu()
{
    BYTE sel = PageNum*RADLINES+RadSetSel;

    if (sel == 0)
    {
        InnerParam.RadioSelIndex = 0;
    }
    else
    {
        InnerParam.RadioSelIndex = RadioStart + sel -1;
    }
    
    currUI = DETASET;
    Clear_DispBuf();

    //GetSenserXl(InnerParam.RadioSelIndex);
    ShowNucDetail(InnerParam.RadioSelIndex);

    GT_Show12X12Char(0,50,"剂量单位",    RadDetailIndex == 1);
    GT_Show12X12Char(0,66,"报警阈值",    RadDetailIndex == 2);
    GT_Show12X12Char(0,82,"活度响应",    RadDetailIndex == 3);
    GT_Show12X12Char(0,98,"效率重新标定",RadDetailIndex == 4);
    GT_Show12X12Char(0,114,"启用该核素",  RadDetailIndex == 5);


    //显示单位
    Show5X7String(70,53,":",0);
    Show5X7String(82,53,UnitStr[pnuc[InnerParam.RadioSelIndex].Unit],0);

    // 显示阈值
    // G1/G2探头的混合场阈值在报警参数中设置
    if (OneThres())
    {
        memset(StrTemp,0,24);
        AlarmThresTemp = CpsUnit(pnuc[InnerParam.RadioSelIndex].AlarmThres, 
                                    pnuc[InnerParam.RadioSelIndex].Unit);
        if (AlarmThresTemp < 0.0)
        {
            AlarmThresTemp = 0.0;
        }

        if (pnuc[InnerParam.RadioSelIndex].Unit == UNIT_CPS)
        {
            sprintf(StrTemp,": %0.0f",AlarmThresTemp);
        }
        else
        {
            sprintf(StrTemp,": %0.2f",AlarmThresTemp);
        }                   
        Show5X7String(70,69,StrTemp,0);  

        //活度响应
        RadActiveTemp = pnuc[InnerParam.RadioSelIndex].Active;
        sprintf(StrTemp,": %0.2f",RadActiveTemp);
        Show5X7String(70,85,StrTemp,0);  
    }
    else
    {
        // 混合场
        Show5X7String(70,69,": ...",0);  
        Show5X7String(70,85,": ...",0);  
    }
    
    ShowOptHint();
    DisplayRefresh();
}



void InputRadAlarm()
{
    InFloat.Title = "报警阈值";
    
    InFloat.Spec  = UnitStr[pnuc[InnerParam.RadioSelIndex].Unit];
    
    InFloat.Max = 300;
    InFloat.Min = 1;
    InFloat.Step = 1;
    InFloat.Rf = InterRadDetailSetMenu;
    InFloat.Val = &AlarmThresTemp;

    InputFloat();
}

void UpdateRadActive()
{
    if (!SetRadActive(RadActiveTemp))
    {
        ShowHint("活度响应设置失败", InterRadDetailSetMenu);
    }
    else
    {
        pnuc[InnerParam.RadioSelIndex].Active = RadActiveTemp;
        
        SaveSenserParam();
        
        //AyncAlarmThres(Thres);
        //memcpy(&Senser.Alarm,&AlarmTmp,sizeof(AlarmTmp));
        //Senser.
                    
        //ShowHint("报警设置成功", InterRadDetailSetMenu);
        StorageLog(EVENT_MOD_OUTALARM);
        InterRadDetailSetMenu();
    }
}

void InputRadActive()
{
    if (InnerParam.RadioSelIndex == 0)  // 混合场
    {
        ShowHint("混合场无此参数",InterRadDetailSetMenu);
        return;
    }
    
    FloatEx.Bp = 3;
    FloatEx.Bn = 2;
    FloatEx.Max = 1000;
    FloatEx.Min = 0;
    FloatEx.Val = &RadActiveTemp;
    FloatEx.x = 60;
    FloatEx.y = 80;
    FloatEx.Rf = UpdateRadActive;
    InputFloatEx();
}

void AyncAlarmThres(float Thres)
{
    // 启用新的参数
    if (pnuc[InnerParam.RadioSelIndex].index == Senser.Param.Hn)
    {
        if (GetCurChannel() == CHANNEL_A)
        {
            Senser.Alarm.Ac = Thres;
        }
        else
        {
            Senser.Alarm.Bc = Thres;
        }
    }
}




void UpdateAlarmThres()
{
    #if 0
    static float Thres;
    
    Thres = ToCps(AlarmThresTemp, pnuc[InnerParam.RadioSelIndex].Unit);

    
    //  如果当前核素为正在使用的核素，需要将
    //  阈值下发到探头
    if (InnerParam.RadioSelIndex == Senser.Param.Hn)
    {
        //  将参数下发到探测器
        if (!SetAlarmThres(Thres))
        {
            ShowHint("报警设置失败", InterRadDetailSetMenu);
        }
        else
        {
            pnuc[InnerParam.RadioSelIndex].AlarmThres = Thres;
            
            SaveSenserParam();
            // 启用新的参数
            //memcpy(&Senser.Alarm,&AlarmTmp,sizeof(AlarmTmp));
            //Senser.
                        
            //ShowHint("报警设置成功", InterRadDetailSetMenu);
            StorageLog(EVENT_MOD_OUTALARM);
            InterRadDetailSetMenu();
        }
    }
    else
    {
        pnuc[InnerParam.RadioSelIndex].AlarmThres = Thres;
        InterRadDetailSetMenu();
    }
    #else
    //  将参数下发到探测器
    static float Thres;
    Thres = ToCps(AlarmThresTemp, pnuc[InnerParam.RadioSelIndex].Unit);
    if (!SetAlarmThres(Thres))
    {
        ShowHint("报警阈值设置失败", InterRadDetailSetMenu);
    }
    else
    {
        pnuc[InnerParam.RadioSelIndex].AlarmThres = Thres;
        
        SaveSenserParam();
        
        AyncAlarmThres(Thres);
        //memcpy(&Senser.Alarm,&AlarmTmp,sizeof(AlarmTmp));
        //Senser.
                    
        //ShowHint("报警设置成功", InterRadDetailSetMenu);
        StorageLog(EVENT_MOD_OUTALARM);
        InterRadDetailSetMenu();
    }
    #endif
}

void InputRadAlarmEx()
{
    if (pnuc[InnerParam.RadioSelIndex].Unit == UNIT_CPS)
    {
        FloatEx.Bp = 3;
        FloatEx.Bn = 0;
        FloatEx.Max = 3000;
    }
    else if (pnuc[InnerParam.RadioSelIndex].Unit == UNIT_Bq)
    {
        FloatEx.Bp = 3;
        FloatEx.Bn = 0;
        FloatEx.Max = 3000;
    }
    else  // Bq_cm2
    {
        FloatEx.Bp = 3;
        FloatEx.Bn = 2;
        FloatEx.Max = 300;
    }
    FloatEx.Min = 0;
    FloatEx.Val = &AlarmThresTemp;
    FloatEx.x = 60;
    FloatEx.y = 64;
    FloatEx.Rf = UpdateAlarmThres;
    InputFloatEx();
}

void RadDetailSetKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN: 
        {
            //if (NeedSaveParam())
            if (0) // 这里不需要更新核素效率
            {
                PopHint("正在启用新参数");
                WriteParamToFlash();
                
                ShowHint("参数启用成功", InterRadSetMenu);
            }
            else
            {
                InterRadSetMenu();        
            }
            break;
        }
        
        case OKKEY:
        {
            switch (RadDetailIndex)
            {
                case 1:  
                {
                    // 新的需求不允许修改单位
                    #if 1   // 2023.10.8 重新打开，客户又要求能修改单位
                    // 混合场不能修改单位
                    if (InnerParam.RadioSelIndex == 0)
                    {
                        ShowHint("混合场不能修改剂量单位",InterRadDetailSetMenu);
                    }
                    else
                    {
                        SelectRadUnit();       
                    }
                    #endif
                    break;
                }
                
                case 2:  
                {
                    // G1/G2探头没有阈值
                    if (OneThres())
                    {
                        InputRadAlarmEx();    
                    }
                    else
                    {
                        // 混合场
                        //ShowHint("该核素无此操作",InterRadDetailSetMenu);
                        StartSetOutAlarm(MIX);
                    }
                    break;
                }
                case 3:  InputRadActive();       break;
                case 4:  InterEffiDemarMenu(); break;
                case 5:  UseThisRad();          break;
            }
        }
        break;
        
        case DOWN:
        {
            if(++RadDetailIndex > 5)
            {
                RadDetailIndex = 1;
            }
            InterRadDetailSetMenu();
           
        }
        break;
        
        case UP:
        {
            if(--RadDetailIndex < 1)
            {
                RadDetailIndex = 5;
            }
            InterRadDetailSetMenu();
           
        }
        break;
    }
}


#if 0
void RadDetailUpdateSel()
{
    Clear(0,13,54,13);
    Clear(0,29,95,13);
    
    GT_Show12X12Char(0,13,"报警阈值",RadDetailIndex == 1);
    GT_Show12X12Char(0,29,"启用该核素测量",0);
    Show5X7String(96,29,"[   ]",0);   
    
    Clear(104,27,16,16);
    if(IsUseThisRad == 0)
    {
        GT_Show12X12Char(104,27,"否",RadDetailIndex == 2);
        Param.RadioSelIndex = 0;
    }
    else if(IsUseThisRad == 1)
    {
        GT_Show12X12Char(104,27,"是",RadDetailIndex == 2);
        Param.RadioSelIndex = PageNum*6+RadSetSel;
    }
    
    Clear(55,15,45,8);
    char strAlarmThres[8] = "";
    sprintf(strAlarmThres,"%d Bq",SelAlarmThres);
    Show5X7String(55,15,strAlarmThres,0);

    
    char streffici[8] = "";
    if(pnuc[PageNum*6+RadSetSel].Efficiency == 0)
    {
        Clear(127-strlen("FF.FF%")*6,2,strlen("FF.FF%")*6,8);
        Show5X7String(127-strlen("FF.FF%")*6,2,"FF.FF%",0);
    }
    else
    {
        sprintf(streffici,"%0.2f %%",pnuc[PageNum*6+RadSetSel].Efficiency);
        Clear(127-strlen(streffici)*6,2,strlen(streffici)*6,8);
        Show5X7String(127-strlen(streffici)*6,2,streffici,0);        
    }
    
    Clear(0,47,56,13);
    GT_Show12X12Char(0,47,"效率标定",RadDetailIndex == 3);
    Clear(100,47,28,17);
    GT_Show12X12Char(100,47,"确定",RadDetailIndex == 4);
    
    DisplayRefresh();
}
#endif




//效率标定界面  effidemar
WORD DemarTimeOut = 0;     // 剩余时间
float uiSourcActiv = 5000; // 活度
float uiDemarTime = 30;    // 标定时间
float ucEffi = 0.0;

char EffiDemarIndex = 1;
void InterEffiDemarMenu()
{    
    currUI = EFFIDEMAR;
    
    if(InnerParam.RadioSelIndex == 0)
    {
        ShowHint("混合场不能标定",InterRadDetailSetMenu);
        return;
    }

    RefreshEffiDemar();
}

void RefreshEffiDemar()
{
    Clear_DispBuf();
    ShowNucDetail(InnerParam.RadioSelIndex);
    
    //GT_Show12X12Char(0,82,"表面活度响应",EffiDemarIndex == 3);
    GT_Show12X12Char(0,50,"表面发射率",EffiDemarIndex == 1);
    GT_Show12X12Char(0,66,"标定时长",EffiDemarIndex == 2);
    GT_Show12X12Char(0,82,"开始标定",EffiDemarIndex == 3);
    
    //char str[12] = "";
    memset(StrTemp,0,24);
    sprintf(StrTemp,": %0.0fcps",uiSourcActiv);
            //UnitStr[pnuc[Param.RadioSelIndex].Unit]);
    Show5X7String(70,53,StrTemp,0);
    
    
    memset(StrTemp,0,24);
    sprintf(StrTemp,": %0.0fs",uiDemarTime);
    Show5X7String(70,69,StrTemp,0);

    ShowOptHint();
    DisplayRefresh();
}

void InputSrcActive()
{
    #if 0
    InFloat.Title = "标定源活度";
    InFloat.Spec  = "cps"; //UnitStr[pnuc[Param.RadioSelIndex].Unit];
    InFloat.Bits  = 0;
    InFloat.Max   = 50000;
    InFloat.Min   = 0;
    InFloat.Step  = 100;
    InFloat.Rf    = InterEffiDemarMenu;
    InFloat.Val   = &uiSourcActiv;
    InputFloat();
    #else
    FloatEx.Bp = 6;
    FloatEx.Bn = 0;
    FloatEx.Max = 99999;
    FloatEx.Min = 1;
    FloatEx.Val = &uiSourcActiv;
    FloatEx.x = 70;
    FloatEx.y = 53;
    
    FloatEx.Rf = InterEffiDemarMenu;
    InputFloatEx();
    #endif
}

void InputDemarTime()
{
    InFloat.Title = "标定时长";
    InFloat.Spec  = "s";
    InFloat.Bits  = 0;
    InFloat.Max   = 120;
    InFloat.Min   = 10;
    InFloat.Step  = 10;
    InFloat.Rf    = InterEffiDemarMenu;
    InFloat.Val   = &uiDemarTime;
    InputFloat();
}

void EffiDemarKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter();        break;
        case RETURN:  InterRadDetailSetMenu();  break;
        
        case OKKEY:
        {
            switch (EffiDemarIndex)
            {
                case 1: InputSrcActive(); break;
                case 2: InputDemarTime(); break;
                case 3: BeginDemar();      break;
            }
        }
        break;
        
        case DOWN:
        {
            if(++EffiDemarIndex > 3)
            {
                EffiDemarIndex = 1;
            }
            RefreshEffiDemar();
        }
        break;
        
        case UP:
        {
            if(--EffiDemarIndex < 1)
            {
                EffiDemarIndex = 3;
            }
            RefreshEffiDemar();
            
        }
        break;
    }
}


// 启动效率标定
BOOL StartDemar(BYTE id)
{
    BOOL ret;

    // 1 . 启动核素号
    if (!SetSenserNuc(id))
    {
        return FALSE;

    }

    //pnuc[id].AlarmThres = ;
    //pnuc[id].Channel = ;
    // 2.  下发效率标定参数
    ret = SenserSendFloat(SCMD_XL_FIX, 1, uiDemarTime, 0);
    if (!ret)
    {
        return FALSE;
    }

    // 3.  下发效率标定活度
    ret = SenserSendFloat(SCMD_XL_FIX, 2, uiSourcActiv, 0);
    if (!ret)
    {
        return FALSE;
    }

    // 这个不返回
    // 标定通道0x01表示α，0x02表示β
    SenserSendFloat(SCMD_XL_FIX, 3, (float)pnuc[id].Channel, 0);  
    
    return TRUE;
}



//unsigned int uiMeterTime = 0;
//unsigned long SumCps = 0;
//标定界面
//char DemarUIIndex = 1;


void RefreshDemarUI()
{
    //char str[10] = {0};
    memset(StrTemp,0,24);
    Clear_DispBuf();
    ShowNucDetail(InnerParam.RadioSelIndex);
    
    
    Show12X12String(0,60,"正在重新标定核素效率",0);
    Show12X12String(0,76,"请稍候...",0);
    GT_Show12X12Char(0,92,"剩余时间",0);
    
    sprintf(StrTemp,": %ds",DemarTimeOut);
    Show5X7String(54,95,StrTemp,0);
    
    DisplayRefresh();
}


void Demaring()
{
    static WORD Cnt = 0;

    if (CanWaitCmd(SCMD_XL_FIX))
    {
        ucEffi = CanDataToFloat(&CanRecBuf[0][0]);
        
        
        InterDemerResUI();
        return;
    }
    else
    {
        Cnt ++;
        if (Cnt%4 == 0)   // 4*300ms == 1.2s  有意加长的
        {
            if (DemarTimeOut-- == 0)
            {
                ShowHint("效率标定失败",InterRadDetailSetMenu);
                return;
            }

            RefreshDemarUI();               
        }
    }
}





void BeginDemar()
{
    currUI = DEMARING;

    ucEffi = 0.0;    
    DemarTimeOut = (WORD)uiDemarTime;

    RefreshDemarUI();
    
    if (!StartDemar(pnuc[InnerParam.RadioSelIndex].index))
    {
        ShowHint("启动标定失败", InterEffiDemarMenu);
    }
}

void DemaringKey(PRESSKEY key)
{
    Demaring();
}
    


//extern unsigned long Alfacps;
//extern unsigned long Bertacps;
//char chTest = 1;

#if 0
void DemarUIKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: RefreshDemarUI(); break;

        // 其他按键不相应
        // 不能中断标定过程
    }
}
#endif


void ShowDemerRes()
{
    //char str[10] = {0};
    memset(StrTemp,0,24);
    Clear_DispBuf();

    ShowNucDetail(InnerParam.RadioSelIndex);
   
    if (ucEffi == 0.0)
    {
        Show12X12String(0,60,"效率标定失败",0);
        Show12X12String(0,76,"效率值",0);

        if (ucEffi < 0.0001)
        {
            sprintf(StrTemp,": FF%%");
        }
        else
        {
            sprintf(StrTemp,": %.0f%%", ucEffi*100);
        }
        Show12X12String(42,76,StrTemp,0);

        Show12X12String(50,112,"返回",1);
    }
    else
    {
        Show12X12String(0,60,"效率标定成功",0);
        Show12X12String(0,76,"效率值",0);

        if (ucEffi < 0.0001)
        {
            sprintf(StrTemp,": FF%%");
        }
        else
        {
            sprintf(StrTemp,": %.0f%%", ucEffi*100);
        }
        Show12X12String(42,76,StrTemp,0);

        Show12X12String(50,112,"保存",1);
    }
    DisplayRefresh();
}


void InterDemerResUI()
{
    currUI = DEMARRES;
    ShowDemerRes();
}


void SetOldRad()
{
    if (pnuc[InnerParam.RadioSelIndex].index != Senser.Param.Hn)
    {
        // 将探头的核素重新设置为原来的。
        SetSenserNuc(Senser.Param.Hn);
    }
}

void DemerResKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: 
        case POWER: 
        {
            SetOldRad();
            EnterMeasuInter(); 
            break;
        }
        
        case RETURN:  
        {
            SetOldRad();
            InterRadDetailSetMenu(); 
            break;
        }
        
        case OKKEY:   
        {
            if (ucEffi == 0.0)
            {
                InterRadDetailSetMenu();
            }
            else
            {
                //if (SenserXlFix(4,ucEffi))
                if (SetPadXl(pnuc[InnerParam.RadioSelIndex].index, ucEffi))
                {
                    pnuc[InnerParam.RadioSelIndex].Efficiency = ucEffi;
                    
                    ShowHint("保存成功",InterRadDetailSetMenu);
                }
                else
                {
                    ShowHint("保存失败",InterRadDetailSetMenu);
                }
            }

            SetOldRad();
        }
        break;
    }
}


void UseThisRad()
{
    if (Senser.Param.Hn == InnerParam.RadioSelIndex)
    {
        ShowHint("核素已经启用",InterRadSetMenu);
        return;
    }

    PopHint("正在启用核素");

    
    if (InnerParam.RadioSelIndex != 0)
    {
        if (pnuc[InnerParam.RadioSelIndex].Active == 0.0)
        {
            // 没有标定的核素不能启用
            if (pnuc[InnerParam.RadioSelIndex].Efficiency == 0.0)
            {
                ShowHint("未标定的核素不能启用",InterRadDetailSetMenu);
                return;
            }
        }
    }

    if (pnuc[InnerParam.RadioSelIndex].AlarmThres == 0.0)
    {
        ShowHint("报警参数错误",InterRadDetailSetMenu);
        return;
    }
    
    if (!SetSenserNuc(pnuc[InnerParam.RadioSelIndex].index))
    {
        ShowHint("核素启用失败",InterRadDetailSetMenu);
        return;
    }

    #if 0   // ---- 新的模式不用下发了
    // 下发报警阈值
    // G1/G2 的混合场在这里不下发
    if (OneThres())
    {
        if (!SetAlarmThres(pnuc[InnerParam.RadioSelIndex].AlarmThres))
        {
            ShowHint("报警设置失败", InterRadDetailSetMenu);
            return;
        }
    }
    #endif

    if (!SaveSenserParam())
    {
        ShowHint("核素保存失败",InterRadDetailSetMenu);
        return;
    }
    
    Senser.Param.Hn = pnuc[InnerParam.RadioSelIndex].index;
    CurRadIndex = InnerParam.RadioSelIndex;

    AyncAlarmThres(pnuc[InnerParam.RadioSelIndex].AlarmThres);
    
    WriteParamToFlash();

    ClearMax();
    ShowHint("核素启用成功",InterRadSetMenu);
    
}

#if 0
void ShowEffiDemar()
{
    if(++chTest <= 3)
    {
        return;
    }
    chTest = 5;
    if(++uiMeterTime == (uiDemarTime+1))
    {
      const unsigned char PerSig[] = {
        0x00,0x00,0x00,0x00,0x18,0x10,0x24,0x20,0x42,0x20,0x42,0x60,0x42,0x40,0x42,0xC0,
	    0x42,0x80,0x42,0x80,0x25,0x00,0x1D,0x30,0x03,0x48,0x02,0x84,0x02,0x84,0x04,0x84,
	    0x04,0x84,0x08,0x84,0x08,0x84,0x18,0x48,0x10,0x38,0x00,0x00,0x00,0x00,0x00,0x00};
        clrbit(TIMERCOUNT,6);
        
        memset((void *)DispBuf, 0, sizeof(DispBuf));
        GT_Show12X12Char(0,0,"标定结果",0);
        Show16X24float(33,16,ucEffi,0);
        Diplay(90,16,16,24,PerSig,0);
        Show5X7String(10,44,"OK",0);
        GT_Show12X12Char(26,42,"保存",0);
        Show5X7String(74,44,"M",0);
        GT_Show12X12Char(82,42,"不保存",0);
        
        
        uiMeterTime = 0;
        chTest = 2;
        //InterEffiDemarMenu();
        DisplayRefresh();
        return;
    }
    
    char strCurrDemar[8] = {0};
    char strCurrEffi[4] = {0};
    if(pnuc[PageNum*6+RadSetSel].Channel == 0)
    {
        SumCps += Alfacps;
        Alfacps = SumCps/uiMeterTime;
        ucEffi = (float)Alfacps*(float)100/(float)uiSourcActiv;
        sprintf(strCurrDemar,"%lu",Alfacps);
    }
    else if(pnuc[PageNum*6+RadSetSel].Channel == 1)
    {
        SumCps += Bertacps;
        Bertacps = SumCps/uiMeterTime;
        ucEffi = (float)Bertacps*(float)100/(float)uiSourcActiv;
        sprintf(strCurrDemar,"%lu",Bertacps);
    }
    
    Clear(30,12,60,8);
    Show5X7String(30,12,strCurrDemar,0);
    DisplayRefresh();
    
    Clear(55,25,60,13);
    sprintf(strCurrEffi,":  %0.2f %%",ucEffi);
    Show5X7String(55,25,strCurrEffi,0);
    
    char strMeterTime[4] = "";
    sprintf(strMeterTime,":  %d",uiDemarTime-uiMeterTime);
    Clear(55,39,60,13);
    Show5X7String(55,39,strMeterTime,0);
    DisplayRefresh();
}





void InterUseRadMenu()
{
    currUI = RADISTEST;
    memset((void *)DispBuf, 0, sizeof(DispBuf));
    GT_Show12X12Char(0,24,"是否启用该核素",0);
    Show5X7String(96,26,"[   ]",0); 
    RefreshUseRad();
}

void RefreshUseRad()
{
    Clear(104,24,16,16);
    if(IsUseThisRad == 0)
    {
        GT_Show12X12Char(104,24,"否",0);
        Param.RadioSelIndex = 0;
    }
    else if(IsUseThisRad == 1)
    {
        GT_Show12X12Char(104,24,"是",0);
        Param.RadioSelIndex = PageNum*6+RadSetSel;
    }
    
    DisplayRefresh();
}

void UseRadKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: EnterMeasuInter(); break;
        
        case MODE:
        {
            RadDetailIndex = 1;
            InterRadDetailSetMenu();
        }
        break;
        case OKKEY:
        {
            if(IsUseThisRad == 1)
            {
                Param.RadioSelIndex = PageNum*6 + RadSetSel;
            }
            memset((void *)DispBuf, 0, sizeof(DispBuf));
            GT_Show12X12Char(38,26,"修改成功",0);
            DisplayRefresh();
            delay_ms(1000);
            RadDetailIndex = 1;
            InterRadDetailSetMenu();
        }
        break;
        case UP:
        {
            IsUseThisRad = IsUseThisRad == 1 ? 0 : 1;
            RefreshUseRad();
        }
        break;
        case DOWN:
        {
            IsUseThisRad = IsUseThisRad == 1 ? 0 : 1;
            RefreshUseRad();
        }
        break;
    }
}
#endif


