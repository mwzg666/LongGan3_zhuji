#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "system.h"
#include "msp430x552x.h"
#include "CommDataDefine.h"
#include "ds1337.h"
#include "Oled.h"
#include "Flash.h"
#include "Mcp2510.h"
#include "Senser.h"
#include "Adc12.h"
#include "Main.h"
#include "Measinter.h"
#include "Eeprom.h"
#include "Uart.h"
#include "SelfCheck.h"
#include "DataStore.h"
#include "RadioSet.h"
#include "Keyboard.h"

//BOOL SysIdle = FALSE;
DS1337_QTIME cuTime;
QPara Param;
QInnerParam InnerParam;
SYSALARM SysAlarm;
DWORD System_Freq;
BOOL  SysHighSpeed = TRUE;
BOOL  CanSleep = FALSE;

extern BYTE Can_RxInt;
//extern FLASH_IO RunLed;
extern FLASH_IO AlarmLed;
extern FLASH_IO BeepCtl;
extern int RadSetSel;
extern CANID CanSendId;
extern CANID CanRecId;
extern BYTE CanSendData[8];
extern BYTE CanRecData[8];
extern WORD  CanRxCnt;
extern WORD  EventTimer;
extern const SENSER SsDefault[SENSERCOUNT];
extern SENSER Senser;
extern WORD BkTimer;    // 背光计数
extern BOOL BkStatus;  // 背光开启状态
extern METEFACEINFO FaceInfo;
extern INTERFACE currUI;
extern CHECKRES CheckRes;
extern BOOL InSenserEnable;


#ifdef DOUBLE_DELAY
extern BYTE KeyTimer;
#endif

WORD  TimerACnt = 0;
WORD  SleepCnt  = 0;


// 内部探头
DWORD InCounter = 0;       // 实时计数
DWORD InSenserCnt = 0;    //  每时间段计数
WORD  InSenserTimer = 0;  // 内部探头逻辑timer
WORD  InSerserErrTimer = 0;  

char * const LogName[] =
{
    "未知",
    "开机",
    "关机",
    "用户登录",
    "管理员登录",
    "修改内部探测器参数",    // 5
    "修改外部探测器参数",
    "修改内部探测器报警值",
    "修改外部探测器报警值",
    "使用默认参数",
    "删除所有数据",      // 10
    "删除操作日志",      // 11
    "自检:电池电压低",
    "自检:高压异常",
    "自检:实时钟异常",
    "自检:外部存储异常",   // 15

    "保留",
    "保留",
    "保留",
    "保留",

    "电池电压低:开始",    // 20
    "高压异常:开始",
    "实时钟异常:开始",
    "探头通讯异常:开始",
    "外部存储异常:开始",

    "数据区满:开始",      //25
    "日志区满:开始",
    "探头状态异常:开始",
    "内部探头异常:开始",
    "探测器电压低:开始",

    "电池电压低:结束",    // 30
    "高压异常:结束",
    "实时钟异常:结束",
    "探头通讯异常:结束",
    "外部存储异常:结束",

    "数据区满:结束",      //35
    "日志区满:结束",
    "探头状态异常:结束",
    "内部探头异常:结束",
    "探测器电压低:结束",
};

// 报警名，必须同报警定义一一对应
char * const AlarmName[]=
{
    "@A过载",
    "@A一级报警",
    "@A二级报警",
    "@A低本底报警",
    "@A高本底报警",
    
    "@B过载",
    "@B一级报警",
    "@B二级报警",
    "@B低本底报警",
    "@B高本底报警",

    "@A@B过载",      // 10
    "@A@B一级报警",
    "@A@B二级报警",
    "@A@B低本底报警",
    "@A@B高本底报警",

    "过载",
    "一级报警",
    "二级报警",
    "低本底报警",
    "高本底报警",

    "GM过载",      // 20
    "GM一级报警",
    "GM二级报警",
    "GM低本底报警",
    "GM高本底报警",

    "地面过载",
    "地面核素预警",
    "地面核素报警",
    "地面低本底报警",
    "地面高本底报警",

    "@Y过载",        // 30  外部 y
    "@Y剂量率预警",
    "@Y剂量率报警",
    "@Y累计剂量预警",
    "@Y累计剂量报警",

    "@Y过载",        // 35  内部Y
    "@Y一级报警",
    "@Y二级报警",
    "@Y三级报警",
    "@Y高本底报警",

    "@Y低本底报警",   // 40
    "@Y累计剂量预警",  
    "@Y累计剂量报警",  
    "@A核素报警",
    "@B核素报警",

    
    "探测器低通道异常",   // 45
    "探测器高通道异常",   
    "探测器双通道异常",   

    "保留1",   
    "保留2",   
    
    "主机电池电压低",   // 50
    "高压异常",       
    "实时钟异常",
    "探测器通讯异常",
    "内部存储异常",
    
    "数据区满",      // 55
    "日志区满",          
    "外部探测器异常",
    "内部探测器异常",
    "探测器电量低",   
};

const ALARMSOUND AlarmSound[64] =
{
    {100,100},
    {200,1000},
    {200,200},
    {100,100},
    {100,100},
        
    {100,100},
    {200,1000},
    {200,200},
    {100,100},
    {100,100},

    {100,100},   // 10
    {200,1000},
    {200,200},
    {100,100},
    {100,100},
        
    {100,100},
    {200,1000},
    {200,200},
    {100,100},
    {100,100},

    {100,100},   // 20
    {200,1000},
    {200,200},
    {100,100},
    {100,100},
        
    {100,100},
    {200,1000},
    {200,200},
    {200,1000},
    {200,200},

    {100,100},   //30
    {200,1000},
    {200,200},
    {200,1000},
    {200,200},
        
    {100,100},
    {200,1000},
    {200,200},
    {100,100},
    {100,100},
        
    {200,200},   // 40
    {200,1000},
    {200,200},
    {100,100},
    {100,100},

    {100,100},   // 45
    {100,100},
    {100,100},
    {100,100},
    {100,100},
        
    {200,3000},  // 50
    {100,100},
    {100,1000},
    {100,100},
    {100,100},
        
    {100,2000},  //55
    {100,2000},
    {100,100},
    {100,100},
    {100,100},
        
    
        
    {100,100},  //60
    {100,100},
    {100,100},
    {100,100},
};


