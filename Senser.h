#ifndef __SENSER_H__
#define __SENSER_H__


#define SENSERCOUNT 15

// 传感器类型定义
#define SENSER_MASTER     0       // 中心
#define SENSER_IN         0x01    // 内置探头  
#define SENSER_PD         0x02    //水下高量程探头（PD）
#define SENSER_GM         0x03    //水下中高量程探头（GM）
#define SENSER_A_A        0x04    //α探头(仅测alphy)
#define SENSER_A_B        0x14    //α探头(仅测beta)
#define SENSER_A_AB       0x24    //α探头(可测alphy+beta)
#define SENSER_B_B        0x05    //β探头(仅测beta)
#define SENSER_B_A        0x15    //β探头(仅测alphy)
#define SENSER_B_AB       0x25    //β探头(可测alphy+beta)
#define SENSER_ABY        0x06    // αβγ探头
#define SENSER_G1         0x07    //地面探头1
#define SENSER_G2         0x08    //地面探头2
#define SENSER_Y          0x09    //外置Y 探头 ( NaI 探头 )
#define SENSER_LP         0x2A    // 长杆Y 探头    (蓝牙)
#define SENSER_LC         0x0B    // 液体辐射探头  (蓝牙)
#define SENSER_BC         0xFF    // 广播
#define SENSER_NONE       0xFE    // 没有外接传感器


//传感器命令定义
#define SCMD_COUNT           15      // 传感器命令个数
#define SCMD_FIND_SENSER    0x43    // C 支持广播
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

// 下面是蓝牙新增的命令
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
    BYTE RetFrameCnt;    // 返回包的数量, 0 表示超时返回
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
    BYTE Type;                      // 传感器类型
    BYTE ParmCnt;                  // 参数个数
    BOOL ParamLoc[PARAMCOUNT];     // 参数位置
    BYTE CounterCnt;               // 计数器的个数
    BOOL CounterLoc[COUNTERCOUNT]; // 计数器的位置
    BYTE AlarmCnt;                 // 报警阀值个数
    BOOL AlarmLoc[ALARMCOUNT];     // 报警阀值位置
    BYTE AlarmId[10];               // 计数1使用的报警ID
    BYTE AlarmId2[10];              // 计数2使用的报警ID
}SENSERCONTXT;

#pragma pack(2)
typedef struct __SENSERPARAM
{
    float  Hv;  // 0x01+高压值
    float  Z1;  // 0x02+甄别器阈值1
    float  Ct;  // 0x03+计数时间
    float  Hd;  // 0x04+高压误
    float  Z2;  // 0x05+甄别器阈值2
    float  Z3;  // 0x06+甄别器阈值3
    float  S1;  // 0x07+探测器修正系数1
    float  S2;  // 0x08+探测器修正系数2
    float  Cr;  // 0x09+校准因子
    BYTE   Hn;  // 0x0A+启用核素号           ---  9
    float  S3;  // 0x0B 探测器修正系数3
    float  Z4;  // 0x0C 甄别器阈值4

    float Pt;   // 0x0D 平滑时间
    float LA;   // 0x0E 低量程通道校准因子A    LP
    float LB;   // 0x0F 低量程通道校准因子B    LP
    float LC;   // 0x10 低量程通道校准因子C    LP
    float HA;   // 0x11高量程通道校准因子A    LP
    float HB;   // 0x12 高量程通道校准因子B   LP
    float HC;   // 0x13 高量程通道校准因子C   LP

    float Ss;   // 0x14  探测器面积  -- 2017.9.28 add
    BYTE  Bk;   // 0x15  是否扣除本底   -- 2018.9.13            ---- 20
    
}SENSERPARAM;



typedef struct __SENSERCOUNTER
{
    float  Aj;  BYTE AjSt;   //0x01+正在调整
    float  Hv;  BYTE HvSt;   //0x02+高压故障
    float  C1;  BYTE C1St;   // 0x03+计数值1+报警状态（1B）（α计数）
    float  C2;  BYTE C2St;   //0x04+计数值2+报警状态（1B）（β计数）
    float  C3;  BYTE C3St;   // 0x05+计数值3+报警状态（1B）(α+β)
    float  P1;  BYTE P1St;   //0x06+平滑计数值1+报警状态（1B）（α计数）
    float  P2;  BYTE P2St;   //0x07+平滑计数值2+报警状态（1B）（α计数）
    float  P3;  BYTE P3St;   //0x08+平滑计数值3+报警状态（1B）（α计数）
    float  Cr;  BYTE CrSt;   // 0x09+剂量率+报警状态（1B）uSv/h   Y
    float  Tc;  BYTE TcSt;   // 0x0A+累计剂量+报警状态（1B）uSv   Y
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
    float  A1;  // 0x01+一级报警阈值(α)
    float  A2;  //0x02+二级报警阈值(α)
    float  Al;  // 0x03+低本底报警阈值(α)
    float  Ah;  //0x04+高本底报警阈值(α)
    float  B1;  // 0x05+一级报警阈值(β)
    float  B2;  //0x06+二级报警阈值(β)
    float  Bl;  // 0x07+低本底报警阈值(β)
    float  Bh;  //0x08+高本底报警阈值(β)
    float  Ac;  // 0x09+alphy核素报警值
    float  Bc;  //0x0A+beta核素报警值
    float  Y1;  //0x0B Y剂量率一级报警(uSv/h)
    float  Y2;  //0x0C Y剂量率二级报警(uSv/h)
    float  Y3;  //0x0D Y剂量率三级报警(uSv/h)
    float  Ma;  //0x0E 累计剂量预警阀值
    float  Mx;  //0x0F 累计剂量报警阀值
    BYTE   Zu;  //0x10 内部累计剂量柱报警
}SENSERALARM;
#pragma pack()



typedef struct __SENSERWORKST
{
    BYTE ESt;
    BYTE PSt;
    BYTE CSt;
}SENSERWORKST;

//     本底
typedef struct __SENSERGND
{
    BOOL  Valid;      // 当前是否有本地测试
    BYTE  TestTime;
    float AVal;       // alerfa  本地值
    float BVal;       // bata   本地值
    BYTE  Res;       // 测量结束1
}SENSERGND;

// 效率标定
typedef struct __SENSERXLFIX
{
    float FixTime;   // 标定时长
    float SrcVal;    // 源活度
    float Res;
    BYTE  Chanel;
}SENSERXLFIX;


// 核素效率
typedef struct __SENSERCXL
{
    BYTE C1;
    BYTE C2;
}SENSERCXL;



typedef struct __SENSER
{
    SENSERCONTXT  const  *Contxt;
    
    SENSERPARAM   const  *ParamDef;   //  默认参数
    SENSERPARAM   const  *ParamMax;   //  参数最大值
    SENSERPARAM   const  *ParamMin;   //  参数最小值
    
    SENSERPARAM    Param;
    SENSERALARM    Alarm;
    SENSERCOUNTER  Conter;
    BYTE           WorkSt;
    SENSERGND      Gnd;            //  本底测量时间
    
    //SENSERXLFIX    XlFix;         // 效率标定

    // 下面两个没有使用
    //SENSERCXL      Cxl;           // 核素效率
    //WORD           BaudRate;
}SENSER;

//电池电量结构体
typedef struct
{
	WORD  Voltage;		//电池电压值mv   	LP
	BYTE  percent;		//电池百分比%		LP
    BYTE  Status;      //0：正常，1：电池电压低
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

