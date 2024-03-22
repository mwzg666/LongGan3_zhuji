#ifndef SYSCONFIG_H
#define SYSCONFIG_H

#ifdef __cplusplus
extern  "C" {
#endif

#include "CommDataDefine.h"


// GPIO Define
#ifdef BATTERY_VER
#define DC_IN      (1)    // (P1IN & BIT1)
#define CHARGING  (1)  //(P6IN & BIT1)
#else
#define DC_IN     (P1IN & BIT1)
#define CHARGING  (P6IN & BIT1)
#endif

#ifndef BATTERY_VER
#define HVL_OFF    P3OUT &= ~BIT2
#define HVH_OFF    P7OUT &= ~BIT3
#define HVL_ON     P3OUT |= BIT2
#define HVH_ON     P7OUT |= BIT3
#else
#define BT_ON     P7OUT |= BIT3
#define BT_OFF    P7OUT &= ~BIT3
#endif

#define P8V_ON     P3OUT |= BIT5         //  外部探头供电
#define P8V_OFF   P3OUT &= ~BIT5
#define PW_ON     P6OUT |= BIT0
#define PW_OFF    P6OUT &= ~BIT0
#define BK_ON     P4OUT |= BIT7 
#define BK_OFF    P4OUT &= ~BIT7 
#define UART_OFF  P5OUT &= ~BIT0
#define UART_ON   P5OUT |= BIT0

#ifdef BATTERY_VER
#define GM_HIGH   {P2OUT |= BIT7; delay_ms(10); P2OUT &= ~BIT7;}      // GM Power  ---  高低量程切换   1:高
#define GM_LOW    {P3OUT |= BIT2; delay_ms(10); P3OUT &= ~BIT2;}     // 0 : 低
#else
#define GM_HIGH   P2OUT |= BIT7      // GM Power  ---  高低量程切换   1:高
#define GM_LOW    P2OUT &= ~BIT7     // 0 : 低
#endif

#define WIFI_ON   P8OUT |=  BIT1
#define WIFI_OFF  P8OUT &= ~BIT1

#define OVL    0   // 过载
#define NO1    1   // 一级报警
#define NO2    2   //  二级报警
#define BGL    3   // 低本底报警
#define BGH    4   // 高本底报警
#define LE     5   // 长杆低通道异常
#define HE     6   // 长杆高通道异常
#define DE     7   // 长杆双通道异常

// 报警定义，优先级高的报警放在前面
#define ALARM_AO   0    // 阿尔法过载报警
#define ALARM_A1   1    // 阿尔法一级报警
#define ALARM_A2   2    // 阿尔法二级报警
#define ALARM_AL   3     //阿尔法 低本底报警
#define ALARM_AH   4     //阿尔法 高本底报警

#define ALARM_BO   5    // 贝塔过载报警
#define ALARM_B1   6    // 贝塔一级报警
#define ALARM_B2   7    // 贝塔二级报警
#define ALARM_BL   8    // 贝塔低本底报警
#define ALARM_BH   9    // 贝塔高本底报警

#define ALARM_ABO   10    // 阿尔法+贝塔过载报警
#define ALARM_AB1   11    // 阿尔法+贝塔一级报警
#define ALARM_AB2   12    // 阿尔法+贝塔二级报警
#define ALARM_ABL   13    // 阿尔法+贝塔低本底报警
#define ALARM_ABH   14    // 阿尔法+贝塔高本底报警


//水下PD
#define ALARM_PDO   15    // 过载报警
#define ALARM_PD1   16    // 一级报警
#define ALARM_PD2   17    // 二级报警
#define ALARM_PDL   18    // 低本底报警
#define ALARM_PDH   19    // 高本底报警


//水下GM
#define ALARM_GMO   20    // 过载报警
#define ALARM_GM1   21    // 一级报警
#define ALARM_GM2   22    // 二级报警
#define ALARM_GML   23    // 低本底报警
#define ALARM_GMH   24    // 高本底报警


// 地面
#define ALARM_GDO   25    // 过载报警
#define ALARM_GD1   26    // 一级报警
#define ALARM_GD2   27    // 二级报警
#define ALARM_GDL   28    // 低本底报警
#define ALARM_GDH   29    // 高本底报警

// 外部Y
#define ALARM_YO   30    // 过载报警
#define ALARM_Y1   31    // 剂量预警
#define ALARM_Y2   32    // 剂量报警
#define ALARM_YL   33    // 累计剂量预警
#define ALARM_YH   34    // 累计剂量报警


// 内部探头
#define ALARM_INO   35    // 过载报警
#define ALARM_IN1   36    // 一级报警
#define ALARM_IN2   37    // 二级报警
#define ALARM_IN3   38    // 三级报警
#define ALARM_INH   39    // 高本底报警
#define ALARM_INL    40   // 低本底报警
#define ALARM_YP     41   // 累计预警
#define ALARM_YT     42   //  总剂量超标



// 下面两个没有使用
#define ALARM_AR    43    // 阿尔法核素报警
#define ALARM_BR    44    // 贝塔核素报警

// 长杆增加了三个报警
#define ALARM_LE   45    // 长杆低通道异常
#define ALARM_HE   46    // 长杆高通道异常
#define ALARM_DE   47    // 长杆双通道异常


#define ALARM_RES1    48    // 阿尔法核素报警
#define ALARM_RES2    49    // 贝塔核素报警


// 系统报警
#define ALARM_BAT_LOW      50    // 电池电压低
#define ALARM_HV_ERR       51    // 高压异常
#define ALARM_RTC_ERR      52    // 实时钟异常
#define ALARM_SENSER_ERR   53    // 探头通讯异常
#define ALARM_EEPROM_ERR   54    // EEPROM 异常

#define ALARM_STOREAGE_FULL  55    // 数据区满
#define ALARM_LOG_FULL        56
#define ALARM_STATUS_ERR      57   // 探头状态异常
#define ALARM_INSENSER_ERR    58   // 内部探头异常
#define ALARM_SENSER_BATERY   59   // 探头电压低

#define ALARM_COUNT  60


// SysLog Define  
#define EVENT_STARTUP       1     // 开机
#define EVENT_SHUTDOWN      2     // 关机
#define EVENT_USER_LOGIN    3     // 用户登录
#define EVENT_ADMIN_LOGIN   4     // 管理员登录

#define EVENT_MOD_INPARAM   5     // 修改内部探头参数
#define EVENT_MOD_OUTPARAM   6    // 修改外部探头参数
#define EVENT_MOD_INALARM    7    // 修改内部探头报警
#define EVENT_MOD_OUTALARM   8    // 修改外部探头报警
#define EVENT_DEF_PARAM      9      // 使用默认参数

#define EVENT_DEL_DATA      10     // 删除数据
#define EVENT_DEL_LOG       11     // 删除日志


// 自检事件
#define EVENT_BAT_LOW       12
#define EVENT_HV_ERR        13
#define EVENT_RTC_ERR       14
#define EVENT_E2_ERR        15

#define EVENT_R1            16   // 保留
#define EVENT_R2            17   // 保留
#define EVENT_R3            18   // 保留
#define EVENT_R4            19   // 保留


// 报警事件-- 开始
#define ALARM_EVT_BAT_LOW          20
#define ALARM_EVT_HV_ERR           21    // 高压异常
#define ALARM_EVT_RTC_ERR          22    // 实时钟异常
#define ALARM_EVT_SENSER_ERR       23    // 外部探头通讯异常
#define ALARM_EVT_EEPROM_ERR       24    // EEPROM 异常

#define ALARM_EVT_STOREAGE_FULL   25    // 数据区满
#define ALARM_EVT_LOG_FULL        26
#define ALARM_EVT_STATUS_ERR      27   // 探头状态异常
#define ALARM_EVT_INSENSER_ERR    28   // 内部探头异常
#define ALARM_EVT_SENSER_PWLOW    29   // 探头电压低

// 报警事件-- 结束
#define ALARM_END_BAT_LOW          30
#define ALARM_END_HV_ERR           31    // 高压异常
#define ALARM_END_RTC_ERR          32    // 实时钟异常
#define ALARM_END_SENSER_ERR       33    // 外部探头通讯异常
#define ALARM_END_EEPROM_ERR       34    // EEPROM 异常

#define ALARM_END_STOREAGE_FULL   35    // 数据区满
#define ALARM_END_LOG_FULL        36
#define ALARM_END_STATUS_ERR      37   // 探头状态异常
#define ALARM_END_INSENSER_ERR    38   // 内部探头异常
#define ALARM_END_SENSER_PWLOW    39   // 探头电压低

// 100 以后用于指示软件异常
#define ERR_BT_WAIT                100
#define ERR_BT_ATTEST             101
#define ERR_BT_SETPW               102
#define ERR_BT_ROLE                103
#define ERR_BT_CLEAR               104
#define ERR_BT_RIGHT               105
#define ERR_BT_BIND                106
#define ERR_BT_SETADD              107
#define ERR_BT_LOWPW               108



// 报警声
typedef struct
{
    WORD OnTime;
    WORD OffTime;
}ALARMSOUND;


//报警灯
typedef struct
{
    WORD OnTime;
    WORD OffTime;
}ALARMLIGHT;


typedef struct 
{
    DWORD Status1;  // 每一位表示一个报警
    DWORD Mask1;    // 0 :  不报警 1: 要报警

    // 32个不够了
    DWORD Status2;  // 每一位表示一个报警
    DWORD Mask2;    // 0 :  不报警 1: 要报警

    DWORD MaskB1;
    DWORD MaskB2;
}SYSALARM;


//extern DWORD System_Freq; // 25000000
//#define System_Freq 25000000
//#define delay_us_ex(f,x)  __delay_cycles((f)*(double)(x)/1000000)
//#define delay_ms_ex(f,x)  __delay_cycles((f)*(double)(x)/1000)

//#define delay_us_ex(f,x)  (f+x)
//#define delay_ms_ex(f,x)  (f+x)

extern BOOL CanSleep;

void SysInit();
void idle();
void CpuHighSpeed();
void CpuLowSpeed();

void SetVcoreUp (unsigned int level);

void TimerAInit();
void TimerBInit();

void Sleep(unsigned int ms);
void delay_ms(DWORD x);
void delay_us(DWORD x);
void InitLogicTimer();

void SystemTask(void);

#ifdef PWM_BEEP
char TBPwmInit(char Clk,char Div,char Mode1,char Mode2);
void TBPwmSetPeriod(unsigned int Period);
void TBPwmSetDuty(char Channel,unsigned int Duty);
void TBPwmSetPermill(char Channel,unsigned int Percent);
#endif

void SaveInnerParam();

void Param_Def();
void Param_Read();
void ParamInit();
void ParamSave();
void ShutDown(BYTE Res);
void PowerOff();

void PowerOn();
BYTE GetVlotPer();

unsigned char GetVerfity(unsigned char * data,int len);
void WriteParamToFlash();

BOOL HaveAlarm();
BOOL HaveInAlarm();
BOOL HaveOutAlarm();
BOOL HaveSsAlarm();


void SetAlarm(BYTE Alarm);
void ClrAlarm(BYTE Alarm);
BOOL GetAlarm(BYTE Alarm);

void InitAlarm();
void EnableAlarm(BYTE Alarm);
void DisableAlarm(BYTE Alarm);
void EnableAllAlarm();
void DisableAllAlarm();
BOOL NeedSaveParam();
BOOL GetValidAlarm(BYTE Alarm);
BOOL HaveValidAlarm();
void FlashIo_Ctl(FLASH_IO *Io);
DWORD GetCounter(void);

#ifdef __cplusplus
}
#endif
#endif