#if 0
const ALARMLIGHT AlarmLight[64] =
{
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},

    {100,100},  //10
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},

    {100,100}, //20
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},

    {100,100},  //30
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},

    {200,3000},   // 40
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},

    {100,100},  //50
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},
    {100,100},

    {100,100},  //60
    {100,100},       
    {100,100},
    {100,100},
};
#endif


void SysInit()
{
  
    WDTCTL = WDTPW+WDTHOLD;                   // Stop WDT

    //P1DIR |= BIT1;                            // P1.1 output
    //P1DIR |= BIT0;                            // ACLK set out to pins
    //P1SEL |= BIT0;                            
    //P2DIR |= BIT2;                            // SMCLK set out to pins
    //P2SEL |= BIT2;                            
    #ifdef TIMERB_DOSE
    P7DIR &= ~BIT7;                         
    P7SEL |= BIT7;    
    #endif

    // Increase Vcore setting to level3 to support fsystem=25MHz
    // NOTE: Change core voltage one level at a time..
    SetVcoreUp (0x01);
    SetVcoreUp (0x02);
    SetVcoreUp (0x03);

    UCSCTL3 = SELREF_2;                       // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_2;                        // Set ACLK = REFO

    //MCLK Out
    //UCSCTL4 &= 0xF8;
    //UCSCTL5 &= 0xF8;
    //UCSCTL5 |= 0x04;

    #if 0
    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_7;                      // Select DCO range 50MHz operation
    //UCSCTL2 = FLLD_0 + 762; //25M                  // Set DCO Multiplier for 25MHz   762
    //UCSCTL2 = FLLD_1 + 244; //8M                      // (N + 1) * FLLRef = Fdco
                                                   // (762 + 1) * 32768 = 25MHz
    UCSCTL2 = FLLD_2 + 122; //4M                         // Set FLL Div = fDCOCLK/2
    //UCSCTL2 = FLLD_3 + 60; //2M 
    //UCSCTL2 = FLLD_4 + 30; //1M         
    
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK cycles for DCO to settle
    __delay_cycles(782000/3);

    // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
    do
    {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                                // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
    #else
    CpuHighSpeed();
    #endif

    //初始化计时器
    TimerAInit();   // 核心定时器

    #ifdef TIMERB_DOSE
    TimerBInit();   // 剂量计数
    #endif
}

void SetVcoreUp (unsigned int level)
{
    // Open PMM registers for write
    PMMCTL0_H = PMMPW_H;              
    // Set SVS/SVM high side new level
    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level;
    // Set SVM low side to new level
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level;
    // Wait till SVM is settled
    while ((PMMIFG & SVSMLDLYIFG) == 0);
    // Clear already set flags
    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);
    // Set VCore to new level
    PMMCTL0_L = PMMCOREV0 * level;
    // Wait till new level reached
    if ((PMMIFG & SVMLIFG))
    while ((PMMIFG & SVMLVLRIFG) == 0);
    // Set SVS/SVM low side to new level
    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level;
    // Lock PMM registers for write access
    PMMCTL0_H = 0x00;
}


