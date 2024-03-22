#include "system.h"
#include "CommDataDefine.h"

#include "Keyboard.h"
#include <msp430x552x.h>
#include "oled.h"

#include "OtherParam.h"
#include <stdio.h>
#include <string.h>
#include "MainMenu.h"
#include "ds1337.h"
#include "mcp4725.h"
#include "hvadc.h"
#include "Measinter.h"
#include "Input.h"
#include "DataStore.h"
#include "Main.h"
#include "Eeprom.h"
#include "SelfCheck.h"
#include "BlueTooth.h"

extern INPUTWORD InWord;
extern INPUTBYTE InByte;
extern INTERFACE currUI;
extern char IsSuperUser;
extern QPara Param;
//extern char chTimeMeasur;
extern CHECKRES CheckRes;
extern char StrTemp[24];
extern QInnerParam InnerParam;
extern DS1337_QTIME cuTime;
extern BOOL DebugMode;

//参数设置
char SysParamCount = 5;
char ParamSetIndex = 1;


void UseDefParam()
{
    if (IsSuperUser == 0)
    {
        ShowHint("没有权限",InterParamSetUI);
        return;
    }
        
    PopHint("正在恢复默认参数...");
    //StorageLog(EVENT_DEF_PARAM);

    // 不能删除数据记录的数量
    //Eeprom_WriteByte(PARAM_OFFSET,0xFF); 
    
    
    
    Param_Def();

    
    
    WriteParamToFlash();
    Sleep(1000);
    PopHint("操作成功, 正在关机...");
    Sleep(3000);
    PowerOff();
    //ShowHint("操作成功, ",PowerOff);
    //EnterMeasuInter();
}


void InputMenuTimeOut()
{
    InByte.Title = "菜单超时时间";
    InByte.Spec  = "S";
    InByte.Max = 60;
    InByte.Min = 0;
    InByte.Step = 5;
    InByte.Rf = InterParamSetUI;
    InByte.Val = &InnerParam.MeauExitTime;
    InputByte();
}


void InputBkTimeOut()
{
    InByte.Title = "背光点亮时间";
    InByte.Spec  = "S";
    InByte.Max = 60;
    InByte.Min = 0;
    InByte.Step = 5;
    InByte.Rf = InterParamSetUI;
    InByte.Val = &InnerParam.BkTimeout;
    InputByte();
}


void SwInnerSensor()
{
    InnerParam.InnerSersor = !InnerParam.InnerSersor;  
    ShowHint("请重启主机, 使此参数生效! ", ShowParamSetUI);
}


void ParamSetUI_Normal()
{
    //if (ParamSetIndex >= 5)
    //{
    //    GT_Show12X12Char(20,32,"蓝牙参数",ParamSetIndex == 5);
    //}
    //else
    {
        GT_Show12X12Char(20,32,"报警音",0);
        Show5X7String(58,35,"[   ]",0);
        if(InnerParam.Sound)
        {
            GT_Show12X12Char(66,32,"开",ParamSetIndex == 1);
        }
        else 
        {
            GT_Show12X12Char(66,32,"关",ParamSetIndex == 1);
        }

        
        GT_Show12X12Char(20,48,"按键音",0);
        Show5X7String(58,51,"[   ]",0);
        if(InnerParam.Keytone)
        {
            GT_Show12X12Char(66,48,"开",ParamSetIndex == 2);
        }
        else 
        {
            GT_Show12X12Char(66,48,"关",ParamSetIndex == 2);
        }
        
        GT_Show12X12Char(20,64,"修改密码",ParamSetIndex == 3);
        GT_Show12X12Char(20,80,"时间设置",ParamSetIndex == 4);

        GT_Show12X12Char(20,96,"蓝牙参数",ParamSetIndex == 5);

        //Show5X7String(20,96,"...",0);   
    }
}

