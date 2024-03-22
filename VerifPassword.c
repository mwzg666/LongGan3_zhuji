#include "system.h"
#include "CommDataDefine.h"

#include "Keyboard.h"
#include <msp430x552x.h>
#include "oled.h"

#include <stdio.h>
#include <string.h>
#include "VerifPassword.h"
#include "QuickMenu.h"
#include "MainMenu.h"
#include "Measinter.h"
#include "Input.h"
#include "Main.h"

extern QPara Param;
extern INTERFACE currUI;
extern char IsSuperUser;
extern QInnerParam InnerParam;


void RightPage1()
{
    Clear_DispBuf();
    Show12X12String(0,   0, "本产品版权，均属于",0);
    Show12X12String(0, 13, "成都麦特斯科技有限",0);
    Show12X12String(0, 26, "公司和成都迈为核监",0);
    Show12X12String(0, 39, "测科技有限公司共同",0);
    Show12X12String(0, 52, "所有，未经成都麦特",0);
    Show12X12String(0, 65, "斯科技有限公司或成",0);
    Show12X12String(0, 78, "都迈为核监测科技有",0);
    Show12X12String(0, 91, "限公司书面授权，您",0);
    Show12X12String(0,104, "不得以任何方式擅自",0);
    DisplayRefresh();
}

void RightPage2()
{
    Clear_DispBuf();
    Show12X12String(0,   0, "复制，再造，传播本",0);
    Show12X12String(0, 13, "产品内容，否则将追",0);
    Show12X12String(0, 26, "究法律责任。特此申",0);
    Show12X12String(0, 39, "明。",0);
    DisplayRefresh();
}


void ShowRight()
{
    currUI = RIGHT;
    RightPage1();
}

void RightKey(PRESSKEY  dir)
{
    switch(dir)
    {       
        case UP:  //按键 上键
        {
            RightPage1();
        }
        break;
        
        case DOWN:  //按键 下键
        {
            RightPage2();
        }
        break;
        
        default: EnterMeasuInter();  break;
    }
}


int num1 = 0;
int num2 = 0;
int num3 = 0;
int num4 = 0;
char PassWDIndex = 0;


void InterPasswordUI()
{
    currUI = PASSWD;
    Clear_DispBuf();
    ShowMeteTitle();
    
    PassWDIndex = 0;
    num1 = num2 = num3 = num4 = 0;
    
    GT_Show16X16Char(0,16,"请输入密码",0);
    Show16X24int(24,50,num1,1);
    Show16X24int(40,50,num2,0);
    Show16X24int(56,50,num3,0);
    Show16X24int(72,50,num4,0);
    //返回
    GT_Show16X16Char(90,80,"确定",0);
    ShowOptHint();
    DisplayRefresh();
}

void RefreshPasswordUI(char pos)
{
    if(pos == 0)
    {
        Clear(24,50,16,24);
        Show16X24int(24,50,num1,1);    
        
        Clear(90,80,32,16);
        GT_Show16X16Char(90,80,"确定",0);
        DisplayRefresh();
    }
    else if(pos == 1)
    {
        Clear(40,50,16,24);
        Show16X24int(40,50,num2,1);
        
        Clear(24,50,16,24);
        Show16X24int(24,50,num1,0);
    }
    else if(pos == 2)
    {
        Clear(56,50,16,24);
        Show16X24int(56,50,num3,1);
        
        Clear(40,50,16,24);
        Show16X24int(40,50,num2,0);
    }
    else if(pos == 3)
    {
        Clear(72,50,16,24);
        Show16X24int(72,50,num4,1);
        
        Clear(56,50,16,24);
        Show16X24int(56,50,num3,0);
    }
    else if(pos == 4)
    {
        Clear(72,50,16,24);
        Show16X24int(72,50,num4,0);  
      
        Clear(90,80,32,16);
        GT_Show16X16Char(90,80,"确定",1);
        DisplayRefresh();
    }
    Clear(20,80,64,16);
    DisplayRefresh();
}

void ErrorInfo()
{
    //GT_Show16X16Char(20,80,"密码错误",0);
    //DisplayRefresh();
    ShowHint("密码错误!",InterPasswordUI);
}

void PasswordKey(PRESSKEY  dir)
{
    
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  EnterMeasuInter();  break;
        
        case MODE:  //按键 模式键
        {
            
        }
        break;
        
        case UP:  //按键 上键
        {
            if(PassWDIndex == 0)
            {
                if(++num1 > 9)
                {
                    num1 = 0;
                }
            }
            if(PassWDIndex == 1)
            {
                if(++num2 > 9)
                {
                    num2 = 0;
                }
            }
            if(PassWDIndex == 2)
            {
                if(++num3 > 9)
                {
                    num3 = 0;
                }
            }
            if(PassWDIndex == 3)
            {
                if(++num4 > 9)
                {
                    num4 = 0;
                }
            }
            RefreshPasswordUI(PassWDIndex);
        }
        break;
        
        case DOWN:  //按键 下键
        {
            if(PassWDIndex == 0)
            {
                if(--num1 < 0)
                {
                    num1 = 9;
                }
            }
            if(PassWDIndex == 1)
            {
                if(--num2 < 0)
                {
                    num2 = 9;
                }
            }
            if(PassWDIndex == 2)
            {
                if(--num3 < 0)
                {
                    num3 = 9;
                }
            }
            if(PassWDIndex == 3)
            {
                if(--num4 < 0)
                {
                    num4 = 9;
                }
            }
            RefreshPasswordUI(PassWDIndex);
        }
        break;
        
        case OKKEY:  //按键 模式键
        {
            if(PassWDIndex == 4)
            {
                //校验密码
                if((num1*1000 + num2*100 + num3*10 + num4) == InnerParam.PassWord)
                {
                     num1 = num2 = num3 = num4 = 0;
                    //进入主菜单界面
                    IsSuperUser = 0;
                    //StorageLog(EVENT_USER_LOGIN);
                    InterMainMenu();
                    return;
                }
                else if((num1*1000 + num2*100 + num3*10 + num4) == 1044)
                {
                    IsSuperUser = 1;
                    //StorageLog(EVENT_ADMIN_LOGIN);
                    num1 = num2 = num3 = num4 = 0;
                    //进入主菜单界面
                    InterMainMenu();
                    return;
                }
                else if((num1*1000 + num2*100 + num3*10 + num4) == 2008)
                {
                    ShowRight();
                    return;
                }
                else
                {    
                    //提示密码错误
                    //PassWDIndex = 0;
                    //RefreshPasswordUI(PassWDIndex);
                    ErrorInfo();
                    return;
                }
            }
            
            if(++PassWDIndex > 4)
            {
                PassWDIndex = 0;
            }
            RefreshPasswordUI(PassWDIndex);
        }
        break;
    }
}