void CpuHighSpeed()
{
    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_7;                      // Select DCO range 50MHz operation
    UCSCTL2 = FLLD_0 + 762; //25M                  // Set DCO Multiplier for 25MHz   762
    //UCSCTL2 = FLLD_1 + 244; //8M                      // (N + 1) * FLLRef = Fdco
                                                   // (762 + 1) * 32768 = 25MHz
    //UCSCTL2 = FLLD_2 + 122; //4M                         // Set FLL Div = fDCOCLK/2
    //UCSCTL2 = FLLD_3 + 60; //2M 
    //UCSCTL2 = FLLD_4 + 30; //1M         
    
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK cycles for DCO to settle
    __delay_cycles(782000/3);

    // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
    do
    {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                                // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

    System_Freq = 25000000;
    SysHighSpeed = TRUE;
}

void CpuLowSpeed()
{
    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_7;                      // Select DCO range 50MHz operation
    //UCSCTL2 = FLLD_0 + 762; //25M                  // Set DCO Multiplier for 25MHz   762
    //UCSCTL2 = FLLD_1 + 244; //8M                      // (N + 1) * FLLRef = Fdco
                                                   // (762 + 1) * 32768 = 25MHz
    UCSCTL2 = FLLD_2 + 122; //4M                         // Set FLL Div = fDCOCLK/2
    //UCSCTL2 = FLLD_3 + 60; //2M 
    //UCSCTL2 = FLLD_4 + 30; //1M         
    
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 25 MHz / 32,768 Hz ~ 780k MCLK cycles for DCO to settle
    __delay_cycles(782000/3);

    // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize
    do
    {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                                // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                      // Clear fault flags
    }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

    System_Freq = 4000000;
    SysHighSpeed = FALSE;
}


void idle()
{
    if (CanSleep)
    {
        #ifdef USE_IDLE
        //_BIS_SR(LPM3_bits + GIE);     // Enter LPM3
        LPM3;
        #endif
    }
}

#if 1
#define HIGH_FREQ 25000000
#define LOW_FREQ  4000000

#define delay_1ms_h()  __delay_cycles((long)HIGH_FREQ*(double)(1.0)/1000.0)
#define delay_1ms_l()  __delay_cycles((long)LOW_FREQ*(double)(1.0)/1000.0)
#define delay_1us_h()  __delay_cycles((long)HIGH_FREQ*(double)(1.0)/1000000.0)
#define delay_1us_l()  __delay_cycles((long)LOW_FREQ*(double)(1.0)/1000000.0)

void delay_ms(DWORD t)
{
    if (SysHighSpeed)
    {
        while(t--)  delay_1ms_h();
    }
    else
    {
        while(t--)  delay_1ms_l();
    }
}


void delay_us(DWORD t)
{
    if (SysHighSpeed)
    {
        while(t--)  delay_1us_h();
    }
    else
    {
        while(t--)  delay_1us_l();
    }
}
#endif

void run()
{
    //_BIC_SR_IRQ(LPM3_bits);
}

void Init_InSenser()
{
    #ifndef TIMERB_DOSE
    P2DIR &= ~BIT6;

    P2IES |= BIT6;
    P2IE  |= BIT6;
    #endif
    
    // 上电时清除累计剂量和最大剂量
    //FaceInfo.In_Total = 0;
    //FaceInfo.In_Max= 0;
}

#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void) 
{ 
    //_BIC_SR_IRQ(LPM3_bits);   // 这句好像还不能打开

    #ifdef BATTERY_VER
    if(P2IFG & BIT3)
    {
       Can_RxInt = 1;
    }

    if(P2IFG & BIT4)
    {
       Can_RxInt = 1;
    }

    #else
    if(P2IFG & BIT5)
    {
       Can_RxInt = 1;
    }
    #endif

    #ifndef TIMERB_DOSE
    if(P2IFG & BIT6)
    {
       InCounter ++;
    }
    #endif

    P2IFG = 0;
}


void TimerAInit()
{ 
    TA1CTL = TASSEL_1 + MC_1 + TACLR;         // ACLK, upmode, clear TAR  32K 
    TA1CCTL0 = CCIE;                          // CCR0 interrupt enabled
    TA1CCR0 = 3277; //约 100MS     // 1s 相当于32768 个ACLK 
    _EINT();
}

#ifdef TIMERB_DOSE
DWORD TimerBCounter = 0;
#endif

#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)  //计数到TA1CCR0中断
{   

    #ifdef USE_IDLE
    //if (SysIdle)
    //{
        //SysIdle = FALSE;
    //    _BIC_SR_IRQ(LPM3_bits);   // 唤醒
    //}
    
    LPM3_EXIT; 
    #endif
    TimerACnt ++;

    #ifdef TIMERB_DOSE
    TimerBCounter += TB0R;
    TB0R = 0;
    #endif
}

#ifdef TIMERB_DOSE  // 用于计量率计数
//static BYTE OverFlowCnt=0;


void TimerBInit()  
{   
    TBCTL =  TBSSEL_0+TBCLR; //使用ACLK,上限模式，1分频，允许中断
	TBCCR0 = 65534; 
	//TBCCTL0 |= CCIE;
	TBCTL |= MC_1;//上限模式  
	
    //TBCTL = TBSSEL_1 + MC_1 + TBCLR;         // ACLK, upmode, clear TAR
    //TBCCTL0 = CCIE;                          // CCR0 interrupt enabled
    //TBCCR0 = 32880;//32770-1;                      // 1s 相当于32776 个ACLK 
    _EINT();
}

#if 0
/*******************************************************************************
功能：TIMER B 中断服务程序
输入：无
输出：无
*******************************************************************************/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void TB0_ISR(void)
{
  	OverFlowCnt++;
}
#endif

DWORD GetCounter(void)
{
  	DWORD Return;

    #if 0
	Return = OverFlowCnt*65000+TB0R;
	TB0R = 0;
	OverFlowCnt = 0;
    #else
    Return = TimerBCounter;
    TimerBCounter = 0;
    #endif
	return Return;
}
#endif

#ifdef PWM_BEEP   // 用PWM 实现蜂鸣器
char TBPwmInit(char Clk,char Div,char Mode1,char Mode2)
{
    TBCTL = 0;                  //清除以前设置
    //TBCTL |= MC_1;  // ok
    TBCTL |= MC_1 | CNTL__8 ;              //定时器设为增计数模式  
    switch(Clk)                 //选择时钟源
    { 
        case 'A': case 'a':  TBCTL|=TBSSEL_1; break;    //ACLK
        case 'S': case 's':  TBCTL|=TBSSEL_2; break;    //SMCLK
        case 'E':            TBCTL|=TBSSEL_0; break;    //外部输入(TACLK)
        case 'e':            TBCTL|=TBSSEL_3; break;    //外部输入(TACLK取反)
        default :  return(0);                           //参数有误
    } 
    
    switch(Div)                 //选择分频系数
    { 
        case 1:   TBCTL|=ID_2; break;   // 1
        case 2:   TBCTL|=ID_1; break;   // 2
        case 4:   TBCTL|=ID_2; break;   //4
        case 8:   TBCTL|=ID_3; break;   // 8
        default :  return(0);           //参数有误
    } 
    
    switch(Mode1)               //设置PWM通道1的输出模式。
    { 
        case 'P':case 'p':          //如果设置为高电平模式
            TBCCTL0 = OUTMOD_7;     //高电平PWM输出
            //TBCCTL1 = OUTMOD_3;     //高电平PWM输出
            P5SEL |= BIT6;          //从P5.6输出 (不同型号单片机可能不一样)
            P5DIR |= BIT6;          //从P5.6输出 (不同型号单片机可能不一样)              
            break;
        case 'N':case 'n':          //如果设置为低电平模式          
            TBCCTL1 = OUTMOD_6;     //低电平PWM输出
            //TBCCTL2 = OUTMOD_6;     //低电平PWM输出
            P5SEL |= BIT6;          //从P1.2输出 (不同型号单片机可能不一样) 
            P5DIR |= BIT6;          //从P1.2输出 (不同型号单片机可能不一样)                
            break; 
        case '0':case 0:            //如果设置为禁用          
            P5SEL &= ~BIT6;         //恢复为普通IO口       
            //P5SEL &= ~BIT7;
            break;                 
        default :  return(0);       //参数有误
    } 

    #if 0
    switch(Mode2)                   //设置PWM通道1的输出模式。
    { 
        case 'P':case 'p':          //如果设置为高电平模式
            TBCCTL1 =OUTMOD_7;      //高电平PWM输出
            TBCCTL2 =OUTMOD_7;
            P5SEL |= BIT7;          //从P1.3输出 (不同型号单片机可能不一样)
            P7DIR |= BIT7;          //从P1.3输出 (不同型号单片机可能不一样)
            break;
        case 'N':case 'n':          //如果设置为低电平模式          
            TBCCTL2 =OUTMOD_3;      //低电平PWM输出
            P1SEL |= BIT3;          //从P1.3输出 (不同型号单片机可能不一样)  
            P1DIR |= BIT3;          //从P1.3输出 (不同型号单片机可能不一样)              
            break; 
        case '0':case 0:            //如果设置为禁用          
            P5SEL &= ~BIT7;         //P1.3恢复为普通IO口              
            break;                 
        default :  return(0);       //参数有误
    }    
    #endif
    
    return(1);  
}

void TBPwmSetPeriod(unsigned int Period)
{
    //TBCCR0 = Period;
    TBCCR0 = Period;
}

void TBPwmSetDuty(char Channel,unsigned int Duty)
{
    TBCCR1 = Duty;
    //TBCCR2 = Duty;
    //TBCCR3 = Duty;
    //TBCCR4 = Duty;
    //TBCCR5 = Duty;
    //TBCCR6 = Duty;
    
    #if 0
    switch(Channel)
    {
        //case 0: TBCCR0=Duty; break; 
        case 1: TBCCR1=Duty; break; 
        case 2: TBCCR2=Duty; break;    
        case 3: TBCCR3=Duty; break;  
    }
    #endif
}


void TBPwmSetPermill(char Channel,unsigned int Percent)
{
    unsigned long int Period;
    unsigned int Duty;
    Period = TBCCR0;
    Duty = Period * Percent ; // / 1000;
    TBPwmSetDuty(Channel,Duty);
}
#endif

void InitLogicTimer()
{
    EventTimer = 0;  // 计数器清零
    TimerACnt = 0;
}


void FlashIo_Ctl(FLASH_IO *Io)
{
    if (Io->Enable)
    {
        if (Io->Status)  // On
        {
            if (Io->Count >= Io->OnTime)
            {
                // Trun off 
                Io->Count = 0;
                Io->Status = 0;
                Io->Off();

                // 如果OffTime == 0 只开一下就关闭
                if (Io->OffTime == 0)  
                {
                    Io->Enable = FALSE;
                }
            }
        }
        else  // Off
        {
            if (Io->Count >= Io->OffTime)
            {
                // Trun On 
                Io->Count = 0;
                Io->Status = 1;
                Io->On();
            }
        }
    }
}


// 内部探头量程切换
// 降低到80%才切到底量程
void InnerSwith()
{
    // S4 不切换量程 |
    if (InnerParam.DevType != DEV_MPRP_S2L)
    {
        // 切换量程
        // 2019.1.10 大于2mSv/h 就切换
        if (FaceInfo.In_Rt > 2000.0)
        {
            if (FaceInfo.Gm == 0)
            {
                GM_HIGH;
                FaceInfo.Gm = 1;
                ClearCounter();
            }
        }
        else
        {
            if (FaceInfo.Gm == 1) 
            {
                if (FaceInfo.In_Rt < 1000.0)
                {
                    GM_LOW;
                    FaceInfo.Gm = 0;
                    ClearCounter();
                }
            }
        }
    }
}


void HndInnerCounter()
{    
    float cps;
    
    // 取内部探头计数值
    if (InSenserTimer >= Param.InSenser.Param.Ct)
    {
        #ifdef TIMERB_DOSE
        if ((currUI == MEASU) || (currUI == INTEST))
        {
            InSenserCnt = GetCounter();
        }
        else
        {
            GetCounter();
        }
        #else
        InSenserCnt = InCounter;
        InCounter = 0;
        #endif
        InSenserTimer = 0;

        if (InSenserCnt == 0)
        {
            // 10 分钟没有计数要判断内部探头异常
            InSerserErrTimer ++;
        }
        else
        {
            InSerserErrTimer = 0;
        }

        if (currUI == MEASU)   // 主界面才取计数
        {
            
            //FaceInfo.In_Cps = Get_InCounter();
            cps = Get_InCounter();
            if (cps > 0)
            {
                FaceInfo.In_Cps = cps;
                FaceInfo.In_Rt = CpsToUsv_h(FaceInfo.In_Cps);   // uSv/h
            }
            
            #if 0
            if (InnerParam.DevType == DEV_MPRP_S2L)
            {
                if (FaceInfo.In_Cps < 1.0)
                {
                    FaceInfo.In_Cps = 0.0;
                }
            }
            #endif

            #ifdef TIMERB_DOSE
            CanSleep = TRUE;
            #else
            #ifdef USE_IDLE
            if (FaceInfo.In_Cps > 2000)
            {
                CanSleep = FALSE;
            }
            else
            {
                CanSleep = TRUE;
            }
            #endif
            #endif
            
            if (FaceInfo.In_Rt <= 0.1)
            {
                FaceInfo.In_Rt = 0.1;
            }

            //  外接NaI探头不切换
            if (CheckRes.SenserType != SENSER_Y)
            {
                InnerSwith();
            }
            
            //记录最大值
            if (FaceInfo.In_Max < FaceInfo.In_Rt)
            {
                FaceInfo.In_Max = FaceInfo.In_Rt;
            }
            
            // 累计剂量
            TotalAdd();

            // 在这里要判断并报警
            //Check_InAlarm();
            if (CheckRes.SenserType == SENSER_Y)
            {
                // 用外部Y 探头的参数
                Check_InAlarmEx();
            }
            else
            {
                Check_InAlarm();
            }
        }
    }
}

// 实时任务-- 里面不能再调用sleep
void SystemTask(void)
{
    WORD Delta;

    Delta = 0;
    if (0 != TimerACnt)
    {
        Delta = TimerACnt * 100;
        TimerACnt = 0;
        
        SleepCnt += Delta;
        CanRxCnt += Delta;
        EventTimer += Delta;
        InSenserTimer += Delta;

        #ifdef DOUBLE_DELAY
        if (KeyTimer < KEYDELAY)
        {
            KeyTimer += Delta;
        }
        #endif

        
        if (AlarmLed.Enable)   
        {
            AlarmLed.Count += Delta;
            FlashIo_Ctl(&AlarmLed);
        }
        
        if (BeepCtl.Enable)
        {
            if (InnerParam.Sound)
            {
                BeepCtl.Count += Delta;
                FlashIo_Ctl(&BeepCtl);
            }
            else
            {
                BeepOff();
            }
        }

        if (InSenserEnable)
        {
            HndInnerCounter();
        }
        else //内部探头禁用
        {
            if( CheckRes.SenserType == SENSER_LC )
            {
                //液体探头
                Check_InAlarmLC();
            }
        }
        
        if (BkStatus) 
        {
            BkTimer += Delta;
            BkLightCtl();
        }
    }

    #ifdef UART_DOSE
    if (currUI == DATAMAN)
    {
        Handle_Uart_Rec();
    }
    #endif
}


// 最小单位是100ms
// 低于100ms的延时请用delay_ms/delay_us
void Sleep(unsigned int ms)
{   
    SleepCnt = 0;
    
    while(1)
    {
        SystemTask();

        if (SleepCnt >= ms)
        {
            break;
        }        
    }
}

void Param_Def()
{   
    // use default
    
    
    #if 0
    Param.StorageNum = 0;
    Param.LastData = 0;
    Param.LogNum = 0;
    Param.LastLog = 0;
    #endif

    //innerparam
    InnerParam.CurrSenserType = SENSER_NONE;
    InnerParam.Sound = 1;
    InnerParam.pisScreen = 0;
    InnerParam.Keytone = 0;
    InnerParam.RadioSelIndex = 0;
    InnerParam.MeauExitTime = 30;
    InnerParam.BkTimeout = 60;
    InnerParam.PassWord = 1000;
    
    #ifdef DG105
    InnerParam.DevType = DEV_MPRP_S2L;
    #else
    InnerParam.DevType = DEV_MPRP_S2D; // 1; -- 2017.9.29  默认改为S2 
    #endif
    
    #ifdef FOR_TEMP
    InnerParam.BqXs = 170; 
    #else
    InnerParam.BqXs = 300;
    #endif

    
    InnerParam.La = 0.63;  // 低量程
    InnerParam.Lb = 0.00019;
    InnerParam.Lc = 0.83;  //1.0;
    InnerParam.Ha = 33.6; // 8.2;  // 高量程
    InnerParam.Hb = 0.000019;
    InnerParam.Hc = 0.23; // 1.0;

    #ifdef DG105
    InnerParam.Ya = 33.6;  // 高量程
    InnerParam.Yb = 0.000023; 
    InnerParam.Yc = 1.0;
    #else
    InnerParam.Ya = 0.63;  // 低量程
    InnerParam.Yb = 0.00019;
    InnerParam.Yc = 0.83;  //1.0;
    #endif
    
    InnerParam.Fudu = 600; // 1500; 2019.1.10  默认改为 600
    //InnerParam.ZhuAlarmRate = 5;

    //memcpy(InnerParam.BtAddress,"123456789012",12);
    //memcpy(InnerParam.BtAddress,"001B3510D7D7",12);
    memcpy(InnerParam.BtAddress,"001B3514315D",12);
    //memcpy(InnerParam.BtAddress,"001B3513C763",12);
    InnerParam.BtAddress[12] = 0;
    InnerParam.BtPower = 7;
    InnerParam.MeasTime = 30;
    InnerParam.ComLog = COM_MW;
    
    if (InnerParam.DevType == DEV_MPRP_S2N)
    {
        InnerParam.InnerSersor = FALSE;
    }
    else
    {
        InnerParam.InnerSersor = TRUE;
    }
    
    Param.ActiveMode = 0;
    Param.AlarmStore = 1;
    Param.MeteTime = 1;     
    Param.DataType = DATA_RT;
    Param.TrigerType = TRG_ALL;
    
    //Param.pUnitSel = 1;   
    //Param.pnHvErr = 5;
    
    
    // Param.pchTimeMeasur = 1;
    Param.ucStartBlock = 0;

    RadInit();

    

    memcpy(&Param.InSenser, &SsDefault[0], sizeof(Senser));

    #ifdef USE_WIFI
    WifiParamDef();
    #endif

    InnerParam.DoseMax = 0;
    InnerParam.DoseTotal = 0;
    //memset(&Param.MetInfo,0,sizeof(METEFACEINFO));
}


void Erase_UserData()
{
    #ifdef LOG_IN_INNER_FALSH
    DWORD i;
    for (i=0;i<LOG_COUNT/64;i++)
    {
        Erase_Block((BYTE *)(FLASH_LOG_START_ADDR+i*512));
    }
    #endif
    
    InnerParam.LogNum = 0;
    InnerParam.LastLog = 0;


    Erase_Data_Seg(BANK_D);
    
    InnerParam.StorageNum = 0;
    Param.ucStartBlock = 0;
    InnerParam.LastData = 0; 
}

void Param_Read()
{  
    memset(&InnerParam,0,sizeof(InnerParam));
    memset(&Param,0,sizeof(Param));
    

    BYTE * Data = (unsigned char *)&Param;

    #if 0
    for (int i=0;i<sizeof(Param);i++)
    {
        Eeprom_ReadByte(PARAM_OFFSET+i,(Data+i));
    }
    #else
    Eeprom_ReadBuff(PARAM_OFFSET,Data,sizeof(QPara));
    #endif

    Sleep(100);
    
    if(Param.ucCheck != GetVerfity((unsigned char *)&Param,sizeof(Param)))
    {
        Param_Def();
    }

    

    // 解决编程后需要将日志记录清空的问题
    Flash_Read((unsigned char *)FLASH_PARA_START_ADDR, (unsigned char *)&InnerParam, sizeof(InnerParam));
    if(InnerParam.ucCheck != GetVerfity((unsigned char *)&InnerParam,sizeof(InnerParam)))
    {
        //InnerParam.Sound     = 1;                    //报警音开关   1 开，0 关
        //InnerParam.Keytone   = 0;                    //按键音
        //InnerParam.pisScreen = 0;                    //屏幕保护  -- 改为背光


        // 下面几个计数不能恢复默认
        InnerParam.LastData    = 0;
        InnerParam.LastLog     = 0;
        InnerParam.LogNum      = 0;
        InnerParam.StorageNum  = 0;
    
        //InnerParam.TotalCounter = 0;
        //InnerParam.MaxCounter   = 0;
        


        // 重新编程后所有参数用默认值
        Param_Def();

        #ifndef DEBUG
        Erase_UserData();
        #endif
        
    }
    
    
}


#if 0
void ParamInit()
{
    isSound             = Param.Sound;          //声音
    AlarmStore          = Param.pAlarmStore;
    ActiveMode          = Param.ActiveMode;     //自动存储
    MeteTime            = Param.MeteTime;       //存储时间
    SelTime             = Param.SelTime;        //本底测量时间
    nPasWD              = Param.PassWord;       //密码
    MenuRtnTimeIndex    = Param.MeauExitTime;   //菜单退出时间
    MeterChannel        = Param.MeterChannel;   //探头类型
    AerfaHBntAlarm      = Param.AerfaHBntAlarm; //阿尔法高本底阈值
    BeitaHBntAlarm      = Param.BeitaHBntAlarm; //贝塔高本底阈值
    AerfaAlarm          = Param.AerfaThround;   //阿尔法报警阈值
    BeitaAlarm          = Param.betaThround;    //贝塔报警阈值
    chShowUnit          = Param.pUnitSel;       //显示单位
    nHvErr              = Param.pnHvErr;        //高压误差
    Alfacps             = Param.AlfaGrossValue; //阿尔法本底值
    Bertacps            = Param.BertaGrossValue;//贝塔本底值
    nAVvalu             = Param.AVvalu;         //阿尔法V
    nBHvalu             = Param.BHvalu;         //贝塔L
    nBLvalu             = Param.BLvalu;         //贝塔H
    nHVvalu             = Param.HVvalu;         //高压
    isScreen            = Param.pisScreen;      //屏幕保护
    isKeytone           = Param.Keytone;        //声音
    chTimeMeasur        = Param.pchTimeMeasur;  //定时测量时间
    //memcpy(Param.pnuc,Param.pnuc,sizeof(Param.pnuc));         //25个核素
}

void ParamSave()
{
    Param.Sound             = isSound;
    Param.AerfaThround      = AerfaAlarm;
    Param.betaThround       = BeitaAlarm;
    Param.pAlarmStore       = AlarmStore;
    Param.ActiveMode        = ActiveMode;
    Param.MeteTime          = MeteTime;
    Param.SelTime           = SelTime;
    Param.PassWord          = nPasWD;
    Param.MeauExitTime      = MenuRtnTimeIndex;
    Param.MeterChannel      = MeterChannel;
    Param.AerfaHBntAlarm    = AerfaHBntAlarm;
    Param.BeitaHBntAlarm    = BeitaHBntAlarm;
    Param.pUnitSel          = chShowUnit;
    Param.pnHvErr           = nHvErr;
    Param.AlfaGrossValue    = Alfacps;
    Param.BertaGrossValue   = Bertacps;
    Param.AVvalu            = nAVvalu;
    Param.BHvalu            = nBHvalu;
    Param.BLvalu            = nBLvalu;
    Param.HVvalu            = nHVvalu;
    Param.pisScreen         = isScreen;
    Param.Keytone           = isKeytone;
    Param.pchTimeMeasur     = chTimeMeasur;
    //memcpy(Param.pnuc,Param.pnuc,sizeof(Param.pnuc));
}
#endif

void PowerOn()
{
    BYTE Cnt = 0;
    Cnt = 0;
    while((P1IN & BIT0) == 0)
    {
        delay_ms(100);
        if (Cnt == 10)
        {
            // 电池电压低于6.2 且没有外接电源
            if ((6.2 > GetVlotValu()) && (DC_IN != 0))
            {
                AlarmLedOn();
                RunLedOn();
                delay_ms(100);
                PW_OFF;
                while(1);
            }
        }
        
        if (Cnt++ > 20)
        {
            return;
        }
    }

    PW_OFF;
    while(1);
}

void PowerOff()
{
    PW_OFF;
    while(1);
}

void ShutDown(BYTE Res)
{
    StopBeep();
    StopAlarmLed();
    CloseBkLight();
    CloseHv();

    //StorageLog(EVENT_SHUTDOWN);

    BeepOn();
    
    Clear_DispBuf();
    //DisplayRefresh();
    if (Res == 1) //  电池电压低
    {
        GT_Show16X16Char(24,10,"电池电压低",0);
    }
    GT_Show16X16Char(32,32,"正在关机",0);

    // 3
    Show16X24int(56,64,3,0);
    DisplayRefresh();

    Sleep(100);
    BeepOff();

    if (CheckRes.CommInterface == COMM_BT)
    {
        // 关机的时候关闭探头电源
        BTSenserPower(0);
    }

    //memcpy(&Param.MetInfo, &FaceInfo,  sizeof(METEFACEINFO));
    InnerParam.DoseMax = FaceInfo.In_Max;
    InnerParam.DoseTotal = FaceInfo.In_Total;
    if (NeedSaveParam())
    {
        WriteParamToFlash();
    }
    else
    {
        SaveInnerParam();
        delay_ms(800);
    }
    // 保存测量数据
    Save_CurrBlock();
    
    // 2
    Clear(56,64,16,24);
    Show16X24int(56,64,2,0);
    DisplayRefresh();

    // 1
    delay_ms(1000);
    Clear(56,64,16,24);
    Show16X24int(56,64,1,0);
    DisplayRefresh();
    
    delay_ms(1000);
    Clear_DispBuf();
    DisplayRefresh();

    

    PW_OFF;
    
    //_BIS_SR(LPM3_bits + GIE);               // Enter LPM3  for test
    while(1);
            
}

#if 0
float GetVlotValu()
{
    //float uiVolt = (Get_Bat_Vol()/(float)1000);

    #if 0
    if(uiVolt < 6.2)
    {
        //关机
        ShutDown();
    }
    #endif
    
    return Get_Bat_Vol();
}
#endif

// 获取电压百分比
BYTE GetVlotPer()
{
    int per;
    float uiVolt;          // 电池电压

    uiVolt = GetVlotValu();

    
    // 计算百分比
    if(DC_IN == 0)   // 外接电源
    {   
        // 充电时电压较高
        if (uiVolt < 8.0)
        {
            //ucVoltPer = (unsigned char)((uiVolt-6.8)*100/1.6);
            per = (int)((uiVolt-6.6)*100/1.8);
        }
        else
        {
            // 大于 8 v 后充电时电压上升缓慢
            per = (int)((uiVolt-6.3)*100/2.1);
        }

        if(per <= 1)
        {
            per = 1;
        }
        else if(per >= 100)
        {
            per = 99;
        }
    }
    else
    {
        per = (int)((uiVolt-6.4)*100/1.8);
        if(per <= 1)
        {
            per = 1;
        }
        else if(per >= 100)
        {
            per = 100;
        }
    }

    return (BYTE)per;
}


#if 0
char* GetNucNameByIndex(char *NucName,char index)
{
    if(index == 0)
    {
        memcpy(NucName,"MIX",strlen("MIX"));
    }
    else if(index == 1)
    {
        memcpy(NucName,"H3",strlen("H3"));
    }
    else if(index == 2)
    {
        memcpy(NucName,"C14",strlen("C14"));
    }
    else if(index == 3)
    {
        memcpy(NucName,"Na22",strlen("Na22"));
    }
    else if(index == 4)
    {
        memcpy(NucName,"P32",strlen("P32"));
    }
    else if(index == 5)
    {
        memcpy(NucName,"Ti44+Sc44",strlen("Ti44+Sc44"));
    }
    else if(index == 6)
    {
        memcpy(NucName,"Ca45",strlen("Ca45"));
    }
    else if(index == 7)
    {
        memcpy(NucName,"Co58",strlen("Co58"));
    }
    else if(index == 8)
    {
        memcpy(NucName,"Co60",strlen("Co60"));
    }
    else if(index == 9)
    {
        memcpy(NucName,"Zn65",strlen("Zn65"));
    }
    else if(index == 10)
    {
        memcpy(NucName,"Ge68+Ga68",strlen("Ge68+Ga68"));
    }
    else if(index == 11)
    {
        memcpy(NucName,"Sr90",strlen("Sr90"));
    }
    else if(index == 12)
    {
        memcpy(NucName,"Y90",strlen("Y90"));
    }
    else if(index == 13)
    {
        memcpy(NucName,"I131",strlen("I131"));
    }
    else if(index == 14)
    {
        memcpy(NucName,"Cs137",strlen("Cs137"));
    }
    else if(index == 15)
    {
        memcpy(NucName,"Au198",strlen("Au198"));
    }
    else if(index == 16)
    {
        memcpy(NucName,"Hg203",strlen("Hg203"));
    }
    else if(index == 17)
    {
        memcpy(NucName,"Tl204",strlen("Tl204"));
    }
    else if(index == 18)
    {
        memcpy(NucName,"Po210",strlen("Po210"));
    }
    else if(index == 19)
    {
        memcpy(NucName,"U235",strlen("U235"));
    }
    else if(index == 20)
    {
        memcpy(NucName,"U238",strlen("U238"));
    }
    else if(index == 21)
    {
        memcpy(NucName,"Pu238",strlen("Pu238"));
    }
    else if(index == 22)
    {
        memcpy(NucName,"Pu239",strlen("Pu239"));
    }
    else if(index == 23)
    {
        memcpy(NucName,"Am241",strlen("Am241"));
    }
    else if(index == 24)
    {
        memcpy(NucName,"Cm244",strlen("Cm244"));
    }
    return NucName;
}
#endif

unsigned char GetVerfity(unsigned char * data,int len)
{
    unsigned long ulVer = 0;
    for(int i = 1; i < len; i++)
    {
        ulVer += *(data+i);
    }
    return ulVer%250;
}

void SaveInnerParam()
{
    Erase_Para_Seg();
    InnerParam.ucCheck = GetVerfity((unsigned char *)&InnerParam,sizeof(InnerParam));
    Write_Pare_Seg((unsigned char *)&InnerParam,sizeof(InnerParam));
}

BOOL NeedSaveParam()
{
    if (Param.ucCheck != GetVerfity((unsigned char *)&Param,sizeof(Param)))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void WriteParamToFlash()
{
    //StopBeep();
    BeepOff();

    SaveInnerParam();

    // 外部参数
    
    Param.ucCheck = GetVerfity((unsigned char *)&Param,sizeof(Param));
    
    BYTE * Data = (unsigned char *)&Param;

    _DINT();
    for (int i=0;i<sizeof(Param);i++)
    {
        Eeprom_WriteByte(PARAM_OFFSET+i,*(Data+i));
    }
    _EINT();

}


/*******************************************/
/*        系统报警处理                                      */
/*******************************************/
BOOL HaveAlarm()
{
    if ( (SysAlarm.Status1 == 0) &&
          (SysAlarm.Status2 ==0 ))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL HaveValidAlarm()
{
    if ( ((SysAlarm.Status1 & SysAlarm.Mask1)==0) &&
          ((SysAlarm.Status2 & SysAlarm.Mask2)==0) )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }

}


BOOL HaveSsAlarm()
{
    #if 1
    BYTE i;
    for (i=ALARM_AO;i<=ALARM_RES2;i++)
    {
        if (GetAlarm(i))
        {
            return TRUE;
        }
    }

    return FALSE;
    #else
    if ( ((SysAlarm.Status1 & SysAlarm.Mask1)==0) &&
          ((SysAlarm.Status2 & SysAlarm.Mask2)==0) )
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
    #endif
}


BOOL HaveInAlarm()
{
    BYTE i;
    for (i=ALARM_INO;i<=ALARM_YT;i++)
    {
        if (GetAlarm(i))
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL HaveOutAlarm()
{
    BYTE i;
    for (i=0;i<ALARM_INO;i++)
    {
        if (GetAlarm(i))
        {
            return TRUE;
        }
    }

    for (i=ALARM_LE;i<ALARM_BAT_LOW;i++)
    {
        if (GetAlarm(i))
        {
            return TRUE;
        }
    }

    return FALSE;
}



// 报警声光控制
// 根据不同报警发出不同的声音和灯光
void AlarmSLCtl()
{
    BYTE i;
    for (i=0;i<32;i++)
    {
        if (SysAlarm.Mask1 & (1l<<i))
        {
            if (SysAlarm.Status1 & (1l<<i))
            {
                StartAlarmLed(AlarmSound[i].OnTime, AlarmSound[i].OffTime);

                if (InnerParam.Sound == 1)
                {
                    StartBeep(AlarmSound[i].OnTime, AlarmSound[i].OffTime);
                }
                return;  // 只能发出一种声音和灯光
            }
        }
    }

    for (i=0;i<32;i++)
    {
        if (SysAlarm.Mask2 & (1l<<i))
        {
            if (SysAlarm.Status2 & (1l<<i))
            {
                StartAlarmLed(AlarmSound[i+32].OnTime, AlarmSound[i+32].OffTime);

                if (InnerParam.Sound == 1)
                {
                    StartBeep(AlarmSound[i+32].OnTime, AlarmSound[i+32].OffTime);
                }
                return;  // 只能发出一种声音和灯光
            }
        }
    }

    StopBeep();
    StopAlarmLed();
}

void SetAlarmLog(BYTE Alarm)
{
    if (Alarm >= ALARM_BAT_LOW)
    {
        StorageLog(Alarm-30);
    }
}

void ClrAlarmLog(BYTE Alarm)
{
    if (Alarm >= ALARM_BAT_LOW)
    {
        StorageLog(Alarm-20);
    }
}

void SetAlarm(BYTE Alarm)
{
    if (Alarm < 32)
    {
        //if (SysAlarm.Mask1 & (1l<<Alarm))   // 允许报警
        {
            if (0 == (SysAlarm.Status1 & (1l<<Alarm)))  // 原来没有这个报警
            {
                SysAlarm.Status1 |= (1l<<Alarm);

                //if (SysAlarm.Mask1 & (1l<<Alarm))   // 允许报警
                {
                    // 根据不同报警发出不同的声音和灯光
                    AlarmSLCtl();

                    //有些告警写入日志
                    SetAlarmLog(Alarm);
                }
            }
        }
    }
    else
    {
        Alarm -= 32;
        //if (SysAlarm.Mask2 & (1l<<Alarm))   // 允许报警
        {
            if (0 == (SysAlarm.Status2 & (1l<<Alarm)))  // 原来没有这个报警
            {
                SysAlarm.Status2 |= (1l<<Alarm);

                //if (SysAlarm.Mask2 & (1l<<Alarm))   // 允许报警
                {
                    // 根据不同报警发出不同的声音和灯光
                    AlarmSLCtl();
                    
                    SetAlarmLog(Alarm+32);
                }
            }
        }
    }
}

void ClrAlarm(BYTE Alarm)
{
    if (Alarm < 32)
    {
        //if (SysAlarm.Mask1 & (1l<<Alarm))   // 允许报警
        {
            if (0 != (SysAlarm.Status1 & (1l<<Alarm)))  // 原来有这个报警
            {
                SysAlarm.Status1 &= ~(1l<<Alarm);
                SysAlarm.Mask1 |= (1l<<Alarm);       // 重新允许报警
                //if (SysAlarm.Mask1 & (1l<<Alarm))  
                {
                    // 根据不同报警发出不同的声音和灯光
                    AlarmSLCtl();
                    
                    ClrAlarmLog(Alarm);
                }
            }
        }
    }
    else
    {
        Alarm -= 32;
        //if (SysAlarm.Mask2 & (1l<<Alarm))   // 允许报警
        {
            if (0 != (SysAlarm.Status2 & (1l<<Alarm)))  // 原来有这个报警
            {
                SysAlarm.Status2 &= ~(1l<<Alarm);
                SysAlarm.Mask2 |= (1l<<Alarm);     // 重新允许报警

                //if (SysAlarm.Mask2 & (1l<<Alarm))   // 允许报警
                {
                    // 根据不同报警发出不同的声音和灯光
                    AlarmSLCtl();
                    
                    ClrAlarmLog(Alarm+32);
                }
            }
        }
    }
}

BOOL GetValidAlarm(BYTE Alarm)
{
    if (Alarm < 32)
    {
        if (SysAlarm.Mask1 & (1l<<Alarm))   // 允许报警
        {
            if (0 != (SysAlarm.Status1 & (1l<<Alarm)))  // 原来有这个报警
            {
                return TRUE;
            }
        }
    }
    else
    {
        Alarm -= 32;
        if (SysAlarm.Mask2 & (1l<<Alarm))   // 允许报警
        {
            if (0 != (SysAlarm.Status2 & (1l<<Alarm)))  // 原来有这个报警
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}


// 不管掩码，只要有报警就返回
BOOL GetAlarm(BYTE Alarm)
{
    if (Alarm < 32)
    {
        //if ((SysAlarm.Mask1 & (1l<<Alarm)) == 0)  // 允许报警
        {
            if (0 != (SysAlarm.Status1 & (1l<<Alarm)))  // 原来有这个报警
            {
                return TRUE;
            }
        }
    }
    else
    {
        Alarm -= 32;
        //if ((SysAlarm.Mask2 & (1l<<Alarm)) == 0)   // 允许报警
        {
            if (0 != (SysAlarm.Status2 & (1l<<Alarm)))  // 原来有这个报警
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}


void DisableAlarm(BYTE Alarm)
{
    if (Alarm < 32)
    {
        SysAlarm.Mask1 &= ~(1l<<Alarm);
    }
    else
    {
        Alarm -= 32;
        SysAlarm.Mask2 &= ~(1l<<Alarm);
    }

    SysAlarm.MaskB1 = SysAlarm.Mask1;
    SysAlarm.MaskB2 = SysAlarm.Mask2;
    AlarmSLCtl();
}

void EnableAlarm(BYTE Alarm)
{
    if (Alarm < 32)
    {
        SysAlarm.Mask1 |= (1l<<Alarm);
    }
    else
    {
        Alarm -= 32;
        SysAlarm.Mask2 |= (1l<<Alarm);
    }

    SysAlarm.MaskB1 = SysAlarm.Mask1;
    SysAlarm.MaskB2 = SysAlarm.Mask2;
    AlarmSLCtl();
}


void DisableAllAlarm()
{
    SysAlarm.MaskB1 = SysAlarm.Mask1;
    SysAlarm.MaskB2 = SysAlarm.Mask2;
    
    SysAlarm.Mask1 = 0;
    SysAlarm.Mask2 = 0;
    
    AlarmSLCtl();
}

void EnableAllAlarm()
{
    SysAlarm.Mask1 = SysAlarm.MaskB1;
    SysAlarm.Mask2 = SysAlarm.MaskB2;
    
    AlarmSLCtl();
}

void InitAlarm()
{
    // 允许所有报警
    SysAlarm.Mask1 = 0xFFFFFFFF;
    SysAlarm.Status1 = 0;
    SysAlarm.Mask2 = 0xFFFFFFFF;
    SysAlarm.Status2 = 0;

    SysAlarm.MaskB1 = 0xFFFFFFFF;
    SysAlarm.MaskB2 = 0xFFFFFFFF;
}