void ParamSetUI_Admin()
{
    if (ParamSetIndex >= 6)
    {
        //GT_Show12X12Char(20,32,"蓝牙参数",ParamSetIndex == 5);
        GT_Show12X12Char(20,32,"恢复默认参数",ParamSetIndex == 6);
        GT_Show12X12Char(20,48,"设备类型选择",ParamSetIndex == 7);
        
        GT_Show12X12Char(20,64,"调试模式",0);
        Show5X7String(74,67,"[   ]",0);
        if(DebugMode)
        {
            GT_Show12X12Char(82,64,"开",ParamSetIndex == 8);
        }
        else 
        {
            GT_Show12X12Char(82,64,"关",ParamSetIndex == 8);
        }

        GT_Show12X12Char(20,80,"内部探头",0);
        Show5X7String(74,83,"[   ]",0);
        if (InnerParam.InnerSersor)
        {
            GT_Show12X12Char(82,80,"有",ParamSetIndex == 9);
        }
        else
        {
            GT_Show12X12Char(82,80,"无",ParamSetIndex == 9);
        }


        GT_Show12X12Char(20,96,"设备制造商",0);
        memset(StrTemp,0,24);
        sprintf(StrTemp,"[%d]", InnerParam.ComLog);
        Show5X7String(86,99,StrTemp,ParamSetIndex == 10);
        
    }
    else
    {
        GT_Show12X12Char(20,32,"报警音",0);
        Show5X7String(58,35,"[   ]",0);
        if(InnerParam.Sound)
        {
            GT_Show12X12Char(66,32,"开",ParamSetIndex == 1);
        }
        else 
        {
            GT_Show12X12Char(66,32,"关",ParamSetIndex == 1);
        }

        
        GT_Show12X12Char(20,48,"按键音",0);
        Show5X7String(58,51,"[   ]",0);
        if(InnerParam.Keytone)
        {
            GT_Show12X12Char(66,48,"开",ParamSetIndex == 2);
        }
        else 
        {
            GT_Show12X12Char(66,48,"关",ParamSetIndex == 2);
        }
        
        GT_Show12X12Char(20,64,"修改密码",ParamSetIndex == 3);
        GT_Show12X12Char(20,80,"时间设置",ParamSetIndex == 4);
        GT_Show12X12Char(20,96,"蓝牙参数",ParamSetIndex == 5);
        Show5X7String(20,112,"...",0);
    }
}


void ShowParamSetUI()
{
    currUI = PARAMSET;
    
    Clear_DispBuf();
    ShowMeteTitle();
    
    GT_Show16X16Char(32,12,"系统参数",0);

    if(IsSuperUser == 1)
    {
        ParamSetUI_Admin();
    }
    else
    {
        ParamSetUI_Normal();
    }
    
    ShowOptHint();
    DisplayRefresh();
}


void InterParamSetUI()
{
    currUI = PARAMSET;

    #ifdef INIT_OPT_LOC
    ParamSetIndex = 1;
    #endif

    // 防止非管理员登录的显示错误
    if (IsSuperUser == 0)
    {
        SysParamCount = 5;
        if (ParamSetIndex > SysParamCount)
        {
            ParamSetIndex = 1;
        }
    }
    else
    {
        SysParamCount = 10;
    }
    ShowParamSetUI();
}

void ParamSetUIKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT:  case POWER: EnterMeasuInter();  break;
        case RETURN:   InterMainMenu();    break;
        case OKKEY:
        {
            switch (ParamSetIndex)
            {
                case 3: InterChngPSWUI();       break;
                case 4: InterModSysTimeUI();    break;
                //case 6: InputMenuTimeOut();    break;
                //case 7: InputBkTimeOut();      break;
                case 5: BlueToothSetUI();        break;
                case 6: UseDefParam();          break;
                case 7: SelectDevType();        break;
                case 10: SelectComLog();        break;
            }
            
        }
        break;
        
        case MODE:
        {
            switch (ParamSetIndex)
            {
                case 1: InnerParam.Sound = !InnerParam.Sound;        ShowParamSetUI(); break;
                case 2: InnerParam.Keytone = !InnerParam.Keytone;    ShowParamSetUI(); break;
                //case 3: SetBkParam();                                 ShowParamSetUI(); break;
                case 8: DebugMode = !DebugMode;                     ShowParamSetUI(); break;
                case 9: SwInnerSensor();           break;
            }
        }
        break;
        
        case DOWN:
        {           
            if(++ParamSetIndex > SysParamCount)
            {
                ParamSetIndex = 1;
            }           
            ShowParamSetUI();
        }
        break;
        
        case UP:
        {
            if(--ParamSetIndex < 1)
            {              
                ParamSetIndex = SysParamCount;               
            }
            ShowParamSetUI();
        }
        break;
    }

}



