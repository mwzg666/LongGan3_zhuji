#ifndef __SENSER_H__
#define __SENSER_H__


#define SENSERCOUNT 15

// ���������Ͷ���
#define SENSER_MASTER     0       // ����
#define SENSER_IN         0x01    // ����̽ͷ  
#define SENSER_PD         0x02    //ˮ�¸�����̽ͷ��PD��
#define SENSER_GM         0x03    //ˮ���и�����̽ͷ��GM��
#define SENSER_A_A        0x04    //��̽ͷ(����alphy)
#define SENSER_A_B        0x14    //��̽ͷ(����beta)
#define SENSER_A_AB       0x24    //��̽ͷ(�ɲ�alphy+beta)
#define SENSER_B_B        0x05    //��̽ͷ(����beta)
#define SENSER_B_A        0x15    //��̽ͷ(����alphy)
#define SENSER_B_AB       0x25    //��̽ͷ(�ɲ�alphy+beta)
#define SENSER_ABY        0x06    // ���¦�̽ͷ
#define SENSER_G1         0x07    //����̽ͷ1
#define SENSER_G2         0x08    //����̽ͷ2
#define SENSER_Y          0x09    //����Y ̽ͷ ( NaI ̽ͷ )
#define SENSER_LP         0x2A    // ����Y ̽ͷ    (����)
#define SENSER_LC         0x0B    // Һ�����̽ͷ  (����)
#define SENSER_BC         0xFF    // �㲥
#define SENSER_NONE       0xFE    // û����Ӵ�����


//�����������
#define SCMD_COUNT           15      // �������������
#define SCMD_FIND_SENSER    0x43    // C ֧�ֹ㲥
#define SCMD_READ_PARAM     0x52
#define SCMD_WRITE_PARAM    0x57
#define SCMD_SAVE_PARAM     0x53
#define SCMD_SET_STATUS     0x4B
#define SCMD_READ_STATUS    0x45  // E
#define SCMD_READ_COUNTER   0x56
#define SCMD_SET_BAUDRATE   0x40
#define SCMD_WRITE_ALARM    0x42  // B
#define SCMD_READ_ALARM     0x46  // F
#define SCMD_BACK_TEST      0x41  // A
#define SCMD_XL_FIX         0x47
#define SCMD_GET_RADCNT    0x48   // H
#define SCMD_READ_RAD      0x49   // I
#define SCMD_WRITE_RAD     0x4A   // J

// ��������������������
#define SCMD_CLEAR_TOTOL    0x01
#define SCMD_CLEAR_MAX      0x02
#define SCMD_SELF_CHECK     0x03
#define SCMD_SENSER_POWER   0x04
#define SCMD_GET_BAT         0x05
#define SCMD_CONFIRM_ALARM  0x06
#define SCMD_GM_SW           0x07



#define CAN_TIMEOUT     300

#define PARAMCOUNT  21
#define ALARMCOUNT  16
#define COUNTERCOUNT 11


typedef struct 
{
    BYTE Cmd;
    BYTE RetFrameCnt;    // ���ذ�������, 0 ��ʾ��ʱ����
}CMDINFO;


typedef union
{
    DWORD Id;  
    struct
    {
        BYTE Cmd;
        BYTE Des;
        BYTE Src;
        BYTE Res;
    };
}CANID;




typedef struct __SENSERCONTXT
{
    BYTE Type;                      // ����������
    BYTE ParmCnt;                  // ��������
    BOOL ParamLoc[PARAMCOUNT];     // ����λ��
    BYTE CounterCnt;               // �������ĸ���
    BOOL CounterLoc[COUNTERCOUNT]; // ��������λ��
    BYTE AlarmCnt;                 // ������ֵ����
    BOOL AlarmLoc[ALARMCOUNT];     // ������ֵλ��
    BYTE AlarmId[10];               // ����1ʹ�õı���ID
    BYTE AlarmId2[10];              // ����2ʹ�õı���ID
}SENSERCONTXT;

