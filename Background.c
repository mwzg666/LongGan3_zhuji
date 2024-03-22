#include <stdio.h>
#include <string.h>
#include <msp430x552x.h>
#include "system.h"
#include "CommDataDefine.h"

#include "Keyboard.h"
#include "oled.h"
#include "Background.h"
#include "MainMenu.h"
#include "OtherParam.h"
#include "Measinter.h"
#include "Senser.h"
#include "SelfCheck.h"
#include "Main.h"
#include "Input.h"

extern INTERFACE currUI;
extern QPara Param;
extern SENSER Senser;
extern CHECKRES CheckRes;
extern BYTE CanRecBuf[14][8];
extern char * const UnitStr[];
extern char StrTemp[24];



BYTE MeterTime = 10;   // ���ز���ʱ��
SENSERGND GndTmp;

void GetBackRes(BYTE cnt)
{
    BYTE i;
    GndTmp.Res = 0;
    for (i=0;i<cnt;i++)
    {
        switch (CanRecBuf[i][0])
        {
            case 2: GndTmp.AVal =  CanDataToFloat(&CanRecBuf[i][0]); break;
            case 3: GndTmp.BVal =  CanDataToFloat(&CanRecBuf[i][0]); break;
            case 1: GndTmp.Res =   1;                                   break;
        }
    }
}

void SaveBackRes()
{
    Senser.Gnd.AVal = GndTmp.AVal;
    Senser.Gnd.BVal = GndTmp.BVal;
    Senser.Gnd.Res  = GndTmp.Res;
    
    memcpy(&Param.Gnd,&Senser.Gnd,sizeof(Param.Gnd));

    //PopHint("���ڱ��汾��");
    WriteParamToFlash();
}


void ShowBackRes()
{
    //char Str[16] = {0};
    //memset(StrTemp,0,24);
    if (GndTmp.Res)
    {
        GT_Show12X12Char(0,60,"���ײ����ɹ�",0);
    }
    else
    {
        GT_Show12X12Char(0,60,"���ײ���ʧ��",0);
    }

    if ((CheckRes.SenserType == SENSER_A_AB)  ||
         (CheckRes.SenserType == SENSER_B_AB) ||
         (CheckRes.SenserType == SENSER_G1)   ||
         (CheckRes.SenserType == SENSER_G2)  // ||
         //(CheckRes.SenserType == SENSER_ABY)
        )
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp,"@A : %0.1f %s",GndTmp.AVal,UnitStr[UNIT_CPS]);
        Show5X7String(24,76,StrTemp,0);

        memset(StrTemp,0,24);
        sprintf(StrTemp,"@B@Y : %0.1f %s",GndTmp.BVal,UnitStr[UNIT_CPS]);
        Show5X7String(18,86,StrTemp,0);    
    }
    else if ((CheckRes.SenserType == SENSER_A_A) ||
             (CheckRes.SenserType == SENSER_B_A))
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp,"@A : %0.1f %s",GndTmp.AVal,UnitStr[UNIT_CPS]);
        Show5X7String(24,76,StrTemp,0);
    }
    else if ((CheckRes.SenserType == SENSER_A_B) ||
             (CheckRes.SenserType == SENSER_B_B))
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp,"@B@Y : %0.1f %s",GndTmp.BVal,UnitStr[UNIT_CPS]);
        Show5X7String(18,86,StrTemp,0);
    }
    else
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp,"%0.1f %s",GndTmp.AVal,UnitStr[UNIT_CPS]);
        Show5X7String(18,86,StrTemp,0);
    }
}


void InterBackResUI()
{
    currUI = BACK;
    
    Clear_DispBuf();
    ShowBackSetHead();
    
    ShowBackRes();
    
    
    GT_Show12X12Char(20,114,"�����ȷ��",1);
    DisplayRefresh();

    SaveBackRes();
}

void BackResKey(PRESSKEY  key)
{
    #if 0
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterMainMenu();  break;
        
        case OKKEY:
        {
            SaveBackRes();
            ShowHint("����ɹ�",InterMainMenu);
        }
        break;
    }
    #else
    //SaveBackRes();
    EnterMeasuInter();
    //ShowHint("����ɹ�",EnterMeasuInter);
    #endif
}



