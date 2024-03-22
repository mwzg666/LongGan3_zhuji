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

#define P8V_ON     P3OUT |= BIT5         //  �ⲿ̽ͷ����
#define P8V_OFF   P3OUT &= ~BIT5
#define PW_ON     P6OUT |= BIT0
#define PW_OFF    P6OUT &= ~BIT0
#define BK_ON     P4OUT |= BIT7 
#define BK_OFF    P4OUT &= ~BIT7 
#define UART_OFF  P5OUT &= ~BIT0
#define UART_ON   P5OUT |= BIT0

#ifdef BATTERY_VER
#define GM_HIGH   {P2OUT |= BIT7; delay_ms(10); P2OUT &= ~BIT7;}      // GM Power  ---  �ߵ������л�   1:��
#define GM_LOW    {P3OUT |= BIT2; delay_ms(10); P3OUT &= ~BIT2;}     // 0 : ��
#else
#define GM_HIGH   P2OUT |= BIT7      // GM Power  ---  �ߵ������л�   1:��
#define GM_LOW    P2OUT &= ~BIT7     // 0 : ��
#endif

#define WIFI_ON   P8OUT |=  BIT1
#define WIFI_OFF  P8OUT &= ~BIT1

#define OVL    0   // ����
#define NO1    1   // һ������
#define NO2    2   //  ��������
#define BGL    3   // �ͱ��ױ���
#define BGH    4   // �߱��ױ���
#define LE     5   // ���˵�ͨ���쳣
#define HE     6   // ���˸�ͨ���쳣
#define DE     7   // ����˫ͨ���쳣

// �������壬���ȼ��ߵı�������ǰ��
#define ALARM_AO   0    // ���������ر���
#define ALARM_A1   1    // ������һ������
#define ALARM_A2   2    // ��������������
#define ALARM_AL   3     //������ �ͱ��ױ���
#define ALARM_AH   4     //������ �߱��ױ���

#define ALARM_BO   5    // �������ر���
#define ALARM_B1   6    // ����һ������
#define ALARM_B2   7    // ������������
#define ALARM_BL   8    // �����ͱ��ױ���
#define ALARM_BH   9    // �����߱��ױ���

#define ALARM_ABO   10    // ������+�������ر���
#define ALARM_AB1   11    // ������+����һ������
#define ALARM_AB2   12    // ������+������������
#define ALARM_ABL   13    // ������+�����ͱ��ױ���
#define ALARM_ABH   14    // ������+�����߱��ױ���


//ˮ��PD
#define ALARM_PDO   15    // ���ر���
#define ALARM_PD1   16    // һ������
#define ALARM_PD2   17    // ��������
#define ALARM_PDL   18    // �ͱ��ױ���
#define ALARM_PDH   19    // �߱��ױ���


//ˮ��GM
#define ALARM_GMO   20    // ���ر���
#define ALARM_GM1   21    // һ������
#define ALARM_GM2   22    // ��������
#define ALARM_GML   23    // �ͱ��ױ���
#define ALARM_GMH   24    // �߱��ױ���


// ����
#define ALARM_GDO   25    // ���ر���
#define ALARM_GD1   26    // һ������
#define ALARM_GD2   27    // ��������
#define ALARM_GDL   28    // �ͱ��ױ���
#define ALARM_GDH   29    // �߱��ױ���

// �ⲿY
#define ALARM_YO   30    // ���ر���
#define ALARM_Y1   31    // ����Ԥ��
#define ALARM_Y2   32    // ��������
#define ALARM_YL   33    // �ۼƼ���Ԥ��
#define ALARM_YH   34    // �ۼƼ�������


// �ڲ�̽ͷ
#define ALARM_INO   35    // ���ر���
#define ALARM_IN1   36    // һ������
#define ALARM_IN2   37    // ��������
#define ALARM_IN3   38    // ��������
#define ALARM_INH   39    // �߱��ױ���
#define ALARM_INL    40   // �ͱ��ױ���
#define ALARM_YP     41   // �ۼ�Ԥ��
#define ALARM_YT     42   //  �ܼ�������



// ��������û��ʹ��
#define ALARM_AR    43    // ���������ر���
#define ALARM_BR    44    // �������ر���

// ������������������
#define ALARM_LE   45    // ���˵�ͨ���쳣
#define ALARM_HE   46    // ���˸�ͨ���쳣
#define ALARM_DE   47    // ����˫ͨ���쳣


#define ALARM_RES1    48    // ���������ر���
#define ALARM_RES2    49    // �������ر���


// ϵͳ����
#define ALARM_BAT_LOW      50    // ��ص�ѹ��
#define ALARM_HV_ERR       51    // ��ѹ�쳣
#define ALARM_RTC_ERR      52    // ʵʱ���쳣
#define ALARM_SENSER_ERR   53    // ̽ͷͨѶ�쳣
#define ALARM_EEPROM_ERR   54    // EEPROM �쳣

#define ALARM_STOREAGE_FULL  55    // ��������
#define ALARM_LOG_FULL        56
#define ALARM_STATUS_ERR      57   // ̽ͷ״̬�쳣
#define ALARM_INSENSER_ERR    58   // �ڲ�̽ͷ�쳣
#define ALARM_SENSER_BATERY   59   // ̽ͷ��ѹ��

#define ALARM_COUNT  60


// SysLog Define  
#define EVENT_STARTUP       1     // ����
#define EVENT_SHUTDOWN      2     // �ػ�
#define EVENT_USER_LOGIN    3     // �û���¼
#define EVENT_ADMIN_LOGIN   4     // ����Ա��¼

#define EVENT_MOD_INPARAM   5     // �޸��ڲ�̽ͷ����
#define EVENT_MOD_OUTPARAM   6    // �޸��ⲿ̽ͷ����
#define EVENT_MOD_INALARM    7    // �޸��ڲ�̽ͷ����
#define EVENT_MOD_OUTALARM   8    // �޸��ⲿ̽ͷ����
#define EVENT_DEF_PARAM      9      // ʹ��Ĭ�ϲ���

#define EVENT_DEL_DATA      10     // ɾ������
#define EVENT_DEL_LOG       11     // ɾ����־


// �Լ��¼�
#define EVENT_BAT_LOW       12
#define EVENT_HV_ERR        13
#define EVENT_RTC_ERR       14
#define EVENT_E2_ERR        15

#define EVENT_R1            16   // ����
#define EVENT_R2            17   // ����
#define EVENT_R3            18   // ����
#define EVENT_R4            19   // ����


// �����¼�-- ��ʼ
#define ALARM_EVT_BAT_LOW          20
#define ALARM_EVT_HV_ERR           21    // ��ѹ�쳣
#define ALARM_EVT_RTC_ERR          22    // ʵʱ���쳣
#define ALARM_EVT_SENSER_ERR       23    // �ⲿ̽ͷͨѶ�쳣
#define ALARM_EVT_EEPROM_ERR       24    // EEPROM �쳣

#define ALARM_EVT_STOREAGE_FULL   25    // ��������
#define ALARM_EVT_LOG_FULL        26
#define ALARM_EVT_STATUS_ERR      27   // ̽ͷ״̬�쳣
#define ALARM_EVT_INSENSER_ERR    28   // �ڲ�̽ͷ�쳣
#define ALARM_EVT_SENSER_PWLOW    29   // ̽ͷ��ѹ��

// �����¼�-- ����
#define ALARM_END_BAT_LOW          30
#define ALARM_END_HV_ERR           31    // ��ѹ�쳣
#define ALARM_END_RTC_ERR          32    // ʵʱ���쳣
#define ALARM_END_SENSER_ERR       33    // �ⲿ̽ͷͨѶ�쳣
#define ALARM_END_EEPROM_ERR       34    // EEPROM �쳣

#define ALARM_END_STOREAGE_FULL   35    // ��������
#define ALARM_END_LOG_FULL        36
#define ALARM_END_STATUS_ERR      37   // ̽ͷ״̬�쳣
#define ALARM_END_INSENSER_ERR    38   // �ڲ�̽ͷ�쳣
#define ALARM_END_SENSER_PWLOW    39   // ̽ͷ��ѹ��

// 100 �Ժ�����ָʾ����쳣
#define ERR_BT_WAIT                100
#define ERR_BT_ATTEST             101
#define ERR_BT_SETPW               102
#define ERR_BT_ROLE                103
#define ERR_BT_CLEAR               104
#define ERR_BT_RIGHT               105
#define ERR_BT_BIND                106
#define ERR_BT_SETADD              107
#define ERR_BT_LOWPW               108



// ������
typedef struct
{
    WORD OnTime;
    WORD OffTime;
}ALARMSOUND;


//������
typedef struct
{
    WORD OnTime;
    WORD OffTime;
}ALARMLIGHT;


typedef struct 
{
    DWORD Status1;  // ÿһλ��ʾһ������
    DWORD Mask1;    // 0 :  ������ 1: Ҫ����

    // 32��������
    DWORD Status2;  // ÿһλ��ʾһ������
    DWORD Mask2;    // 0 :  ������ 1: Ҫ����

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