#pragma pack(2)
typedef struct __SENSERPARAM
{
    float  Hv;  // 0x01+��ѹֵ
    float  Z1;  // 0x02+�������ֵ1
    float  Ct;  // 0x03+����ʱ��
    float  Hd;  // 0x04+��ѹ��
    float  Z2;  // 0x05+�������ֵ2
    float  Z3;  // 0x06+�������ֵ3
    float  S1;  // 0x07+̽��������ϵ��1
    float  S2;  // 0x08+̽��������ϵ��2
    float  Cr;  // 0x09+У׼����
    BYTE   Hn;  // 0x0A+���ú��غ�           ---  9
    float  S3;  // 0x0B ̽��������ϵ��3
    float  Z4;  // 0x0C �������ֵ4

    float Pt;   // 0x0D ƽ��ʱ��
    float LA;   // 0x0E ������ͨ��У׼����A    LP
    float LB;   // 0x0F ������ͨ��У׼����B    LP
    float LC;   // 0x10 ������ͨ��У׼����C    LP
    float HA;   // 0x11������ͨ��У׼����A    LP
    float HB;   // 0x12 ������ͨ��У׼����B   LP
    float HC;   // 0x13 ������ͨ��У׼����C   LP

    float Ss;   // 0x14  ̽�������  -- 2017.9.28 add
    BYTE  Bk;   // 0x15  �Ƿ�۳�����   -- 2018.9.13            ---- 20
    
}SENSERPARAM;



typedef struct __SENSERCOUNTER
{
    float  Aj;  BYTE AjSt;   //0x01+���ڵ���
    float  Hv;  BYTE HvSt;   //0x02+��ѹ����
    float  C1;  BYTE C1St;   // 0x03+����ֵ1+����״̬��1B������������
    float  C2;  BYTE C2St;   //0x04+����ֵ2+����״̬��1B�����¼�����
    float  C3;  BYTE C3St;   // 0x05+����ֵ3+����״̬��1B��(��+��)
    float  P1;  BYTE P1St;   //0x06+ƽ������ֵ1+����״̬��1B������������
    float  P2;  BYTE P2St;   //0x07+ƽ������ֵ2+����״̬��1B������������
    float  P3;  BYTE P3St;   //0x08+ƽ������ֵ3+����״̬��1B������������
    float  Cr;  BYTE CrSt;   // 0x09+������+����״̬��1B��uSv/h   Y
    float  Tc;  BYTE TcSt;   // 0x0A+�ۼƼ���+����״̬��1B��uSv   Y
    float  Mr;  BYTE MrSt;
}SENSERCOUNTER;


#define ALMID_A1   1
#define ALMID_A2   2
#define ALMID_AL   3
#define ALMID_AH   4
#define ALMID_B1   5
#define ALMID_B2   6
#define ALMID_BL   7
#define ALMID_BH   8
#define ALMID_AC   9
#define ALMID_BC   10
#define ALMID_Y1   11
#define ALMID_Y2   12
#define ALMID_Y3   13
#define ALMID_MA   14
#define ALMID_MX   15
#define ALMID_ZU   16



typedef struct __SENSERALARM
{
    float  A1;  // 0x01+һ��������ֵ(��)
    float  A2;  //0x02+����������ֵ(��)
    float  Al;  // 0x03+�ͱ��ױ�����ֵ(��)
    float  Ah;  //0x04+�߱��ױ�����ֵ(��)
    float  B1;  // 0x05+һ��������ֵ(��)
    float  B2;  //0x06+����������ֵ(��)
    float  Bl;  // 0x07+�ͱ��ױ�����ֵ(��)
    float  Bh;  //0x08+�߱��ױ�����ֵ(��)
    float  Ac;  // 0x09+alphy���ر���ֵ
    float  Bc;  //0x0A+beta���ر���ֵ
    float  Y1;  //0x0B Y������һ������(uSv/h)
    float  Y2;  //0x0C Y�����ʶ�������(uSv/h)
    float  Y3;  //0x0D Y��������������(uSv/h)
    float  Ma;  //0x0E �ۼƼ���Ԥ����ֵ
    float  Mx;  //0x0F �ۼƼ���������ֵ
    BYTE   Zu;  //0x10 �ڲ��ۼƼ���������
}SENSERALARM;
#pragma pack()