void RefreshBackUI()
{
    //BYTE i,j;
    //char Str[16] = {0};
    memset(StrTemp,0,24);
    Clear_DispBuf();
    ShowBackSetHead();
    

    //��ʾ���ײ���
    GT_Show12X12Char(0,64,"����ǿ�Ʊ��ײ���",0);
    Show12X12String(0,80,"���Ժ�...",0);
    //��ʾ����ʱ��
    GT_Show12X12Char(0,96,"ʣ��ʱ��",0);
    sprintf(StrTemp,": %d s",MeterTime);
    Show5X7String(54,99,StrTemp,0);

    #if 0
    if (GetSenserCounter())
    {
        for (i=0;i<Senser.Contxt->CounterCnt;i++)
        {
            j = GetValidCounterLoc(i);
            Show5X7Int(0+i*40,100, GetCounterVal(j),0);
        }
    }
    #endif
    
    DisplayRefresh();

}


void BackTest()
{
    static WORD Cnt = 0;
    BYTE ret;
    //while(1)
    {
        ret = CanWaitCmd(SCMD_BACK_TEST);
        if (ret)
        {
            GetBackRes(ret);
            InterBackResUI();
            return;
        }
        else
        {
            Cnt ++;
            if (Cnt%4 == 0)   // 4*300 == 1200ms  ��ʱ
            {
                if (MeterTime-- == 0)
                {
                    ShowHint("���ײ���ʧ��",EnterMeasuInter);
                    return;
                    //break;
                }
                RefreshBackUI();
            }
        }
    }

    
}


void StartBackTest()
{
    currUI = BACKTEST;
    
    if (!SenserBackTest())
    {
        ShowHint("��������ʧ��",InterBackSetUI);
        return;
    }
 
    //SumAlfacps = 0;
    //SumBertacps = 0;

    if(Senser.Gnd.TestTime == 1)
    {
        MeterTime = 10;
    }
    else if(Senser.Gnd.TestTime == 2)
    {
        MeterTime = 30;
    }
    else if(Senser.Gnd.TestTime == 3)
    {
        MeterTime = 60;
    }
    else if(Senser.Gnd.TestTime == 4)
    {
        MeterTime = 120;
    }

    RefreshBackUI();

    //BackTest();
}

void BackTestKey(PRESSKEY key)
{
    BackTest();
}

#if 0
unsigned int BackCount = 0;

 
void ShowBack()
{
    const unsigned char HibckgrAlarm[] = {
          0x07,0xE0,0x08,0x10,0x20,0x04,0x20,0x04,0x52,0xFA,0x92,0x49,0x92,0x71,0x9E,0x49,
        0x92,0x49,0x92,0x49,0x92,0xF1,0x40,0x02,0x20,0x04,0x30,0x0C,0x08,0x10,0x07,0xE0
        };
    
  
    if(++chTest <= 3)
    {
        return;
    }
    chTest = 5;
    
    char Alfastr[8] = {0};
    char Bertastr[8] = {0};
    char Timestr[8] = {0};
    
    
    
    char bAlarm = 0;
        
    if((AerfaHBntAlarm < Alfacps) ||(BeitaHBntAlarm < Bertacps))
    {
        bAlarm = 1;
        Diplay(0,48,16,16,HibckgrAlarm,0);
    }
    
    if(bAlarm == 1)
    { 
        setbit(TIMERCOUNT,3);
        setbit(UMESSAGE,4);
        P5OUT &= ~BIT6;//�ر��̵� 
    }
    else if(bAlarm == 0)
    {
         Clear(0,48,16,16);
         clrbit(UMESSAGE,4);
         P4OUT &= ~BIT7;//�رպ��
         P5OUT |= BIT6;//�����̵�
    }
    BackCount++;
    SumAlfacps += Alfacps;
    SumBertacps += Bertacps;
    
    sprintf(Alfastr,"%lu",SumAlfacps/BackCount);
    sprintf(Bertastr,"%lu",SumBertacps/BackCount);
    sprintf(Timestr,": %d s",--MeterTime);
    
    
    #if 0
    if(MeterChannel == 1)
    {
        Clear(40,20,60,8);
        Show5X7String(50,20,Alfastr,0);
    }
    else if(MeterChannel == 2)
    {
        Clear(40,30,60,8);
        Show5X7String(50,30,Bertastr,0);
    }
    else if(MeterChannel == 3)
    {
        Clear(40,20,60,8);
        Show5X7String(50,20,Alfastr,0);
        
        Clear(40,30,60,8);
        Show5X7String(50,30,Bertastr,0);
    }
    #endif
    
    
    Clear(70,49,45,8);
    Show5X7String(70,49,Timestr,0);
    
    DisplayRefresh();
    
    if(MeterTime == 0)
    {
        clrbit(UMESSAGE,4);
        P4OUT &= ~BIT7;//�رպ��
        P5OUT |= BIT6;//�����̵�
        clrbit(TIMERCOUNT,5);
        chTest = 1;
        BackKey(MODE);
    }
}
#endif