//修改密码界面
extern int num1;
extern int num2;
extern int num3;
extern int num4;


char ChanPSWDIndex = 0;
void InterChngPSWUI()
{
    ChanPSWDIndex = 0;
    currUI = CHGNPSW;

    PopWindow(10,36,100,60,"请输入新密码");
    
    //GT_Show12X12Char(0,0,"修改密码",0);
    Show16X24int(24,50,num1,1);
    Show16X24int(40,50,num2,0);
    Show16X24int(56,50,num3,0);
    Show16X24int(72,50,num4,0);
    
    GT_Show12X12Char(80,80,"确定",0);
    DisplayRefresh();
}

void RefreshChngPSWUI()
{    
    if(ChanPSWDIndex == 0)
    {
        Clear(24,50,16,24);
        Show16X24int(24,50,num1,1);    
        
        Clear(80,80,28,12);
        GT_Show12X12Char(80,80,"确定",0);
    }
    else if(ChanPSWDIndex == 1)
    {
        Clear(40,50,16,24);
        Show16X24int(40,50,num2,1);
        
        Clear(24,50,16,24);
        Show16X24int(24,50,num1,0);
    }
    else if(ChanPSWDIndex == 2)
    {
        Clear(56,50,16,24);
        Show16X24int(56,50,num3,1);
        
        Clear(40,50,16,24);
        Show16X24int(40,50,num2,0);
    }
    else if(ChanPSWDIndex == 3)
    {
        Clear(72,50,16,24);
        Show16X24int(72,50,num4,1);
        
        Clear(56,50,16,24);
        Show16X24int(56,50,num3,0);
    }
    else if(ChanPSWDIndex == 4)
    {
        Clear(72,50,16,24);
        Show16X24int(72,50,num4,0);  
      
        Clear(80,80,28,12);
        GT_Show12X12Char(80,80,"确定",1);
    }
    DisplayRefresh();
}

void ChngPSWUIKey(PRESSKEY  dir)
{
    
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        
        case RETURN:  
        {
            num1 = num2 = num3 = num4 = 0;
            ChanPSWDIndex = 0;
            InterParamSetUI();
        }
        break;
        
        case UP:  //按键 上键
        {
            if(ChanPSWDIndex == 0)
            {
                if(++num1 > 9)
                {
                    num1 = 0;
                }
            }
            if(ChanPSWDIndex == 1)
            {
                if(++num2 > 9)
                {
                    num2 = 0;
                }
            }
            if(ChanPSWDIndex == 2)
            {
                if(++num3 > 9)
                {
                    num3 = 0;
                }
            }
            if(ChanPSWDIndex == 3)
            {
                if(++num4 > 9)
                {
                    num4 = 0;
                }
            }
            RefreshChngPSWUI();
        }
        break;
        
        case DOWN:  //按键 下键
        {
            if(ChanPSWDIndex == 0)
            {
                if(--num1 < 0)
                {
                    num1 = 9;
                }
            }
            if(ChanPSWDIndex == 1)
            {
                if(--num2 < 0)
                {
                    num2 = 9;
                }
            }
            if(ChanPSWDIndex == 2)
            {
                if(--num3 < 0)
                {
                    num3 = 9;
                }
            }
            if(ChanPSWDIndex == 3)
            {
                if(--num4 < 0)
                {
                    num4 = 9;
                }
            }
            RefreshChngPSWUI();
        }
        break;
        
        case OKKEY:  //按键 模式键
        {
            if(ChanPSWDIndex == 4)
            {
                InnerParam.PassWord = num1*1000+num2*100+num3*10+num4;
                num1 = num2 = num3 = num4 = 0;
                //返回其他参数界面
                
                ChanPSWDIndex = 0;

                ShowParamSetUI();
                ShowHint("密码修改成功",InterParamSetUI);
                return;
            }
            ++ChanPSWDIndex;
            RefreshChngPSWUI();
        }
        break;
    }
}