typedef struct __SENSERWORKST
{
    BYTE ESt;
    BYTE PSt;
    BYTE CSt;
}SENSERWORKST;

//     ����
typedef struct __SENSERGND
{
    BOOL  Valid;      // ��ǰ�Ƿ��б��ز���
    BYTE  TestTime;
    float AVal;       // alerfa  ����ֵ
    float BVal;       // bata   ����ֵ
    BYTE  Res;       // ��������1
}SENSERGND;

// Ч�ʱ궨
typedef struct __SENSERXLFIX
{
    float FixTime;   // �궨ʱ��
    float SrcVal;    // Դ���
    float Res;
    BYTE  Chanel;
}SENSERXLFIX;


// ����Ч��
typedef struct __SENSERCXL
{
    BYTE C1;
    BYTE C2;
}SENSERCXL;



typedef struct __SENSER
{
    SENSERCONTXT  const  *Contxt;
    
    SENSERPARAM   const  *ParamDef;   //  Ĭ�ϲ���
    SENSERPARAM   const  *ParamMax;   //  �������ֵ
    SENSERPARAM   const  *ParamMin;   //  ������Сֵ
    
    SENSERPARAM    Param;
    SENSERALARM    Alarm;
    SENSERCOUNTER  Conter;
    BYTE           WorkSt;
    SENSERGND      Gnd;            //  ���ײ���ʱ��
    
    //SENSERXLFIX    XlFix;         // Ч�ʱ궨

    // ��������û��ʹ��
    //SENSERCXL      Cxl;           // ����Ч��
    //WORD           BaudRate;
}SENSER;

//��ص����ṹ��
typedef struct
{
	WORD  Voltage;		//��ص�ѹֵmv   	LP
	BYTE  percent;		//��ذٷֱ�%		LP
    BYTE  Status;      //0��������1����ص�ѹ��
}BATTERY;


BOOL HaveRadSet();
BOOL IsAscllCode();
BYTE OneThres();

void InitSenser();

BOOL HndRadDataBase();
BOOL ReConnectSenser();
BYTE FindSenser();
BOOL GetSenserParam();
BOOL GetSenserAlarm();
BYTE GetValidCounterLoc(BYTE Index);
float GetCounterVal(BYTE Index);
BOOL GetSenserCounter();
BOOL GetSenserWork();
BOOL SetSenserWork(BYTE St, BYTE Ch);
BOOL SetSenserParam();
BOOL SaveSenserParam();
float AscllToFloat(BYTE *Data);
float CanDataToFloat(BYTE *Data);
DWORD CanDataToDword(BYTE *Data);
BOOL SetPadXl(BYTE id, float xl);
void InterSsWorkSetUI();
void SsWorkSetKey(PRESSKEY key);
BOOL GetSenserXl(BYTE i);
BOOL SetRadActive(float val);
BOOL SenserXlFix(BYTE Id, float Val);
BOOL SetRadUnit(BYTE val);
BOOL SetSenserNuc(BYTE id);
BYTE CanWaitCmd(BYTE Cmd);
BYTE CanReadCmd(BYTE Cmd);
BOOL SetSenserAlarm();
BOOL SenserBackTest();
BOOL SenserSendByte(BYTE Cmd, BYTE Index, BYTE val);
BYTE SenserSendFloat(BYTE Cmd, BYTE Index, float val, BYTE pc);
BYTE SenserSendFloatEx(BYTE Cmd, BYTE Index, float val, BYTE pc);
BYTE GetSenserIndex(BYTE type);
BOOL SetAlarmThres(float val);
BYTE GetThresId();

BOOL BTClearMax();
BOOL BTClearTotol();
BOOL BTSenserPower(WORD On);
BOOL BTGetBattary(BYTE Retry);
BOOL BTGmSw(BYTE Gm);

void CanSendCmd(BYTE Cmd, BYTE Addr, BYTE Len);

#endif