// ���ײ���ʱ��ѡ��
BYTE TimeSelectIndex = 1;
void PopTimeSelWindow()
{
    PopWindow(64,50,30,44,"");
    Show5X7String(68,54,"10s", TimeSelectIndex == 1);
    Show5X7String(68,64,"30s", TimeSelectIndex == 2);
    Show5X7String(68,74,"60s", TimeSelectIndex == 3);
    Show5X7String(68,84,"120s",TimeSelectIndex == 4);
    DisplayRefresh();
}

void InterTimeSelUI()
{
    currUI = TIMESEL;
    
    TimeSelectIndex = Senser.Gnd.TestTime;

    PopTimeSelWindow();    
}

void TimeSelKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER:  EnterMeasuInter(); break;
        case RETURN:  InterBackSetUI();  break;   
        
        case OKKEY:  
        {
            Senser.Gnd.TestTime = TimeSelectIndex;  
            InterBackSetUI();
            break;
        }

        case UP:
        {
            if(--TimeSelectIndex < 1)
            {
                TimeSelectIndex = 4;
            }
            PopTimeSelWindow();
        }
        break;
        
        case DOWN:
        {
            if(++TimeSelectIndex > 4)
            {
                TimeSelectIndex = 1;
            }
            PopTimeSelWindow();
        }
        break;
    }
}


// -----   ���ز������ý��� -------
char BackSetIndex = 1;

// ����̽ͷ������ʾ��ǰ����

void ShowBackVal(BYTE Ty)
{
    memset(StrTemp,0,24);

    switch (Ty)
    {
        case 1:
        {
            //Aerfa ����
            StrTemp[0] = 0;
            ShowSpecChar(26,34,StrTemp,0,1);
            memset(StrTemp,0,24);
            sprintf(StrTemp," : %0.1f %s",Senser.Gnd.AVal,UnitStr[UNIT_CPS]);
            Show5X7String(32,34,StrTemp,0);
            break;
        }

        case  2:
        {
            //Berta ����
            StrTemp[0] = 1;
            StrTemp[1] = 2;
            ShowSpecChar(26,34,StrTemp,0,2);
            memset(StrTemp,0,24);
            sprintf(StrTemp," : %0.1f %s",Senser.Gnd.BVal,UnitStr[UNIT_CPS]);
            Show5X7String(32,34,StrTemp,0);
            break;
        }

        case 3:
        {
            //Aerfa ����
            memset(StrTemp,0,24);
            StrTemp[0] = 0;
            ShowSpecChar(26,30,StrTemp,0,1);
            memset(StrTemp,0,24);
            sprintf(StrTemp," : %0.1f %s",Senser.Gnd.AVal,UnitStr[UNIT_CPS]);
            Show5X7String(32,30,StrTemp,0);

            //Berta ����
            memset(StrTemp,0,24);
            StrTemp[0] = 1;
            StrTemp[1] = 2;
            ShowSpecChar(20,38,StrTemp,0,2);
            memset(StrTemp,0,24);
            sprintf(StrTemp," : %0.1f %s",Senser.Gnd.BVal,UnitStr[UNIT_CPS]);
            Show5X7String(32,38,StrTemp,0);

            break;
        }

        default: 
        {
            //StrTemp[0] = 0;
            //ShowSpecChar(26,34,StrTemp,0,1);
            memset(StrTemp,0,24);
            sprintf(StrTemp,"%0.1f %s",Senser.Gnd.AVal,UnitStr[UNIT_CPS]);
            Show5X7String(32,34,StrTemp,0);
            break;
        }
    }
}