char ModSysTmIndex = 0;
int modYear = 0;
int modMonth = 0;
int modDay = 0;
int modHour = 0;
int modMinute = 0;
int modSecond = 0;
void InterModSysTimeUI()
{
    currUI = MODSYSTM;
    Clear_DispBuf();
    DS1337_QTIME cuTime;
    ModSysTmIndex = 0;

    GT_Show16X16Char(16,0,"修改系统时间",0);
    
    GT_Show12X12Char(0,20,"当前时间",0);
    
    GT_Show12X12Char(0,54,"设置时间",0);
    
    GetTimer(&cuTime);
    
    memset(StrTemp,0,24);
    sprintf(StrTemp,"%d-%02d-%02d %02d:%02d:%02d",cuTime.year%100,cuTime.month,cuTime.day,cuTime.hour,cuTime.minute,cuTime.second);
    Show5X7String(0,36,StrTemp,0);
    modYear = cuTime.year%100;
    modMonth = cuTime.month;
    modDay = cuTime.day;
    modHour = cuTime.hour;
    modMinute = cuTime.minute;
    modSecond = cuTime.second;
    ShowModTime();
}

void ShowModTime()
{
    Clear(0,70,128,19);

    const unsigned char Sign[] = {
        0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x03,0x80,0x03,0x80,0x07,0xC0,0x07,0xC0,
	    0x0F,0xE0,0x0F,0xE0,0x1F,0xF0,0x1F,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    
    
    if(ModSysTmIndex != 6)
    {
        Diplay(ModSysTmIndex*17,77,16,16,Sign,0);    
    }
    
    //char SetTime[24] = "";    
    memset(StrTemp,0,24);
    sprintf(StrTemp,"%d-%02d-%02d %02d:%02d:%02d",modYear,modMonth,modDay,modHour,modMinute,modSecond);
    Show5X7String(0,70,StrTemp,0);

    Clear(90,100,32,16);
    GT_Show16X16Char(90,100,"确定",ModSysTmIndex == 6);
    
    DisplayRefresh();
}

void SetModTime()
{
    DS1337_QTIME ModTime;
    ModTime.year = modYear + 2000;
    ModTime.month = modMonth;
    ModTime.day = modDay;
    ModTime.hour = modHour;
    ModTime.minute = modMinute;
    ModTime.second = modSecond;
    SetTimer(&ModTime);    
}

void ModSysTimeKey(PRESSKEY  dir)
{
    
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        
        case RETURN:  
        {   
            ModSysTmIndex = 0;
            InterParamSetUI();
        }
        break;
        
        case OKKEY:  //按键 模式键
        {
            if(ModSysTmIndex == 6)
            {
                SetModTime();
                GetTimer(&cuTime);  // 刷新时间
                ShowHint("修改成功",InterParamSetUI);
                //InterParamSetUI();
                return;
            }
            ++ModSysTmIndex;
            ShowModTime();
        }
        break;
        
        case UP:  //按键 模式键
        {
            if(ModSysTmIndex == 0)//年
            {
                if(++modYear>99)
                {
                    modYear = 0;
                }
            }
            if(ModSysTmIndex == 1)//月
            {
                if(++modMonth>12)
                {
                    modMonth = 1;
                }
            }
            if(ModSysTmIndex == 2)//日
            {
                if(++modDay>31)
                {
                    modDay = 1;
                }
            }
            if(ModSysTmIndex == 3)//时
            {
                if(++modHour>23)
                {
                    modHour = 0;
                }
            }
            if(ModSysTmIndex == 4)//分
            {
                if(++modMinute>59)
                {
                    modMinute = 0;
                }
            }
            if(ModSysTmIndex == 5)//秒
            {
                if(++modSecond>59)
                {
                    modSecond = 0;
                }
            }
            ShowModTime();
        }
        break;
        
        case DOWN:  //按键 模式键
        {
            if(ModSysTmIndex == 0)//年
            {
                if(--modYear<0)
                {
                    modYear = 99;
                }
            }
            if(ModSysTmIndex == 1)//月
            {
                if(--modMonth<1)
                {
                    modMonth = 12;
                }
            }
            if(ModSysTmIndex == 2)//日
            {
                if(--modDay<1)
                {
                    modDay = 31;
                }
            }
            if(ModSysTmIndex == 3)//时
            {
                if(--modHour<0)
                {
                    modHour = 23;
                }
            }
            if(ModSysTmIndex == 4)//分
            {
                if(--modMinute<0)
                {
                    modMinute = 59;
                }
            }
            if(ModSysTmIndex == 5)//秒
            {
                if(--modSecond<0)
                {
                    modSecond = 59;
                }
            }
            ShowModTime();
        }
        break;
    }
}


