#ifndef COMMDATADEFINE_H
#define COMMDATADEFINE_H

#ifdef __cplusplus
extern  "C" {
#endif


//公司logo 定义
#define COM_NONE   0
#define COM_MW     1
#define COM_MTS    2
#define XI_AN_262  3
#define XI_AN_HLT  4  //  西安海利特


//#define UNIT_GY    // 单位改为 Gy

//#define NO_BT_WIFI    // 无蓝牙和Wifi -- 内部带GM管  // S2B

//#define FOR_TEMP              // 临时测试用
//#define LOG_IN_INNER_FALSH
//#define DATA_IN_INNER_FALSH   // 测试数据放在外部E2
#define INIT_OPT_LOC     // 菜单操作时保留上次操作位置
#define USE_IDLE         // 启用idle 模式
//#define PWM_BEEP
#define TIMERB_DOSE    // 用定时器计剂量 
#define DOUBLE_DELAY   // 双击时间间隔
//#define USE_WIFI
//#define USE_LP_DOSECALC   // 使用长杆剂量率算法 
//#define UART_DOSE         // 串口打印剂量 -- 关了会导致功耗上升

#define BATTERY_VER         //  使用电池，不充电

#ifdef BATTERY_VER
#define NEW_KEYBOARD      // 新按键(5键)
#endif

//#define DG105               // 专用

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

#define BK_CPS  0.2  // 本底计数
#define PAGE_MENUCOUNT  6  // 每页显示菜单数量
//#define System_Freq  25000000    //由于核心电压为2.5V，所以频率最大只能设置为4MHz


//#define setbit(x,y) x|=(1<<y)		//将X的第Y位置1
//#define clrbit(x,y) x&=~(1<<y)		//将X的第Y位清0  
  
//#define UPKEY       (P1IN & BIT5)
//#define DOWNKEY     (P1IN & BIT4)

// 剂量单位定义
#define UNIT_CPS      1
#define UNIT_Bq       2
#define UNIT_Bq_cm2   3
#define UNIT_uSv_h    4
#define UNIT_mSv_h    5
#define UNIT_Sv_h     6
#define UNIT_uSv      7
#define UNIT_mSv      8
#define UNIT_Sv       9

#define DATA_AV      1   // 平均值
#define DATA_MAX     2   // 最大值
#define DATA_RT      3   // 实时值

#define TRG_IN    1
#define TRG_OUT   2
#define TRG_ALL   3

#define DEV_MPRP_S2N    1
#define DEV_MPRP_S2D    2    // 双GM管
#define DEV_MPRP_S3    3
#define DEV_MPRP_S2L    4    // 单GM管
#define DEV_MPRP_S5    5


// 核素通道类型
#define CHANNEL_MIX   0     // α
#define CHANNEL_A     1     // α
#define CHANNEL_B     2     // β

#define MIX  1     // 混合场报警参数设置
#define NOR  0

// 自定义浮点(16bit) 无符号
// 整数部分10bit(0..999) ，小数部分6bit  -- 2^(e-63)
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
    EventFun  Ef;   // 事件处理函数
    WORD      Rt;   // 界面超时时间ms, 0 表示根据参数设置的时间超时返回
                    // 0xFFFF 表示不超时，需要特殊处理
}FACEHANDLE;


#include "Senser.h"
#include "Wifi.h"

typedef struct _RADIO_
{
    char Valid;        // 是否有效
    char index;          //核素名称
    char Channel;       //核素频道  0x01  alpha, 0x02 beta&r,  0x00 MIX
    char Unit;          //单位
    
    float AlarmThres;     //报警阈值(CPS)， 显示或输入时根据不同单位换算
    float Efficiency;     //效率
    float Active;         //活度响应
    //char  Name[8];
    
}RADIO;



// 测量界面信息
typedef struct
{
    float OutA_Cps_Rt;    // 实时值
    float OutA_Cps_Max;   // 最大值
    //float OutA_Cps_Av;    // 平均值
    //float OutA_Cps_Total;     
    
    float OutB_Cps_Rt;
    float OutB_Cps_Max;
    //float OutB_Cps_Av;

    //float OutC_Cps_Rt;
    //float OutC_Cps_Max;
    //float OutC_Cps_Av;
    
    float OutA_Cur_Rt;   // 外部探头a实时剂量值，已经转换单位
    float OutA_Cur_Max;
    //float OutA_Cur_Av;
    float OutA_Cur_Total;  
    
    float OutB_Cur_Rt;   // 外部探头b实时剂量值，已经转换单位
    float OutB_Cur_Max;
    //float OutB_Cur_Av;
    float OutB_Cur_Total;  

    float In_Cps;
    float In_Rt;      //   内部探头实时剂量值，已经转换单位为 uSv/h
    float In_Max;
    //float In_Av;
    float In_Total;          // 累计剂量
    
    
    //BYTE RcDown;      //实时剂量报警下限百分比
    //BYTE RcUp;        //实时剂量报警上限百分比
    //BYTE RcCurr;     //实时剂量百分比

    BYTE Gm;         // 量程状态  0 : 低量程 1 : 高量程

    // 当前显示值
    float Counter;
    BYTE  Unit;
}METEFACEINFO;




