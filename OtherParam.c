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

//��������
char SysParamCount = 5;
char ParamSetIndex = 1;


void UseDefParam()
{
    if (IsSuperUser == 0)
    {
        ShowHint("û��Ȩ��",InterParamSetUI);
        return;
    }
        
    PopHint("���ڻָ�Ĭ�ϲ���...");
    //StorageLog(EVENT_DEF_PARAM);

    // ����ɾ�����ݼ�¼������
    //Eeprom_WriteByte(PARAM_OFFSET,0xFF); 
    
    
    
    Param_Def();

    
    
    WriteParamToFlash();
    Sleep(1000);
    PopHint("�����ɹ�, ���ڹػ�...");
    Sleep(3000);
    PowerOff();
    //ShowHint("�����ɹ�, ",PowerOff);
    //EnterMeasuInter();
}


void InputMenuTimeOut()
{
    InByte.Title = "�˵���ʱʱ��";
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
    InByte.Title = "�������ʱ��";
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
    ShowHint("����������, ʹ�˲�����Ч! ", ShowParamSetUI);
}


void ParamSetUI_Normal()
{
    //if (ParamSetIndex >= 5)
    //{
    //    GT_Show12X12Char(20,32,"��������",ParamSetIndex == 5);
    //}
    //else
    {
        GT_Show12X12Char(20,32,"������",0);
        Show5X7String(58,35,"[   ]",0);
        if(InnerParam.Sound)
        {
            GT_Show12X12Char(66,32,"��",ParamSetIndex == 1);
        }
        else 
        {
            GT_Show12X12Char(66,32,"��",ParamSetIndex == 1);
        }

        
        GT_Show12X12Char(20,48,"������",0);
        Show5X7String(58,51,"[   ]",0);
        if(InnerParam.Keytone)
        {
            GT_Show12X12Char(66,48,"��",ParamSetIndex == 2);
        }
        else 
        {
            GT_Show12X12Char(66,48,"��",ParamSetIndex == 2);
        }
        
        GT_Show12X12Char(20,64,"�޸�����",ParamSetIndex == 3);
        GT_Show12X12Char(20,80,"ʱ������",ParamSetIndex == 4);

        GT_Show12X12Char(20,96,"��������",ParamSetIndex == 5);

        //Show5X7String(20,96,"...",0);   
    }
}

void ParamSetUI_Admin()
{
    if (ParamSetIndex >= 6)
    {
        //GT_Show12X12Char(20,32,"��������",ParamSetIndex == 5);
        GT_Show12X12Char(20,32,"�ָ�Ĭ�ϲ���",ParamSetIndex == 6);
        GT_Show12X12Char(20,48,"�豸����ѡ��",ParamSetIndex == 7);
        
        GT_Show12X12Char(20,64,"����ģʽ",0);
        Show5X7String(74,67,"[   ]",0);
        if(DebugMode)
        {
            GT_Show12X12Char(82,64,"��",ParamSetIndex == 8);
        }
        else 
        {
            GT_Show12X12Char(82,64,"��",ParamSetIndex == 8);
        }

        GT_Show12X12Char(20,80,"�ڲ�̽ͷ",0);
        Show5X7String(74,83,"[   ]",0);
        if (InnerParam.InnerSersor)
        {
            GT_Show12X12Char(82,80,"��",ParamSetIndex == 9);
        }
        else
        {
            GT_Show12X12Char(82,80,"��",ParamSetIndex == 9);
        }


        GT_Show12X12Char(20,96,"�豸������",0);
        memset(StrTemp,0,24);
        sprintf(StrTemp,"[%d]", InnerParam.ComLog);
        Show5X7String(86,99,StrTemp,ParamSetIndex == 10);
        
    }
    else
    {
        GT_Show12X12Char(20,32,"������",0);
        Show5X7String(58,35,"[   ]",0);
        if(InnerParam.Sound)
        {
            GT_Show12X12Char(66,32,"��",ParamSetIndex == 1);
        }
        else 
        {
            GT_Show12X12Char(66,32,"��",ParamSetIndex == 1);
        }

        
        GT_Show12X12Char(20,48,"������",0);
        Show5X7String(58,51,"[   ]",0);
        if(InnerParam.Keytone)
        {
            GT_Show12X12Char(66,48,"��",ParamSetIndex == 2);
        }
        else 
        {
            GT_Show12X12Char(66,48,"��",ParamSetIndex == 2);
        }
        
        GT_Show12X12Char(20,64,"�޸�����",ParamSetIndex == 3);
        GT_Show12X12Char(20,80,"ʱ������",ParamSetIndex == 4);
        GT_Show12X12Char(20,96,"��������",ParamSetIndex == 5);
        Show5X7String(20,112,"...",0);
    }
}