// 设备类型
BYTE DevTypeIndex = 1;
void RefreshDevType()
{
    PopWindow(60,60,60,46,"");
    Show5X7String(64,64,"MPRP-S2N",  DevTypeIndex == 1);
    Show5X7String(64,72,"MPRP-S2D",  DevTypeIndex == 2);
    Show5X7String(64,80,"MPRP-S3",  DevTypeIndex == 3);
    Show5X7String(64,88,"MPRP-S2L",  DevTypeIndex == 4);
    Show5X7String(64,96,"MPRP-S5",  DevTypeIndex == 5);
    DisplayRefresh();
}

void SelectDevType()
{
    currUI = DEVTYPE;
    DevTypeIndex = InnerParam.DevType;
    RefreshDevType();
}


void DevTypeKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT:  case POWER: EnterMeasuInter();  break;
        case RETURN:   ShowParamSetUI();    break;
        
        case OKKEY:
        {
            if (InnerParam.DevType != DevTypeIndex)
            {
                InnerParam.DevType = DevTypeIndex;
            }

            ShowParamSetUI();
            
        }
        break;
        
        case MODE:
        {
            
        }
        break;
        
        case DOWN:
        {           
            if(++DevTypeIndex > 5)
            {
                DevTypeIndex = 1;
            }           
            RefreshDevType();
        }
        break;
        
        case UP:
        {
            if(--DevTypeIndex < 1)
            {              
                DevTypeIndex = 5;               
            }
            RefreshDevType();
        }
        break;
    }

}


// 公司Logo
BYTE ComLogIndex = 1;
void RefreshComLog()
{
    PopWindow(80,60,20,46,"");
    Show5X7String(84,64,"0",  ComLogIndex == 1);
    Show5X7String(84,72,"1",  ComLogIndex == 2);
    Show5X7String(84,80,"2",  ComLogIndex == 3);
    Show5X7String(84,88,"3",  ComLogIndex == 4);
    Show5X7String(84,96,"4",  ComLogIndex == 5);
    DisplayRefresh();
}

void SelectComLog()
{
    currUI = COMLOG;
    ComLogIndex = InnerParam.ComLog+1;
    RefreshComLog();
}


void ComLogKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT:  case POWER: EnterMeasuInter();  break;
        case RETURN:   ShowParamSetUI();    break;
        
        case OKKEY:
        {
            InnerParam.ComLog = ComLogIndex-1;
            ShowParamSetUI();
        }
        break;
        
        case MODE:
        {
            
        }
        break;
        
        case DOWN:
        {           
            if(++ComLogIndex > 5)
            {
                ComLogIndex = 1;
            }           
            RefreshComLog();
        }
        break;
        
        case UP:
        {
            if(--ComLogIndex < 1)
            {              
                ComLogIndex = 5;               
            }
            RefreshComLog();
        }
        break;
    }

}


