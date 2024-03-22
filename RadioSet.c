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

BYTE  CurRadIndex = 0;   // ����ʹ�ú����ں��ؿ��е�����
BYTE  RadCount = 0;      // ��������(����ϳ���)
RADIO pnuc[25];      //����


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
    
    pnuc[0].Valid = 1;  // ��ϳ�������Ч
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


// ��ȡ��ǰ���ص�ͨ������
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

// ��ȡ��ǰ���ص�����
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
        //��ʾ ������
        char alfa[1] = {0};
        ShowSpecChar(x,y,alfa,0,1); 
    }
    else if(Channel == CHANNEL_B)
    {
        //��ʾ����&٤��
        char BeitaGama[2] = {1,2};
        ShowSpecChar(x,y,BeitaGama,0,2); 
    }
    else if(Channel == CHANNEL_MIX) // MIX
    {
        //��ʾ ������ ���� ٤��
        //char alfaBeitaGama[3] = {0,1,2};
        //ShowSpecChar(110,7,alfaBeitaGama,0,3);
        
        // ��ϳ�����̽ͷ��ʾͨ��
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

#define RADLINES  14    // һ����ʾ14�к���
int RadSetSel = 0;
int PageNum = 0;
//BYTE SelectedRad = 0;
int CntRadio = 0;
int RadioStart = 0;


void ShowNucDetail(BYTE i)
{
    GT_Show16X16Char(0,4,"����",0);
    GT_Show8X16Ascii(32,4,":",0);
    
    if (Senser.Param.Hn == pnuc[i].index)
    {   
        // �ڵ�ǰ����ǰ����ʾһ��*
        Show5X7String(40,7,"*",0);    
        GT_Show8X16Ascii(48,4,NucName[pnuc[i].index],0);    
    }
    else
    {
        GT_Show8X16Ascii(40,4,NucName[pnuc[i].index],0);    
    }
    
    ShowChannel(110,7,pnuc[i].Channel);
    
    Show12X12String(0,26,"��ǰЧ��",0);
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

// line : ��ʾ����,  index : ��������
void ShowRad(BYTE line, BYTE index)
{
    // ��ʾ������
    if (Senser.Param.Hn == pnuc[index].index)
    {   
        // �ڵ�ǰ����ǰ����ʾһ��*
        Show5X7String(0,13+(line%RADLINES)*8,"*",0);
        Show5X7String(6,13+(line%RADLINES)*8,NucName[pnuc[index].index],0);
    }
    else
    {
        Show5X7String(0,13+(line%RADLINES)*8,NucName[pnuc[index].index],0);
    }
    
    // ��ʾ����̽ͷ
    ShowChannel(48, 13+(line%RADLINES)*8, pnuc[index].Channel);


    
    // ��ʾ��ֵ
    if ( ( (CheckRes.SenserType == SENSER_G1)    || 
            (CheckRes.SenserType == SENSER_G2)    ||
            (CheckRes.SenserType == SENSER_ABY)   ||
            (CheckRes.SenserType == SENSER_A_AB)  ||
            (CheckRes.SenserType == SENSER_B_AB)
          ) && (index == 0)   // ��ϳ�
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
    
    GT_Show12X12Char(0,0,"����",0);
    GT_Show12X12Char(40,0,"ͨ��",0);
    GT_Show12X12Char(76,0,"��ֵ",0);

    
    RefreshRadSet();
}


void InterRadSetMenu()
{
    if (!HaveRadSet())
    {
        ShowHint("��̽ͷ�޺���",InterMainMenu);
        return;
    }
    
    currUI = RADLIST;
    ShowRadSetUI();
}


void RefreshRadSet()
{
    // ����Ҫ����̽ͷ��ʾ��Ӧ�ĺ���
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
        // A����
        CntRadio = 7;
        RadioStart = 18;
    }
    else
    {
        // BY����
        CntRadio = 17;
        RadioStart = 1;
    }
    #else
    CntRadio = RadCount;
    RadioStart = 1;
    #endif

    
    if (PageNum == 0)
    {
        ShowRad(0,0);// ��һҳҪ��ʾ��ϳ�
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




// ѡ����ص�λ
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






//������ϸ���ý���
char RadDetailIndex = 1;
char IsUseThisRad = 1;
int SelAlarmThres = 0;     
float AlarmThresTemp = 0.0;  // ������ֵ
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

    GT_Show12X12Char(0,50,"������λ",    RadDetailIndex == 1);
    GT_Show12X12Char(0,66,"������ֵ",    RadDetailIndex == 2);
    GT_Show12X12Char(0,82,"�����Ӧ",    RadDetailIndex == 3);
    GT_Show12X12Char(0,98,"Ч�����±궨",RadDetailIndex == 4);
    GT_Show12X12Char(0,114,"���øú���",  RadDetailIndex == 5);


    //��ʾ��λ
    Show5X7String(70,53,":",0);
    Show5X7String(82,53,UnitStr[pnuc[InnerParam.RadioSelIndex].Unit],0);

    // ��ʾ��ֵ
    // G1/G2̽ͷ�Ļ�ϳ���ֵ�ڱ�������������
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

        //�����Ӧ
        RadActiveTemp = pnuc[InnerParam.RadioSelIndex].Active;
        sprintf(StrTemp,": %0.2f",RadActiveTemp);
        Show5X7String(70,85,StrTemp,0);  
    }
    else
    {
        // ��ϳ�
        Show5X7String(70,69,": ...",0);  
        Show5X7String(70,85,": ...",0);  
    }
    
    ShowOptHint();
    DisplayRefresh();
}



void InputRadAlarm()
{
    InFloat.Title = "������ֵ";
    
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
        ShowHint("�����Ӧ����ʧ��", InterRadDetailSetMenu);
    }
    else
    {
        pnuc[InnerParam.RadioSelIndex].Active = RadActiveTemp;
        
        SaveSenserParam();
        
        //AyncAlarmThres(Thres);
        //memcpy(&Senser.Alarm,&AlarmTmp,sizeof(AlarmTmp));
        //Senser.
                    
        //ShowHint("�������óɹ�", InterRadDetailSetMenu);
        StorageLog(EVENT_MOD_OUTALARM);
        InterRadDetailSetMenu();
    }
}

void InputRadActive()
{
    if (InnerParam.RadioSelIndex == 0)  // ��ϳ�
    {
        ShowHint("��ϳ��޴˲���",InterRadDetailSetMenu);
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
    // �����µĲ���
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

    
    //  �����ǰ����Ϊ����ʹ�õĺ��أ���Ҫ��
    //  ��ֵ�·���̽ͷ
    if (InnerParam.RadioSelIndex == Senser.Param.Hn)
    {
        //  �������·���̽����
        if (!SetAlarmThres(Thres))
        {
            ShowHint("��������ʧ��", InterRadDetailSetMenu);
        }
        else
        {
            pnuc[InnerParam.RadioSelIndex].AlarmThres = Thres;
            
            SaveSenserParam();
            // �����µĲ���
            //memcpy(&Senser.Alarm,&AlarmTmp,sizeof(AlarmTmp));
            //Senser.
                        
            //ShowHint("�������óɹ�", InterRadDetailSetMenu);
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
    //  �������·���̽����
    static float Thres;
    Thres = ToCps(AlarmThresTemp, pnuc[InnerParam.RadioSelIndex].Unit);
    if (!SetAlarmThres(Thres))
    {
        ShowHint("������ֵ����ʧ��", InterRadDetailSetMenu);
    }
    else
    {
        pnuc[InnerParam.RadioSelIndex].AlarmThres = Thres;
        
        SaveSenserParam();
        
        AyncAlarmThres(Thres);
        //memcpy(&Senser.Alarm,&AlarmTmp,sizeof(AlarmTmp));
        //Senser.
                    
        //ShowHint("�������óɹ�", InterRadDetailSetMenu);
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
            if (0) // ���ﲻ��Ҫ���º���Ч��
            {
                PopHint("���������²���");
                WriteParamToFlash();
                
                ShowHint("�������óɹ�", InterRadSetMenu);
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
                    // �µ����������޸ĵ�λ
                    #if 1   // 2023.10.8 ���´򿪣��ͻ���Ҫ�����޸ĵ�λ
                    // ��ϳ������޸ĵ�λ
                    if (InnerParam.RadioSelIndex == 0)
                    {
                        ShowHint("��ϳ������޸ļ�����λ",InterRadDetailSetMenu);
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
                    // G1/G2̽ͷû����ֵ
                    if (OneThres())
                    {
                        InputRadAlarmEx();    
                    }
                    else
                    {
                        // ��ϳ�
                        //ShowHint("�ú����޴˲���",InterRadDetailSetMenu);
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
    
    GT_Show12X12Char(0,13,"������ֵ",RadDetailIndex == 1);
    GT_Show12X12Char(0,29,"���øú��ز���",0);
    Show5X7String(96,29,"[   ]",0);   
    
    Clear(104,27,16,16);
    if(IsUseThisRad == 0)
    {
        GT_Show12X12Char(104,27,"��",RadDetailIndex == 2);
        Param.RadioSelIndex = 0;
    }
    else if(IsUseThisRad == 1)
    {
        GT_Show12X12Char(104,27,"��",RadDetailIndex == 2);
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
    GT_Show12X12Char(0,47,"Ч�ʱ궨",RadDetailIndex == 3);
    Clear(100,47,28,17);
    GT_Show12X12Char(100,47,"ȷ��",RadDetailIndex == 4);
    
    DisplayRefresh();
}
#endif




//Ч�ʱ궨����  effidemar
WORD DemarTimeOut = 0;     // ʣ��ʱ��
float uiSourcActiv = 5000; // ���
float uiDemarTime = 30;    // �궨ʱ��
float ucEffi = 0.0;

char EffiDemarIndex = 1;
void InterEffiDemarMenu()
{    
    currUI = EFFIDEMAR;
    
    if(InnerParam.RadioSelIndex == 0)
    {
        ShowHint("��ϳ����ܱ궨",InterRadDetailSetMenu);
        return;
    }

    RefreshEffiDemar();
}

void RefreshEffiDemar()
{
    Clear_DispBuf();
    ShowNucDetail(InnerParam.RadioSelIndex);
    
    //GT_Show12X12Char(0,82,"��������Ӧ",EffiDemarIndex == 3);
    GT_Show12X12Char(0,50,"���淢����",EffiDemarIndex == 1);
    GT_Show12X12Char(0,66,"�궨ʱ��",EffiDemarIndex == 2);
    GT_Show12X12Char(0,82,"��ʼ�궨",EffiDemarIndex == 3);
    
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
    InFloat.Title = "�궨Դ���";
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
    InFloat.Title = "�궨ʱ��";
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


// ����Ч�ʱ궨
BOOL StartDemar(BYTE id)
{
    BOOL ret;

    // 1 . �������غ�
    if (!SetSenserNuc(id))
    {
        return FALSE;

    }

    //pnuc[id].AlarmThres = ;
    //pnuc[id].Channel = ;
    // 2.  �·�Ч�ʱ궨����
    ret = SenserSendFloat(SCMD_XL_FIX, 1, uiDemarTime, 0);
    if (!ret)
    {
        return FALSE;
    }

    // 3.  �·�Ч�ʱ궨���
    ret = SenserSendFloat(SCMD_XL_FIX, 2, uiSourcActiv, 0);
    if (!ret)
    {
        return FALSE;
    }

    // ���������
    // �궨ͨ��0x01��ʾ����0x02��ʾ��
    SenserSendFloat(SCMD_XL_FIX, 3, (float)pnuc[id].Channel, 0);  
    
    return TRUE;
}



//unsigned int uiMeterTime = 0;
//unsigned long SumCps = 0;
//�궨����
//char DemarUIIndex = 1;


void RefreshDemarUI()
{
    //char str[10] = {0};
    memset(StrTemp,0,24);
    Clear_DispBuf();
    ShowNucDetail(InnerParam.RadioSelIndex);
    
    
    Show12X12String(0,60,"�������±궨����Ч��",0);
    Show12X12String(0,76,"���Ժ�...",0);
    GT_Show12X12Char(0,92,"ʣ��ʱ��",0);
    
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
        if (Cnt%4 == 0)   // 4*300ms == 1.2s  ����ӳ���
        {
            if (DemarTimeOut-- == 0)
            {
                ShowHint("Ч�ʱ궨ʧ��",InterRadDetailSetMenu);
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
        ShowHint("�����궨ʧ��", InterEffiDemarMenu);
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

        // ������������Ӧ
        // �����жϱ궨����
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
        Show12X12String(0,60,"Ч�ʱ궨ʧ��",0);
        Show12X12String(0,76,"Ч��ֵ",0);

        if (ucEffi < 0.0001)
        {
            sprintf(StrTemp,": FF%%");
        }
        else
        {
            sprintf(StrTemp,": %.0f%%", ucEffi*100);
        }
        Show12X12String(42,76,StrTemp,0);

        Show12X12String(50,112,"����",1);
    }
    else
    {
        Show12X12String(0,60,"Ч�ʱ궨�ɹ�",0);
        Show12X12String(0,76,"Ч��ֵ",0);

        if (ucEffi < 0.0001)
        {
            sprintf(StrTemp,": FF%%");
        }
        else
        {
            sprintf(StrTemp,": %.0f%%", ucEffi*100);
        }
        Show12X12String(42,76,StrTemp,0);

        Show12X12String(50,112,"����",1);
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
        // ��̽ͷ�ĺ�����������Ϊԭ���ġ�
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
                    
                    ShowHint("����ɹ�",InterRadDetailSetMenu);
                }
                else
                {
                    ShowHint("����ʧ��",InterRadDetailSetMenu);
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
        ShowHint("�����Ѿ�����",InterRadSetMenu);
        return;
    }

    PopHint("�������ú���");

    
    if (InnerParam.RadioSelIndex != 0)
    {
        if (pnuc[InnerParam.RadioSelIndex].Active == 0.0)
        {
            // û�б궨�ĺ��ز�������
            if (pnuc[InnerParam.RadioSelIndex].Efficiency == 0.0)
            {
                ShowHint("δ�궨�ĺ��ز�������",InterRadDetailSetMenu);
                return;
            }
        }
    }

    if (pnuc[InnerParam.RadioSelIndex].AlarmThres == 0.0)
    {
        ShowHint("������������",InterRadDetailSetMenu);
        return;
    }
    
    if (!SetSenserNuc(pnuc[InnerParam.RadioSelIndex].index))
    {
        ShowHint("��������ʧ��",InterRadDetailSetMenu);
        return;
    }

    #if 0   // ---- �µ�ģʽ�����·���
    // �·�������ֵ
    // G1/G2 �Ļ�ϳ������ﲻ�·�
    if (OneThres())
    {
        if (!SetAlarmThres(pnuc[InnerParam.RadioSelIndex].AlarmThres))
        {
            ShowHint("��������ʧ��", InterRadDetailSetMenu);
            return;
        }
    }
    #endif

    if (!SaveSenserParam())
    {
        ShowHint("���ر���ʧ��",InterRadDetailSetMenu);
        return;
    }
    
    Senser.Param.Hn = pnuc[InnerParam.RadioSelIndex].index;
    CurRadIndex = InnerParam.RadioSelIndex;

    AyncAlarmThres(pnuc[InnerParam.RadioSelIndex].AlarmThres);
    
    WriteParamToFlash();

    ClearMax();
    ShowHint("�������óɹ�",InterRadSetMenu);
    
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
        GT_Show12X12Char(0,0,"�궨���",0);
        Show16X24float(33,16,ucEffi,0);
        Diplay(90,16,16,24,PerSig,0);
        Show5X7String(10,44,"OK",0);
        GT_Show12X12Char(26,42,"����",0);
        Show5X7String(74,44,"M",0);
        GT_Show12X12Char(82,42,"������",0);
        
        
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
    GT_Show12X12Char(0,24,"�Ƿ����øú���",0);
    Show5X7String(96,26,"[   ]",0); 
    RefreshUseRad();
}

void RefreshUseRad()
{
    Clear(104,24,16,16);
    if(IsUseThisRad == 0)
    {
        GT_Show12X12Char(104,24,"��",0);
        Param.RadioSelIndex = 0;
    }
    else if(IsUseThisRad == 1)
    {
        GT_Show12X12Char(104,24,"��",0);
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
            GT_Show12X12Char(38,26,"�޸ĳɹ�",0);
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