void ShowParamSetUI()
{
    currUI = PARAMSET;
    
    Clear_DispBuf();
    ShowMeteTitle();
    
    GT_Show16X16Char(32,12,"ϵͳ����",0);

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

    // ��ֹ�ǹ���Ա��¼����ʾ����
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



//�޸��������
extern int num1;
extern int num2;
extern int num3;
extern int num4;


char ChanPSWDIndex = 0;
void InterChngPSWUI()
{
    ChanPSWDIndex = 0;
    currUI = CHGNPSW;

    PopWindow(10,36,100,60,"������������");
    
    //GT_Show12X12Char(0,0,"�޸�����",0);
    Show16X24int(24,50,num1,1);
    Show16X24int(40,50,num2,0);
    Show16X24int(56,50,num3,0);
    Show16X24int(72,50,num4,0);
    
    GT_Show12X12Char(80,80,"ȷ��",0);
    DisplayRefresh();
}

void RefreshChngPSWUI()
{    
    if(ChanPSWDIndex == 0)
    {
        Clear(24,50,16,24);
        Show16X24int(24,50,num1,1);    
        
        Clear(80,80,28,12);
        GT_Show12X12Char(80,80,"ȷ��",0);
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
        GT_Show12X12Char(80,80,"ȷ��",1);
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
        
        case UP:  //���� �ϼ�
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
        
        case DOWN:  //���� �¼�
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
        
        case OKKEY:  //���� ģʽ��
        {
            if(ChanPSWDIndex == 4)
            {
                InnerParam.PassWord = num1*1000+num2*100+num3*10+num4;
                num1 = num2 = num3 = num4 = 0;
                //����������������
                
                ChanPSWDIndex = 0;

                ShowParamSetUI();
                ShowHint("�����޸ĳɹ�",InterParamSetUI);
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

    GT_Show16X16Char(16,0,"�޸�ϵͳʱ��",0);
    
    GT_Show12X12Char(0,20,"��ǰʱ��",0);
    
    GT_Show12X12Char(0,54,"����ʱ��",0);
    
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
    GT_Show16X16Char(90,100,"ȷ��",ModSysTmIndex == 6);
    
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
        
        case OKKEY:  //���� ģʽ��
        {
            if(ModSysTmIndex == 6)
            {
                SetModTime();
                GetTimer(&cuTime);  // ˢ��ʱ��
                ShowHint("�޸ĳɹ�",InterParamSetUI);
                //InterParamSetUI();
                return;
            }
            ++ModSysTmIndex;
            ShowModTime();
        }
        break;
        
        case UP:  //���� ģʽ��
        {
            if(ModSysTmIndex == 0)//��
            {
                if(++modYear>99)
                {
                    modYear = 0;
                }
            }
            if(ModSysTmIndex == 1)//��
            {
                if(++modMonth>12)
                {
                    modMonth = 1;
                }
            }
            if(ModSysTmIndex == 2)//��
            {
                if(++modDay>31)
                {
                    modDay = 1;
                }
            }
            if(ModSysTmIndex == 3)//ʱ
            {
                if(++modHour>23)
                {
                    modHour = 0;
                }
            }
            if(ModSysTmIndex == 4)//��
            {
                if(++modMinute>59)
                {
                    modMinute = 0;
                }
            }
            if(ModSysTmIndex == 5)//��
            {
                if(++modSecond>59)
                {
                    modSecond = 0;
                }
            }
            ShowModTime();
        }
        break;
        
        case DOWN:  //���� ģʽ��
        {
            if(ModSysTmIndex == 0)//��
            {
                if(--modYear<0)
                {
                    modYear = 99;
                }
            }
            if(ModSysTmIndex == 1)//��
            {
                if(--modMonth<1)
                {
                    modMonth = 12;
                }
            }
            if(ModSysTmIndex == 2)//��
            {
                if(--modDay<1)
                {
                    modDay = 31;
                }
            }
            if(ModSysTmIndex == 3)//ʱ
            {
                if(--modHour<0)
                {
                    modHour = 23;
                }
            }
            if(ModSysTmIndex == 4)//��
            {
                if(--modMinute<0)
                {
                    modMinute = 59;
                }
            }
            if(ModSysTmIndex == 5)//��
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


// �豸����
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


// ��˾Logo
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