void ShowBackSetHead()
{
    Show12X12String(0,0, "̽ͷ����",0);
    Show5X7String(54,3,":",0);
    ShowSenser(66,3,CheckRes.SenserType);
    Show12X12String(0,16,"��ǰ����",0);

    
    switch (CheckRes.SenserType)
    {
        case SENSER_IN:   ShowBackVal(0);     break;
        case SENSER_PD:   ShowBackVal(0);     break;   //         0x02    //ˮ�¸�����̽ͷ��PD��
        case SENSER_GM:   ShowBackVal(0);     break;  //         0x03    //ˮ���и�����̽ͷ��GM��
        case SENSER_A_A:  ShowBackVal(1);     break;  //       0x04        ��̽ͷ(����alphy)
        case SENSER_A_B:  ShowBackVal(2);     break;  //        0x14    //��̽ͷ(����beta)
        case SENSER_A_AB: ShowBackVal(3);     break;  //      0x24    //��̽ͷ(�ɲ�alphy+beta)
        case SENSER_B_B:  ShowBackVal(2);    break;   //       0x05    //��̽ͷ(����beta)
        case SENSER_B_A:  ShowBackVal(1);    break;   //       0x15    //��̽ͷ(����alphy)
        case SENSER_B_AB: ShowBackVal(3);    break;   //      0x25    //��̽ͷ(�ɲ�alphy+beta)
        case SENSER_ABY:  ShowBackVal(0);    break;   //      0x06    // ���¦�̽ͷ
        case SENSER_G1:   ShowBackVal(3);    break;   //        0x07    //����̽ͷ1
        case SENSER_G2:   ShowBackVal(3);    break;   //        0x08    //����̽ͷ2
    } 
    

    ShowLine(50);
}


void RefreshBackSetUI()
{
    //char Str[16] = {0};
    Clear_DispBuf();
    ShowBackSetHead();
    memset(StrTemp,0,24);
    
    Show12X12String(0,60,"����ʱ��",BackSetIndex == 1);
    
    switch(Senser.Gnd.TestTime)
    {
        case 1: sprintf(StrTemp,": %d s",10);  break;
        case 2: sprintf(StrTemp,": %d s",30);  break;
        case 3: sprintf(StrTemp,": %d s",60);  break;
        case 4: sprintf(StrTemp,": %d s",120); break;
    }
    
    Show5X7String(54,63,StrTemp,0);

    GT_Show12X12Char(0,76,"��ʼ����",BackSetIndex == 2);

    ShowOptHint();
    DisplayRefresh();
    
}

void InterBackSetUI()
{
    if (!Senser.Gnd.Valid)
    {
        ShowHint("��̽ͷ�ޱ���",InterMainMenu);
        return;
    }
    
    BackSetIndex = 1;
    currUI = BACKSET;  
    RefreshBackSetUI();
}


void BackSetKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;   
        case RETURN:  InterMainMenu();   break;
        
        case OKKEY:  //���� OK
        {
            switch (BackSetIndex)
            {
                case 1:  InterTimeSelUI();   break;               
                case 2:  StartBackTest();    break;
            }
        }
        break;

        case UP:
        {
            if(--BackSetIndex < 1)
            {
                BackSetIndex = 2;
            }
            RefreshBackSetUI();
        }
        break;
        
        case DOWN:
        {
            if(++BackSetIndex > 2)
            {
                BackSetIndex = 1;
            }
            RefreshBackSetUI();
        }
        break;
    }
}