// 外部EEPROM 参数
typedef struct _PARA_STRUCT
{
    BYTE ucCheck;          //参数校验位
    
    //数据存储
    char AlarmStore;    // 1 Yes 0 No           //报警存储
    char ActiveMode;    // 1 Yes 0 No            //自动存储
    char MeteTime;                       //存储时间
    char DataType;       // 自动存储数据类型
    char TrigerType;     // 报警触发类型

    //BYTE  RadCount;     // 核素数量
    //RADIO pnuc[25];      //核素

    BYTE ucStartBlock;   //起始块---- 没有使用
    
    SENSER    InSenser;  // 内部探头
    SENSERGND Gnd;        // 外部探头本底

    #ifdef USE_WIFI
    WIFIPARAM Wifi;       // Wifi 参数
    #endif

    //METEFACEINFO  MetInfo;   // 测量信息
}QPara;


// 保存在CPU 内部Flash 的参数 
typedef struct
{
    BYTE   ucCheck;                  //参数校验位
    BOOL   Sound;                    //报警音开关   1 开，0 关
    BOOL   Keytone;                 //按键音
    BOOL   pisScreen;               //屏幕保护  -- 改为背光
    
    WORD   PassWord;               //密码
    WORD   StorageNum;             //存储数据数量
    WORD   LastData; 
    WORD   LogNum;                 // 日志数量
    WORD   LastLog; 
    WORD   BqXs;                   // Bq/cm2 转换系数
    
    // 内部探头计数校准因子
    float  La;  // 低量程
    float  Lb;
    float  Lc;  
    float  Ha;  // 高量程
    float  Hb;
    float  Hc;  

    float  Ya;  // 外部NaI  DevType == 4 时
    float  Yb;
    float  Yc;

    WORD   Fudu;
    //BYTE   ZhuAlarmRate; 
    BYTE   CurrSenserType;        // 正在使用的外部探头类型
    BYTE   RadioSelIndex;         // 当前核素
    BYTE   MeauExitTime;           //菜单退出时间 :  单位秒 0 表示不退出
    BYTE   BkTimeout;              //背光超时时间单位: 秒
    BYTE   DevType;                // 设备类型
    BOOL   InnerSersor;           // 是否有内部探测器  -- 2019.9.6 add

    float  DoseMax;            //  最大剂量
    float  DoseTotal;          // 累计剂量
    
    char   BtAddress[14];         // 蓝牙目标地址
    BYTE   BtPower;                // 蓝牙功率
    BYTE   MeasTime;
    BYTE   ComLog;                 
}QInnerParam;


// 操作日志  8 字节
typedef struct 
{
    unsigned char ucCheck;         // 校验码
    unsigned char LogId;
    
    //测量时间，不用存储星期
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    
}SYSLOG;



/************************************************************************
    测量记录共16字节
    说明: 为了保证每条记录16字节

    剂量率存储方式为3字节
    Value1 和 Ext1的最高位存放整数部分 0 -- 99999 
                  计数大于65535时把高位存放在Ext1的最高位
    Ext1的低7位存放小数部分 0 -- 99
    
    如: 456.78 :  Value1 = 456   Ext1 = 78
    如: 99999.99(99999=0x1869F): Value1 = 0x869F;  Ext1(Bit7) = 1;  Ext1(Bit0--6)=99

    Unit: 存放3个数值的单位，每个单位占2位
       Bit0-1: Value2 的单位
       Bit2-3: Value1 的单位
       Bit4-5: AlarmValue 的单位
*************************************************************************/
typedef struct _Storage_Data_
{
    unsigned char ucCheck;            // 校验码

    unsigned char NucName;           //核素名称
    unsigned int  AlarmValue;        //报警值-- 自取整数部分
    unsigned int  Value1;            //外接探头测量值
    unsigned int  Value2;            //内部探头测量值
    unsigned char Ext1;              //转浮点后多出的位
    unsigned char Ext2;
    unsigned char Unit;              //单位: 要记录三个值的单位每个单位2bit
    unsigned char Channel;           //测量频道-- 探头类型
    
    //测量时间，不用存储星期压缩到4个字节
    #if 0
    unsigned char year;   // 99 - 7BIT
    unsigned char month;  // 12 - 4BIT  
    unsigned char day;    // 31 - 5BIT
    unsigned char hour;   // 23 - 5BIT
    unsigned char minute; // 59 - 6BIT
    unsigned char second; // 59 - 6BIT
    //unsigned char resv; // 保留，用于数据对齐
    #else
    unsigned long Time;
    #endif
}QStorageData;


// 时间压缩
typedef union
{
    unsigned long Time;
    struct
    {
        unsigned long y:6;    // 年0 -- 63  -- 暂时只支持到2063年
        unsigned long m:4;    // 月1 -- 12
        unsigned long d:5;    // 日1 -- 31
        unsigned long h:5;    // 时0 -- 23
        unsigned long t:6;    // 分0 -- 59
        unsigned long s:6;    // 秒0 -- 59
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

