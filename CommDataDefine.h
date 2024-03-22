#ifndef COMMDATADEFINE_H
#define COMMDATADEFINE_H

#ifdef __cplusplus
extern  "C" {
#endif


//��˾logo ����
#define COM_NONE   0
#define COM_MW     1
#define COM_MTS    2
#define XI_AN_262  3
#define XI_AN_HLT  4  //  ����������


//#define UNIT_GY    // ��λ��Ϊ Gy

//#define NO_BT_WIFI    // ��������Wifi -- �ڲ���GM��  // S2B

//#define FOR_TEMP              // ��ʱ������
//#define LOG_IN_INNER_FALSH
//#define DATA_IN_INNER_FALSH   // �������ݷ����ⲿE2
#define INIT_OPT_LOC     // �˵�����ʱ�����ϴβ���λ��
#define USE_IDLE         // ����idle ģʽ
//#define PWM_BEEP
#define TIMERB_DOSE    // �ö�ʱ���Ƽ��� 
#define DOUBLE_DELAY   // ˫��ʱ����
//#define USE_WIFI
//#define USE_LP_DOSECALC   // ʹ�ó��˼������㷨 
//#define UART_DOSE         // ���ڴ�ӡ���� -- ���˻ᵼ�¹�������

#define BATTERY_VER         //  ʹ�õ�أ������

#ifdef BATTERY_VER
#define NEW_KEYBOARD      // �°���(5��)
#endif

//#define DG105               // ר��

#ifdef DEBUG
//#define UART_DEBUG
//#define DEBUG_BAT
#endif



#define BYTE unsigned char
#define WORD unsigned int
#define DWORD unsigned long

#define BOOL BYTE
#define TRUE 1
#define FALSE 0

#define BK_CPS  0.2  // ���׼���
#define PAGE_MENUCOUNT  6  // ÿҳ��ʾ�˵�����
//#define System_Freq  25000000    //���ں��ĵ�ѹΪ2.5V������Ƶ�����ֻ������Ϊ4MHz


//#define setbit(x,y) x|=(1<<y)		//��X�ĵ�Yλ��1
//#define clrbit(x,y) x&=~(1<<y)		//��X�ĵ�Yλ��0  
  
//#define UPKEY       (P1IN & BIT5)
//#define DOWNKEY     (P1IN & BIT4)

// ������λ����
#define UNIT_CPS      1
#define UNIT_Bq       2
#define UNIT_Bq_cm2   3
#define UNIT_uSv_h    4
#define UNIT_mSv_h    5
#define UNIT_Sv_h     6
#define UNIT_uSv      7
#define UNIT_mSv      8
#define UNIT_Sv       9

#define DATA_AV      1   // ƽ��ֵ
#define DATA_MAX     2   // ���ֵ
#define DATA_RT      3   // ʵʱֵ

#define TRG_IN    1
#define TRG_OUT   2
#define TRG_ALL   3

#define DEV_MPRP_S2N    1
#define DEV_MPRP_S2D    2    // ˫GM��
#define DEV_MPRP_S3    3
#define DEV_MPRP_S2L    4    // ��GM��
#define DEV_MPRP_S5    5


// ����ͨ������
#define CHANNEL_MIX   0     // ��
#define CHANNEL_A     1     // ��
#define CHANNEL_B     2     // ��

#define MIX  1     // ��ϳ�������������
#define NOR  0

// �Զ��帡��(16bit) �޷���
// ��������10bit(0..999) ��С������6bit  -- 2^(e-63)
typedef union
{
    //float fVal;
    WORD  wVal;
    struct
    {
        WORD m:8;
        WORD e:7;
        WORD s:1;
    };
}tiny;

typedef union
{
    float fVal;
    DWORD dwVal;
    struct
    {
        DWORD m:23;
        DWORD e:8;
        DWORD s:1;
    };
}mfloat;



  
typedef enum _PRESSKEY_
{
    //NONE=1,UP,DOWN,OKKEY,MODE,RETURN,TIMEOUT
    #ifdef NEW_KEYBOARD
    NONE=1,OKKEY,RETURN,MODE,DOWN,UP,TIMEOUT,POWER
    #else
    NONE=1,RETURN,MODE,OKKEY,DOWN,UP,TIMEOUT,POWER
    #endif
}PRESSKEY;


typedef enum _INTERFACE_
{
   NULLUI=1,       MEASU,       QUICK,         BACK,          PASSWD,
   MAIN,           METERSET,    METERMODE,    BACKSET,       DATASTORE,
   CHGNPSW,        MODSYSTM,    MENURTN,      RADLIST,       DETASET,
   VIEWSTORE,      DETAIL,      OTHPARASET,   HIGBOTTOM,    ALARMTHRE,
   DATAMAN,        DATASET,     HIGVOLT,      DISCRI,        ADISCRI,
   BDISCRI,        ABDISCRI,    ALARMPARAM,   EFFIDEMAR,    DEMARUI,
   PARAMSET,       UNITSEL,     VIEWDATA,     BYTIMEVIEW,   SHOWBYTIME,
   DETAILBYTIME,  RADTHRED,    RADISTEST,    LOGO,          SELFCHECK,
   SSPARAM,        INBYTE,      INWORD,       INDWORD,       INFLOAT,
   SSALARM,        HINT,        SENSERWORK,   DEMARRES,      TIMESEL,
   DEMARING,       BACKTEST,    METERTIME,    DATADEL,      VIEWALLLOG,
   OUTSS,          INSS,         COMFIRM,      INFLOATEX,    SELUNIT, 
   INFLOATUNIT,   ELECTEST,     PHYTEST,      SELRADUNIT,   METERTYPE,
   METERMAN,      INTEST,       TRIGERTYPE,   DEVTYPE,       WIFIMAN,
   WIFISTATUS,    WIFIMODE,     WIFIAP,       WIFISRV,       WIFIDETAIL,
   INCNTFACT,     METERVIEW,    SHOWALM,      BLUETOOTHSET, BTSRV,
   RIGHT,          MEASRES,     COMLOG
}INTERFACE;


typedef void (*ReturnFun)();
typedef void (*EventFun)(PRESSKEY);
typedef struct __FACEHANDLE
{
    INTERFACE UiId;
    EventFun  Ef;   // �¼�������
    WORD      Rt;   // ���泬ʱʱ��ms, 0 ��ʾ���ݲ������õ�ʱ�䳬ʱ����
                    // 0xFFFF ��ʾ����ʱ����Ҫ���⴦��
}FACEHANDLE;


#include "Senser.h"
#include "Wifi.h"

typedef struct _RADIO_
{
    char Valid;        // �Ƿ���Ч
    char index;          //��������
    char Channel;       //����Ƶ��  0x01  alpha, 0x02 beta&r,  0x00 MIX
    char Unit;          //��λ
    
    float AlarmThres;     //������ֵ(CPS)�� ��ʾ������ʱ���ݲ�ͬ��λ����
    float Efficiency;     //Ч��
    float Active;         //�����Ӧ
    //char  Name[8];
    
}RADIO;



// ����������Ϣ
typedef struct
{
    float OutA_Cps_Rt;    // ʵʱֵ
    float OutA_Cps_Max;   // ���ֵ
    //float OutA_Cps_Av;    // ƽ��ֵ
    //float OutA_Cps_Total;     
    
    float OutB_Cps_Rt;
    float OutB_Cps_Max;
    //float OutB_Cps_Av;

    //float OutC_Cps_Rt;
    //float OutC_Cps_Max;
    //float OutC_Cps_Av;
    
    float OutA_Cur_Rt;   // �ⲿ̽ͷaʵʱ����ֵ���Ѿ�ת����λ
    float OutA_Cur_Max;
    //float OutA_Cur_Av;
    float OutA_Cur_Total;  
    
    float OutB_Cur_Rt;   // �ⲿ̽ͷbʵʱ����ֵ���Ѿ�ת����λ
    float OutB_Cur_Max;
    //float OutB_Cur_Av;
    float OutB_Cur_Total;  

    float In_Cps;
    float In_Rt;      //   �ڲ�̽ͷʵʱ����ֵ���Ѿ�ת����λΪ uSv/h
    float In_Max;
    //float In_Av;
    float In_Total;          // �ۼƼ���
    
    
    //BYTE RcDown;      //ʵʱ�����������ްٷֱ�
    //BYTE RcUp;        //ʵʱ�����������ްٷֱ�
    //BYTE RcCurr;     //ʵʱ�����ٷֱ�

    BYTE Gm;         // ����״̬  0 : ������ 1 : ������

    // ��ǰ��ʾֵ
    float Counter;
    BYTE  Unit;
}METEFACEINFO;




// �ⲿEEPROM ����
typedef struct _PARA_STRUCT
{
    BYTE ucCheck;          //����У��λ
    
    //���ݴ洢
    char AlarmStore;    // 1 Yes 0 No           //�����洢
    char ActiveMode;    // 1 Yes 0 No            //�Զ��洢
    char MeteTime;                       //�洢ʱ��
    char DataType;       // �Զ��洢��������
    char TrigerType;     // ������������

    //BYTE  RadCount;     // ��������
    //RADIO pnuc[25];      //����

    BYTE ucStartBlock;   //��ʼ��---- û��ʹ��
    
    SENSER    InSenser;  // �ڲ�̽ͷ
    SENSERGND Gnd;        // �ⲿ̽ͷ����

    #ifdef USE_WIFI
    WIFIPARAM Wifi;       // Wifi ����
    #endif

    //METEFACEINFO  MetInfo;   // ������Ϣ
}QPara;


// ������CPU �ڲ�Flash �Ĳ��� 
typedef struct
{
    BYTE   ucCheck;                  //����У��λ
    BOOL   Sound;                    //����������   1 ����0 ��
    BOOL   Keytone;                 //������
    BOOL   pisScreen;               //��Ļ����  -- ��Ϊ����
    
    WORD   PassWord;               //����
    WORD   StorageNum;             //�洢��������
    WORD   LastData; 
    WORD   LogNum;                 // ��־����
    WORD   LastLog; 
    WORD   BqXs;                   // Bq/cm2 ת��ϵ��
    
    // �ڲ�̽ͷ����У׼����
    float  La;  // ������
    float  Lb;
    float  Lc;  
    float  Ha;  // ������
    float  Hb;
    float  Hc;  

    float  Ya;  // �ⲿNaI  DevType == 4 ʱ
    float  Yb;
    float  Yc;

    WORD   Fudu;
    //BYTE   ZhuAlarmRate; 
    BYTE   CurrSenserType;        // ����ʹ�õ��ⲿ̽ͷ����
    BYTE   RadioSelIndex;         // ��ǰ����
    BYTE   MeauExitTime;           //�˵��˳�ʱ�� :  ��λ�� 0 ��ʾ���˳�
    BYTE   BkTimeout;              //���ⳬʱʱ�䵥λ: ��
    BYTE   DevType;                // �豸����
    BOOL   InnerSersor;           // �Ƿ����ڲ�̽����  -- 2019.9.6 add

    float  DoseMax;            //  ������
    float  DoseTotal;          // �ۼƼ���
    
    char   BtAddress[14];         // ����Ŀ���ַ
    BYTE   BtPower;                // ��������
    BYTE   MeasTime;
    BYTE   ComLog;                 
}QInnerParam;


// ������־  8 �ֽ�
typedef struct 
{
    unsigned char ucCheck;         // У����
    unsigned char LogId;
    
    //����ʱ�䣬���ô洢����
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    
}SYSLOG;



/************************************************************************
    ������¼��16�ֽ�
    ˵��: Ϊ�˱�֤ÿ����¼16�ֽ�

    �����ʴ洢��ʽΪ3�ֽ�
    Value1 �� Ext1�����λ����������� 0 -- 99999 
                  ��������65535ʱ�Ѹ�λ�����Ext1�����λ
    Ext1�ĵ�7λ���С������ 0 -- 99
    
    ��: 456.78 :  Value1 = 456   Ext1 = 78
    ��: 99999.99(99999=0x1869F): Value1 = 0x869F;  Ext1(Bit7) = 1;  Ext1(Bit0--6)=99

    Unit: ���3����ֵ�ĵ�λ��ÿ����λռ2λ
       Bit0-1: Value2 �ĵ�λ
       Bit2-3: Value1 �ĵ�λ
       Bit4-5: AlarmValue �ĵ�λ
*************************************************************************/
typedef struct _Storage_Data_
{
    unsigned char ucCheck;            // У����

    unsigned char NucName;           //��������
    unsigned int  AlarmValue;        //����ֵ-- ��ȡ��������
    unsigned int  Value1;            //���̽ͷ����ֵ
    unsigned int  Value2;            //�ڲ�̽ͷ����ֵ
    unsigned char Ext1;              //ת���������λ
    unsigned char Ext2;
    unsigned char Unit;              //��λ: Ҫ��¼����ֵ�ĵ�λÿ����λ2bit
    unsigned char Channel;           //����Ƶ��-- ̽ͷ����
    
    //����ʱ�䣬���ô洢����ѹ����4���ֽ�
    #if 0
    unsigned char year;   // 99 - 7BIT
    unsigned char month;  // 12 - 4BIT  
    unsigned char day;    // 31 - 5BIT
    unsigned char hour;   // 23 - 5BIT
    unsigned char minute; // 59 - 6BIT
    unsigned char second; // 59 - 6BIT
    //unsigned char resv; // �������������ݶ���
    #else
    unsigned long Time;
    #endif
}QStorageData;


// ʱ��ѹ��
typedef union
{
    unsigned long Time;
    struct
    {
        unsigned long y:6;    // ��0 -- 63  -- ��ʱֻ֧�ֵ�2063��
        unsigned long m:4;    // ��1 -- 12
        unsigned long d:5;    // ��1 -- 31
        unsigned long h:5;    // ʱ0 -- 23
        unsigned long t:6;    // ��0 -- 59
        unsigned long s:6;    // ��0 -- 59
    };
}CMPTIME;


typedef void (*ONFUN)(void);
typedef void (*OFFFUN)(void);
typedef struct __FLASH_IO
{
    BOOL Enable;
    BOOL Status;
    WORD OnTime;
    WORD OffTime;
    WORD Count;
    ONFUN On;
    OFFFUN Off;
}FLASH_IO;

typedef struct __LOGICTIMER
{
    BOOL Enable;
    WORD Counter;
}LOGICTIMER;



tiny FloatToTiny(float val);
float TinyToFloat(tiny val);
float CpsToUsv_h(float cps);
float CpsUnit(float cps, BYTE unit);
float ToCps(float val, BYTE unit);
DWORD CounterToCps(DWORD Cnt);
  
#ifdef __cplusplus
}
#endif
#endif  

