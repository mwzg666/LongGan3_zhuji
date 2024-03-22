#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <msp430.h>
#include "system.h"
#include "CommDataDefine.h"

#include "Senser.h"
#include "Mcp2510.h"
#include "Main.h"
#include "SsParam.h"
#include "SsAlarm.h"
#include "Input.h"
#include "MainMenu.h"
#include "Oled.h"
#include "Measinter.h"
#include "SelfCheck.h"
#include "SsSrv.h"
#include "RadioSet.h"
#include "BlueTooth.h"

extern INTERFACE currUI;
extern BYTE Can_RxInt;
extern CHECKRES CheckRes;
extern QPara Param;
extern SENSERPARAM ParamTmp;    // 参数临时存储
extern SENSERALARM AlarmTmp;
extern QInnerParam InnerParam;
extern char StrTemp[24];
extern BYTE  RadCount;     // 核素数量
extern RADIO pnuc[25];      //核素
extern BYTE BTRecvBuf[BT_BUF_LEN];
extern BYTE BTSendBuf[BT_BUF_LEN];
extern char StrHint[24];
extern BYTE Setting_SenserType;

WORD  CanRxCnt = 0;  // Can 超时计数器
CANID CanSendId;
CANID CanRecId;
BYTE CanSendData[8] = {0};
BYTE CanRecData[8] = {0};
BYTE CanRecBuf[14][8] = {0};
BATTERY SenserBatery;
BOOL InSenserEnable = TRUE;
BYTE ElecTestChannel = 0;

SENSER Senser;

const CMDINFO CmdInfo[] =
{
    {SCMD_FIND_SENSER   , 1},
    {SCMD_READ_PARAM    , 12},
    {SCMD_WRITE_PARAM   , 12},
    {SCMD_SAVE_PARAM    , 1},
    {SCMD_SET_STATUS    , 1},     // 5
    {SCMD_READ_STATUS   , 1},
    {SCMD_READ_COUNTER  , 11},
    {SCMD_SET_BAUDRATE  , 1},
    {SCMD_WRITE_ALARM   , 14},
    {SCMD_READ_ALARM    , 14},    // 10
    {SCMD_BACK_TEST     , 3},
    {SCMD_XL_FIX        , 1},
    {SCMD_GET_RADCNT    , 1},
    {SCMD_READ_RAD      , 6},
    {SCMD_WRITE_RAD     , 1},     // 15
};

char * const SenserName[] = 
{
    {""},        // 0
    {"内置探头"},
    {"PD"},
    {"GM"},
    {"@A(仅测@A)"},
    {"@B(仅测@B)"},   //5
    {"@A@B@Y探头"},
    {"地面1"},
    {"地面2"},
    {"NaI"},
    {""},         //A
    {"液体"},
    {""},
    {""},
    {""},
    {""},
    {""},         // 0x10
    {""},
    {""},
    {""},
    {"@A(仅测@B)"},
    {"@B(仅测@A)"},    // 0x15
    {""},       
    {""},
    {""},
    {""},
    {""},    // 0x1A
    {""},
    {""},   
    {""},
    {""},
    {""},
    {""},    //0x20
    {""},
    {""},
    {""},
    {"@A(@A+@B)"},
    {"@B(@A+@B)"},    // 0x25
    {""},    
    {""},
    {""},
    {""},
    {"长杆(@Y)"}       // 0x2A
};


const SENSERCONTXT SenserContxt[SENSERCOUNT] =
{                                               
    {
        SENSER_IN,                         // 0x01    // 内置探头  
        2, {0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},         // 参数              
        1, {0,0,0,0,0,0,0,1,0,0,0},              // 计数   
        6, {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},     //告警参数             
        {ALARM_INO, ALARM_IN1, ALARM_IN2, ALARM_IN3, ALARM_INH, 0,0,0,0,0},   // 使用的报警ID
        {ALARM_INO, ALARM_IN1, ALARM_IN2, ALARM_IN3, ALARM_INH, 0,0,0,0,0}
    },    

    {   
        SENSER_PD,                         // 0x02    //水下高量程探头（PD）
        3, {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0},    // 参数      
        1, {0,0,0,0,0,1,0,0,0,0,0},                       // 计数   
        2, {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},   //告警参数     
        {ALARM_PDO, ALARM_PD1, ALARM_PD2, ALARM_PDL, ALARM_PDH, 0,0,0,0,0},
        {ALARM_PDO, ALARM_PD1, ALARM_PD2, ALARM_PDL, ALARM_PDH, 0,0,0,0,0}
    },     

    {
        SENSER_GM,                        // 0x03    //水下中高量程探头（GM）
        6, {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0},    // 参数     
        1, {0,0,0,0,0,1,0,0,0,0,0},              // 计数   
        2, {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  //告警参数    
        {ALARM_GMO, ALARM_GM1, ALARM_GM2, ALARM_GML, ALARM_GMH, 0,0,0,0,0},
        {ALARM_GMO, ALARM_GM1, ALARM_GM2, ALARM_GML, ALARM_GMH, 0,0,0,0,0} 
    },     

    {
        SENSER_A_A,                      //  0x04    //α探头(仅测alphy)
        5, {1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},   
        1, {0,0,0,0,0,1,0,0,0,0,0},   
        1, {0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},   //告警参数     
        {ALARM_AO, ALARM_A1, ALARM_A2, ALARM_AL, ALARM_AH, 0,0,0,0,0},
        {ALARM_AO, ALARM_A1, ALARM_A2, ALARM_AL, ALARM_AH, 0,0,0,0,0}
    },    

    {
        SENSER_A_B,                      //  0x14    //α探头(仅测beta)
        5, {1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},   
        1, {0,0,0,1,0,0,0,0,0,0,0},   
        4, {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0},
        {ALARM_BO, ALARM_B1, ALARM_B2, ALARM_BL, ALARM_BH, 0,0,0,0,0},
        {ALARM_BO, ALARM_B1, ALARM_B2, ALARM_BL, ALARM_BH, 0,0,0,0,0}
    },    

    {
        SENSER_A_AB,                    // 0x24    //α探头(可测alphy+beta)
        6, {1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0},       // 参数     
        2, {0,0,0,0,0,1,1,0,0,0,0},           // 计数   
        8, {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0},
        {ALARM_AO, ALARM_A1, ALARM_A2, ALARM_AL, ALARM_AH, 0,0,0,0,0},
        {ALARM_BO, ALARM_B1, ALARM_B2, ALARM_BL, ALARM_BH, 0,0,0,0,0}
    },   


    {
        SENSER_ABY,                    // 0x6    //αβγ探头
        1, {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},   // 参数         
        1, {0,0,0,0,0,1,0,0,0,0,0},        // 计数                   
        2, {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        //{ALARM_ABO, ALARM_AB1, ALARM_AB2, ALARM_ABL, ALARM_ABH, 0,0,0,0,0},
        //{ALARM_ABO, ALARM_AB1, ALARM_AB2, ALARM_ABL, ALARM_ABH, 0,0,0,0,0}
        {ALARM_PDO, ALARM_PD1, ALARM_PD2, ALARM_PDL, ALARM_PDH, 0,0,0,0,0},
        {ALARM_PDO, ALARM_PD1, ALARM_PD2, ALARM_PDL, ALARM_PDH, 0,0,0,0,0}
    }, 

    {
        SENSER_B_B,                       // 0x05    //β探头(仅测beta)
        6, {1,0,1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1},   
        1, {0,0,0,0,0,0,1,0,0,0,0},   
        1, {0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0},  //告警参数     
        {ALARM_BO, ALARM_B1, ALARM_B2, ALARM_BL, ALARM_BH, 0,0,0,0,0},
        {ALARM_BO, ALARM_B1, ALARM_B2, ALARM_BL, ALARM_BH, 0,0,0,0,0}
    },    

    {
        SENSER_B_A,                        // 0x15    //β探头(仅测alphy)
        5, {1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},   
        1, {0,0,1,0,0,0,0,0,0,0,0},   
        4, {1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0},
        {ALARM_AO, ALARM_A1, ALARM_A2, ALARM_AL, ALARM_AH, 0,0,0,0,0},
        {ALARM_AO, ALARM_A1, ALARM_A2, ALARM_AL, ALARM_AH, 0,0,0,0,0}
    },    

    {
        SENSER_B_AB,                         // 0x25    //β探头(可测alphy+beta)
        6, {1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0},       // 参数     
        2, {0,0,0,0,0,1,1,0,0,0,0},           // 计数   
        8, {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0},  //告警
        {ALARM_AO, ALARM_A1, ALARM_A2, ALARM_AL, ALARM_AH, 0,0,0,0,0},
        {ALARM_BO, ALARM_B1, ALARM_B2, ALARM_BL, ALARM_BH, 0,0,0,0,0}
    },    

    {
        SENSER_G1,                         // 0x07    //地面探头1
        //4, {1,1,1,1,0,0,0,0,0,0,0,0},   
        6, {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},   // 参数     
        2, {0,0,0,0,0,1,1,0,0,0,0},       // 计数   
        8, {1,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0},  //告警
        {ALARM_GDO, ALARM_GD1, ALARM_GD2, ALARM_GDL, ALARM_GDH, 0,0,0,0,0},
        {ALARM_GDO, ALARM_GD1, ALARM_GD2, ALARM_GDL, ALARM_GDH, 0,0,0,0,0}
    },    

    {
        SENSER_G2,                          // 0x08    //地面探头2
        //4, {1,1,1,1,0,0,0,0,0,0,0,0},   
        6, {1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},   // 参数     
        2, {0,0,0,0,0,1,1,0,0,0,0},       // 计数   
        8, {1,1,1,0,1,1,1,0,1,1,0,0,0,0,0,0},  //告警
        {ALARM_GDO, ALARM_GD1, ALARM_GD2, ALARM_GDL, ALARM_GDH, 0,0,0,0,0},
        {ALARM_GDO, ALARM_GD1, ALARM_GD2, ALARM_GDL, ALARM_GDH, 0,0,0,0,0}
    },     

    {
        SENSER_Y,                         // 0x09    //外部NaI(Y) 探头         
        7, {1,1,0,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0},   // 参数     
        2, {0,0,0,0,0,0,0,0,1,1,0},       // 计数   
        6, {0,0,0,0,0,0,1,1,0,0,1,1,0,1,1,0},     //告警
        {ALARM_YO, ALARM_Y1, ALARM_Y2, ALARM_INL, ALARM_INH, 0,0,0,0,0}, // ALARM_YL, ALARM_YH} 
        {ALARM_YO, ALARM_Y1, ALARM_Y2, ALARM_INL, ALARM_INH, 0,0,0,0,0}
    }, 

    {
        SENSER_LP,                         // 0x2A    //长杆Y 探头         
        7, {0,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0},   // 参数     
        2, {0,0,0,0,0,0,0,0,1,1,0},       // 计数   
        2, {0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0},     //告警
        {ALARM_YO, ALARM_Y1, ALARM_Y2, ALARM_INL, ALARM_INH, ALARM_LE,ALARM_HE,ALARM_DE,0,0}, // ALARM_YL, ALARM_YH} 
        {ALARM_YO, ALARM_Y1, ALARM_Y2, ALARM_INL, ALARM_INH, ALARM_LE,ALARM_HE,ALARM_DE,0,0}
    },

    {
        SENSER_LC,                         // 0x0B    // 液体探测 -- 蓝牙 
        5, {1,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0},   // 参数     
        2, {0,0,0,0,0,1,1,0,0,0,0},                         // 计数   
        8, {1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0},              //告警
        {ALARM_AO, ALARM_A1, ALARM_A2, ALARM_AL, ALARM_AH, 0,0,0,0,0},
        {ALARM_BO, ALARM_B1, ALARM_B2, ALARM_BL, ALARM_BH, 0,0,0,0,0}
    },
};

#if 0
//  参数默认值
const SENSERPARAM SsDef[SENSERCOUNT] =
{   //Hv   Z1    Ct       Hd   Z2   Z3      S1   S2   Cr  Hn  S3     Z4 
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
    {800, 100, 1000, 3,  50,  0,   0,  0,  1, 0, 10000, 0},
};

// 最大值
const SENSERPARAM SsMax[SENSERCOUNT] =
{
    //Hv    Z1          Ct         Hd     Z2         Z3         S1          S2            Cr  Hn   S3         Z4
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
    {1000, 2500,   1000, 10,  2500,   2500,   100000, 100000,  10, 24, 100000, 2500},
};


// 最小值
const SENSERPARAM SsMin[SENSERCOUNT] =
{
    //Hv   Z1    Ct     Hd   Z2   Z3      S1    S2      Cr   Hn  S3   Z4  
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
    {500, 0,   200, 1,  0,   0,    0,   0,    0,  0, 100, 0},
};
#else
const SENSERPARAM SsDef[] =
{   
    //Hv   Z1    Ct       Hd   Z2   Z3      S1   S2   Cr  Hn  S3     Z4 
    {800, 100, 1000, 3,  50,  0,   1,  0,  1, 0, 10000, 0,      10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},
};

const SENSERPARAM SsMax[] =
{
    //Hv    Z1     Ct         Hd     Z2         Z3         S1          S2            Cr  Hn   S3         Z4
    {1000, 2500,  9999, 10,  2500,   2500,   10.0,  100000,  10, 24, 100000, 2500, 100, 99, 9.9, 100, 99, 9.9, 100, 9999, 1},
};

const SENSERPARAM SsMin[] =
{
    //Hv   Z1    Ct     Hd   Z2   Z3      S1    S2    Cr   Hn  S3   Z4  
    {500,  0,    0,     1,   0,   0,     0,     0,    0,   0,  100,  0,  1, 0, 0, 0, 0, 0, 0, 1, 0},
};

#endif

// 参数默认值
// 参数保存在探头，本地不用保存了(除内置探头)
const SENSER SsDefault[SENSERCOUNT] = 
{
    {   // 内置探头--- 参数要保存在本地
        &SenserContxt[0],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,1000,0,0,0,0.0,0.0,1,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm  阀值
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        1,     // WorkStatus
        {0,1,0,0,0}, // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100        // baudrate
    },

    {   //水下高量程探头（PD）
        &SenserContxt[1],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {0,1,0,0,0},        // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   //水下中高量程探头（GM）
        &SenserContxt[2],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {0,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   //α探头(仅测alphy)
        &SenserContxt[3],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {1,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   //α探头(仅测beta)
        &SenserContxt[4],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {1,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {    //α探头(可测alphy+beta)
        &SenserContxt[5],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {1,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },


    {     //αβγ探头
        &SenserContxt[6],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {1,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   //β探头(仅测beta)
        &SenserContxt[7],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 1},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {1,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   //β探头(仅测alphy)
        &SenserContxt[8],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {1,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   //β探头(可测alphy+beta)
        &SenserContxt[9],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {1,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   //地面探头1
        &SenserContxt[10],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {1,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   //地面探头2
        &SenserContxt[11],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {1,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   // 外部Y 探头
        &SenserContxt[12],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},         // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},             // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {0,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   // 长杆Y 探头
        &SenserContxt[13],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},                        // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},         // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {0,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },

    {   // 液体探头
        &SenserContxt[14],                  // Contxt
        &SsDef[0],
        &SsMax[0],
        &SsMin[0],
        {0,0,0,0,0,0,0.0,0.0,0,0,100,0, 10, 0.43, 0.00018, 1, 7.5, 0.000019, 1, 300, 0},                        // Param
        {0,0,0,0,0,0,0,0,0,0,100,200,500,1000,2000,5},         // Alarm
        {0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0, 0,0},     // Counter
        0,   // WorkStatus
        {0,1,0,0,0},     // 本底测量
        //{0,0,0,0}, // 效率标定
        //{0,0},     // 核素效率
        //100       // baudrate
    },
};

// 新增探头的时候注意修改下面三个函数

// 兼容两种协议
BOOL IsAscllCode()
{
    if ((CheckRes.SenserType  == SENSER_G1) ||
        (CheckRes.SenserType  == SENSER_G2)  ||
        (CheckRes.SenserType  == SENSER_B_A) ||
        (CheckRes.SenserType  == SENSER_B_B) ||
        (CheckRes.SenserType  == SENSER_A_AB) ||
        (CheckRes.SenserType  == SENSER_B_AB) ||
        (CheckRes.SenserType  == SENSER_ABY) //   ||
        // (CheckRes.SenserType  == SENSER_GM)   ||
        // (CheckRes.SenserType  == SENSER_PD)
        )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// 兼容两种核素管理
BOOL HaveRadSet()
{
    if ((CheckRes.SenserType == SENSER_A_A)  || 
        (CheckRes.SenserType == SENSER_B_B)  ||
        (CheckRes.SenserType == SENSER_G1)   ||
        (CheckRes.SenserType == SENSER_G2)   ||
        (CheckRes.SenserType == SENSER_A_AB) ||
        (CheckRes.SenserType == SENSER_B_AB) ||
        (CheckRes.SenserType == SENSER_ABY)  ||
        (CheckRes.SenserType == SENSER_B_A)  )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// 核素报警阈值
BYTE OneThres()
{
    if ((CheckRes.SenserType == SENSER_G1) ||
        (CheckRes.SenserType == SENSER_G2) ||
        (CheckRes.SenserType == SENSER_A_AB) ||
        (CheckRes.SenserType == SENSER_B_AB) ||
        (CheckRes.SenserType == SENSER_ABY)
        )
    {
        if (InnerParam.RadioSelIndex == 0)  // 混合场
        {
            return FALSE;
        }
    }

    return TRUE;
}



BYTE GetSenserIndex(BYTE type)
{
    BYTE i;
    for (i=0;i<SENSERCOUNT;i++)
    {
        if (type == SenserContxt[i].Type)
        {
            return i;
        }
    }
    return 0xFF;
}



// 外部探头
void InitSenser()
{
    BYTE i;
    memset((BYTE *)&Senser,0,sizeof(Senser));

    // 初始化默认参数
    for (i=0;i<SENSERCOUNT;i++)
    {
        if (CheckRes.SenserType == SenserContxt[i].Type)
        {
            memcpy(&Senser,&SsDefault[i],sizeof(Senser));
            break;
        }
    }

    if (CheckRes.SenserType != InnerParam.CurrSenserType)
    {
        InnerParam.CurrSenserType = CheckRes.SenserType;

        // 默认本底
        memcpy(&Param.Gnd,&Senser.Gnd,sizeof(Param.Gnd));
    }   
    else
    {
        // 没有更换探头就用原来的本底
        memcpy(&Senser.Gnd,&Param.Gnd,sizeof(Param.Gnd));
    }

    if (CheckRes.CommInterface == COMM_BT)
    {
        BTSenserPower(1);
        Sleep(3000);
        BTGetBattary(10);
    }
}

BYTE GetRetFrameCnt(BYTE Cmd)
{
    BYTE i;
    for (i=0;i<SCMD_COUNT;i++)
    {
        if (CmdInfo[i].Cmd == Cmd)
        {
            if (Cmd == SCMD_READ_COUNTER)
            {
                return Senser.Contxt->CounterCnt;
            }
            else
            {
                return CmdInfo[i].RetFrameCnt;
            }
        }
    }
    return 0;
}
 
// 接收到指定个返回帧就返回
// 没有返回帧超时返回                                       
BYTE CanWaitCmd(BYTE Cmd)
{
    BOOL   Tr,Ex;
    BYTE   Len;
    DWORD  Id;
    BYTE   FrmCnt;  // 希望返回帧数量
    BYTE   RetCnt;  // 实际返回帧数量

    memset(&CanRecBuf,0,sizeof(CanRecBuf));
    Id = 0;
    Len = 0;
    RetCnt = 0;
    CanRxCnt = 0;
    FrmCnt = GetRetFrameCnt(Cmd);
    
    while(CanRxCnt < CAN_TIMEOUT)  
    {
        SystemTask();
        
        if (Can_RxInt)
        {
            Can_RxInt = 0;
            CanRxCnt = 0;    // 重新开始计时
            
            memset(CanRecData,0,8);
            if (canRead(0,&Id,CanRecData,&Len,&Tr,&Ex))
            {
                CanRecId.Id = Id;
                
                if (CanRecId.Cmd == Cmd)
                {
                    if (Len > 0)
                    {
                        memcpy(&CanRecBuf[RetCnt][0],CanRecData,Len);
                    }
                    RetCnt ++;

                    if ((FrmCnt == RetCnt) || (RetCnt > 16))
                    {
                        return RetCnt; // 返回接收到帧的个数
                    }
                }
            }
        }

        
    }

    // 超时了再读一下，清中断
    //canRead(0,&Id,CanRecData,&Len,&Tr,&Ex);
    return RetCnt;  // 返回接收到帧的个数
}

BYTE CanReadCmd(BYTE Cmd)
{
    BOOL Tr,Ex;
    U8   Len;
    U32  Id;
    BYTE CmdCnt;

    memset(&CanRecBuf,0,sizeof(CanRecBuf));
    Id = 0;
    Len = 0;
    CmdCnt = 0;
    //CanRxCnt = 0;
    //while(CanRxCnt < CAN_TIMEOUT)  
    {
        if (Can_RxInt)
        {
            Can_RxInt = 0;
            //CanRxCnt = 0;    // 重新开始计时
            
            memset(CanRecData,0,8);
            if (canRead(0,&Id,CanRecData,&Len,&Tr,&Ex))
            {
                CanRecId.Id = Id;
                
                if (CanRecId.Cmd == Cmd)
                {
                    if (Len > 0)
                    {
                        memcpy(&CanRecBuf[CmdCnt][0],CanRecData,Len);
                    }
                    CmdCnt ++;

                    //if (Ctu == FALSE) return CmdCnt;
                }
            }
        }

        //SystemTask();
    }
    
    
    return CmdCnt;
}

void CanSendCmd(BYTE Cmd, BYTE Addr, BYTE Len)
{
    CanSendId.Res = 0;
    CanSendId.Cmd = Cmd;
    CanSendId.Des = Addr;
    CanSendId.Src = SENSER_MASTER;  // 主机
    canWrite(CanSendId.Id,CanSendData,Len,1,0);
}


DWORD CanDataToDword(BYTE *Data)
{
    DWORD ret;
    ret = (Data[0])*0x1000000l +
          (Data[1])*0x10000l +
          (Data[2])*0x100l +
          (Data[3]);
    return ret;
}


float AscllToFloat(BYTE *Data)
{
    float ret;
    // 从1到7的6个字节

    BYTE i;

    // 找小数点的位置
    for (i=1;i<7;i++)
    {
        if (Data[i] == '.')
        {
            break;
        }
    }

    ret = 0;
    
    switch(i) 
    {
        case 7:   // 没有小数点
        {
            ret = (Data[1] - 0x30) * 100000 +
                  (Data[2] - 0x30) * 10000 +
                  (Data[3] - 0x30) * 1000 +
                  (Data[4] - 0x30) * 100 +
                  (Data[5] - 0x30) * 10 +
                  (Data[6] - 0x30) ;
            break;  
        }
        
        case 6:   break;  // 最后一位为小数也是非法
        
        case 5:   
        {
            ret = (Data[1] - 0x30) * 1000 +
                  (Data[2] - 0x30) * 100 +
                  (Data[3] - 0x30) * 10 +
                  (Data[4] - 0x30) +
                  (Data[6] - 0x30) * 0.1;
            break;
        }
        
        case 4: 
        {
            ret = (Data[1] - 0x30) * 100 +
                  (Data[2] - 0x30) * 10 +
                  (Data[3] - 0x30) +
                  (Data[5] - 0x30) * 0.1 +
                  (Data[6] - 0x30) * 0.01;
            break;
        }
        
        case 3: 
        {
            ret = (Data[1] - 0x30) * 10 +
                  (Data[2] - 0x30) +
                  (Data[4] - 0x30) * 0.1 + 
                  (Data[5] - 0x30) * 0.01 +
                  (Data[6] - 0x30) * 0.001;
            break;
        }
        
        case 2:   
        {
            ret = (Data[1] - 0x30) +
                  (Data[3] - 0x30) * 0.1 +
                  (Data[4] - 0x30) * 0.01 + 
                  (Data[5] - 0x30) * 0.001 +
                  (Data[6] - 0x30) * 0.0001;
            break;
        }
        
        case 1:   break; // 应该属于非法浮点
    }
    
    
    return ret;
}


float CanDataToFloat(BYTE *Data)
{
    float ret;

    if (IsAscllCode())
    {
        #if 1
        memset(StrTemp,0,24);
        memcpy(StrTemp, &Data[1], 6);
        ret = atof(StrTemp);
        #else
        ret = AscllToFloat(Data);
        #endif
    }
    else
    {
        memcpy(&ret,&Data[3],4);
    }
    return ret;
}


BYTE GetAlarmBit(BYTE Alm)
{
    #if 0
    if (IsAscllCode())
    {
        return (Alm-0x30);
    }
    else
    {
        return Alm;
    }
    #else
    return (Alm-0x30);
    #endif
}


void FloatToCanData(float dat, BYTE pc)
{
    BYTE Len;
    
    memset(StrTemp,0,24);

    if (pc == 0)
    {
        sprintf(StrTemp,"%.0f",dat);

        Len = strlen(StrTemp);
        if (Len > 6)
        {
            memset(&CanSendData[1],0x39,6);
        }
        else
        {
            memset(&CanSendData[1],0x30,6);
            memcpy(&CanSendData[1+(6-Len)],StrTemp,Len);
        }
    }
    else if (pc == 1)
    {
        sprintf(StrTemp,"%.1f",dat);

        Len = strlen(StrTemp);
        if (Len > 6)
        {
            // 9999.9
            memset(&CanSendData[1],0x39,6);
            CanSendData[5] = '.';
        }
        else
        {
            memset(&CanSendData[1],0x30,6);
            memcpy(&CanSendData[1+(6-Len)],StrTemp,Len);
        }
    }
    else if (pc == 2)
    {
        sprintf(StrTemp,"%.2f",dat);

        Len = strlen(StrTemp);
        if (Len > 6)
        {
            // 999.99
            memset(&CanSendData[1],0x39,6);
            CanSendData[4] = '.';
        }
        else
        {
            memset(&CanSendData[1],0x30,6);
            memcpy(&CanSendData[1+(6-Len)],StrTemp,Len);
        }
    }
    else
    {
        // 不支持
    }
    
}


// pc : 保留小数点后面的位数
BYTE SenserSendFloat(BYTE Cmd, BYTE Index, float val, BYTE pc)
{
    //static BYTE ret;
    BYTE bv[4] = {0};

    memset(CanSendData,0,8);
    CanSendData[0] = Index;
    if(IsAscllCode())
    {
        // 0 -- 999999
        FloatToCanData(val,pc);
    }
    else
    {
        memcpy(bv,&val,4);
        CanSendData[1] = 0;
        CanSendData[2] = 0;
        CanSendData[3] = bv[0];
        CanSendData[4] = bv[1];
        CanSendData[5] = bv[2];
        CanSendData[6] = bv[3];
    }
    CanSendData[7] = 0;   // res
    CanSendCmd(Cmd,CheckRes.SenserType,8);
    return CanWaitCmd(Cmd);  // 返回接收到帧的个数

    
    //if (ret == 0)
    //{
    //    return FALSE;
    //}
    //return TRUE;
}


BYTE SenserSendFloatEx(BYTE Cmd, BYTE Index, float val, BYTE pc)
{
    //static BYTE ret;
    BYTE bv[4] = {0};

    memset(CanSendData,0,8);
    CanSendData[0] = Index;
    
    memcpy(bv,&val,4);
    CanSendData[1] = 0;
    CanSendData[2] = 0;
    CanSendData[3] = bv[0];
    CanSendData[4] = bv[1];
    CanSendData[5] = bv[2];
    CanSendData[6] = bv[3];
    
    CanSendData[7] = 0;   // res
    CanSendCmd(Cmd,CheckRes.SenserType,8);
    return CanWaitCmd(Cmd);  // 返回接收到帧的个数

    
    //if (ret == 0)
    //{
    //    return FALSE;
    //}
    //return TRUE;
}


BOOL SenserSendDword(BYTE Cmd, DWORD val)
{
    return TRUE;
}

BOOL SenserSendByte(BYTE Cmd, BYTE Index, BYTE val)
{
    CanSendData[0] = Index;   
    CanSendData[1] = val;
    CanSendCmd(Cmd,CheckRes.SenserType,2);
    return CanWaitCmd(Cmd);   
}

void CheckSenserParam()
{
    if ((Senser.Param.Hv < Senser.ParamMin->Hv) ||
        (Senser.Param.Hv > Senser.ParamMax->Hv))
    {
        Senser.Param.Hv = Senser.ParamDef->Hv;
    }

    if ((Senser.Param.Z1 < Senser.ParamMin->Z1) ||
        (Senser.Param.Z1 > Senser.ParamMax->Z1))
    {
        Senser.Param.Z1 = Senser.ParamDef->Z1;
    }

    if ((Senser.Param.Ct < Senser.ParamMin->Ct) ||
        (Senser.Param.Ct > Senser.ParamMax->Ct))
    {
        Senser.Param.Ct = Senser.ParamDef->Ct;
    }

    if ((Senser.Param.Hd < Senser.ParamMin->Hd) ||
        (Senser.Param.Hd > Senser.ParamMax->Hd))
    {
        Senser.Param.Hd = Senser.ParamDef->Hd;
    }

    if ((Senser.Param.Z2 < Senser.ParamMin->Z2) ||
        (Senser.Param.Z2 > Senser.ParamMax->Z2))
    {
        Senser.Param.Z2 = Senser.ParamDef->Z2;
    }

    if ((Senser.Param.Z3 < Senser.ParamMin->Z3) ||
        (Senser.Param.Z3 > Senser.ParamMax->Z3))
    {
        Senser.Param.Z3 = Senser.ParamDef->Z3;
    }

    if ((Senser.Param.Z4 < Senser.ParamMin->Z4) ||
        (Senser.Param.Z4 > Senser.ParamMax->Z4))
    {
        Senser.Param.Z4 = Senser.ParamDef->Z4;
    }

    if ((Senser.Param.S1 < Senser.ParamMin->S1) ||
        (Senser.Param.S1 > Senser.ParamMax->S1))
    {
        Senser.Param.S1 = Senser.ParamDef->S1;
    }

    if ((Senser.Param.S2 < Senser.ParamMin->S2) ||
        (Senser.Param.S2 > Senser.ParamMax->S2))
    {
        Senser.Param.S2 = Senser.ParamDef->S2;
    }

    if ((Senser.Param.Cr < Senser.ParamMin->Cr) ||
        (Senser.Param.Cr > Senser.ParamMax->Cr))
    {
        Senser.Param.Cr = Senser.ParamDef->Cr;
    }

    if ((Senser.Param.Hn < Senser.ParamMin->Hn) ||
        (Senser.Param.Hn > Senser.ParamMax->Hn))
    {
        Senser.Param.Hn = Senser.ParamDef->Hn;
    }
}

BOOL HndSenserParam(BYTE Cnt)
{
    BYTE i;
    if (Cnt == 0) return FALSE;
    
    for (i=0;i<Cnt;i++)
    {
        switch (CanRecBuf[i][0])
        {
            case 1:  Senser.Param.Hv = CanDataToFloat(&CanRecBuf[i][0]);        break;
            case 2:  Senser.Param.Z1 = 2500 - CanDataToFloat(&CanRecBuf[i][0]); break;
            case 3:  Senser.Param.Ct = CanDataToFloat(&CanRecBuf[i][0]);        break;
            case 4:  Senser.Param.Hd = CanDataToFloat(&CanRecBuf[i][0]);        break;
            case 5:  Senser.Param.Z2 = 2500 - CanDataToFloat(&CanRecBuf[i][0]); break;
            case 6:  Senser.Param.Z3 = 2500 - CanDataToFloat(&CanRecBuf[i][0]); break;
            case 7:  Senser.Param.S1 = CanDataToFloat(&CanRecBuf[i][0]);         break;
            case 8:  Senser.Param.S2 = CanDataToFloat(&CanRecBuf[i][0]);         break;
            case 9:  Senser.Param.Cr = CanDataToFloat(&CanRecBuf[i][0]);         break;
            case 10: Senser.Param.Hn = (BYTE)CanDataToFloat(&CanRecBuf[i][0]);   break;
            case 11: Senser.Param.S3 = CanDataToFloat(&CanRecBuf[i][0]);         break;
            case 12: Senser.Param.Z4 = 2500 - CanDataToFloat(&CanRecBuf[i][0]);  break;
            case 13: Senser.Param.Pt = CanDataToFloat(&CanRecBuf[i][0]);         break;
            case 20: Senser.Param.Ss = CanDataToFloat(&CanRecBuf[i][0]);         break;
            case 21: Senser.Param.Bk = (BYTE)CanDataToFloat(&CanRecBuf[i][0]);   break;

            // 14 (0x0E) -- 19 (0x13) 为 水下 GM 探头 --- 浮点型，二进制
            case 14: memcpy(&Senser.Param.LA, &CanRecBuf[i][3], 4);  break;
            case 15: memcpy(&Senser.Param.LB, &CanRecBuf[i][3], 4);  break;
            case 16: memcpy(&Senser.Param.LC, &CanRecBuf[i][3], 4);  break;
            case 17: memcpy(&Senser.Param.HA, &CanRecBuf[i][3], 4);  break;
            case 18: memcpy(&Senser.Param.HB, &CanRecBuf[i][3], 4);  break;
            case 19: memcpy(&Senser.Param.HC, &CanRecBuf[i][3], 4);  break;

            #if 0   // 同上面的共用，不单独增加了
            // 新增 32(0x20)  -- 40(0x28)  为水下 PD 探头
            case 0x20:   break;
            case 0x21:   break;
            case 0x22:   break;
            case 0x23:   break;
            case 0x24:   break;
            case 0x25:   break;
            case 0x26:   break;
            case 0x27:   break;
            case 0x28:   break;
            #endif
        }
        
    }

    CheckSenserParam();
    
    return TRUE;
}

void BTHndParam()
{
    memcpy(&Senser.Param, &BTRecvBuf[4], sizeof(SENSERPARAM));

    // 阈值要特殊处理
    Senser.Param.Z1 = 2500 - Senser.Param.Z1;
    Senser.Param.Z2 = 2500 - Senser.Param.Z2;
    Senser.Param.Z3 = 2500 - Senser.Param.Z3;
    Senser.Param.Z4 = 2500 - Senser.Param.Z4;
    CheckSenserParam();
}

void BTHndAlarm()
{
    memcpy(&Senser.Alarm, &BTRecvBuf[4], sizeof(SENSERALARM));
}

BOOL HndSenserAlarm(BYTE Cnt)
{
    BYTE i;
    if (Cnt == 0) return FALSE;
    for (i=0;i<Cnt;i++)
    {
        switch (CanRecBuf[i][0])
        {
            case 1:  Senser.Alarm.A1 = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 2:  Senser.Alarm.A2 = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 3:  Senser.Alarm.Al = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 4:  Senser.Alarm.Ah = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 5:  Senser.Alarm.B1 = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 6:  Senser.Alarm.B2 = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 7:  Senser.Alarm.Bl = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 8:  Senser.Alarm.Bh = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 9:  Senser.Alarm.Ac = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 10: Senser.Alarm.Bc = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 11: Senser.Alarm.Y1 = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 12: Senser.Alarm.Y2 = CanDataToFloat(&CanRecBuf[i][0]); break;
            // 外部探头暂时没有Y3;
            //case 13: Senser.Alarm.Y3 = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 13: Senser.Alarm.Ma = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 14: Senser.Alarm.Mx = CanDataToFloat(&CanRecBuf[i][0]); break;
        }
    }
    return TRUE;
}


void ClearOtherAlarm(BYTE Index, BYTE Id)
{
    if (Index == 0)
    {
        if (Id != OVL) ClrAlarm(Senser.Contxt->AlarmId[OVL]);
        if (Id != NO1) ClrAlarm(Senser.Contxt->AlarmId[NO1]);
        if (Id != NO2) ClrAlarm(Senser.Contxt->AlarmId[NO2]);
        if (Id != BGL) ClrAlarm(Senser.Contxt->AlarmId[BGL]);
        if (Id != BGH) ClrAlarm(Senser.Contxt->AlarmId[BGH]);
        if (Id != LE)  ClrAlarm(Senser.Contxt->AlarmId[LE]);
        if (Id != HE)  ClrAlarm(Senser.Contxt->AlarmId[HE]);
        if (Id != DE)  ClrAlarm(Senser.Contxt->AlarmId[DE]);
    }
    else
    {
        if (Id != OVL) ClrAlarm(Senser.Contxt->AlarmId2[OVL]);
        if (Id != NO1) ClrAlarm(Senser.Contxt->AlarmId2[NO1]);
        if (Id != NO2) ClrAlarm(Senser.Contxt->AlarmId2[NO2]);
        if (Id != BGL) ClrAlarm(Senser.Contxt->AlarmId2[BGL]);
        if (Id != BGH) ClrAlarm(Senser.Contxt->AlarmId2[BGH]);
        if (Id != LE)  ClrAlarm(Senser.Contxt->AlarmId2[LE]);
        if (Id != HE)  ClrAlarm(Senser.Contxt->AlarmId2[HE]);
        if (Id != DE)  ClrAlarm(Senser.Contxt->AlarmId2[DE]);
    }
}


BYTE GetCounterAlarm(BYTE Index, BYTE Alarm)
{
    BYTE AlmId;
    if (Index == 0) 
    {
        AlmId = Senser.Contxt->AlarmId[Alarm];
    }
    else 
    {
        AlmId = Senser.Contxt->AlarmId2[Alarm];
    }
    return AlmId;
}


// 根据探头类型设置相应的报警
// Index: 计数索引0/1
void UpdateAlarm(BYTE Index, BYTE St)
{
    BYTE AlmId = 0xFF;
    BYTE Id = 0xFF;
    
    switch (St)
    {
        case 0:  // 无报警
        {
            // 清楚所有报警
            ClearOtherAlarm(Index,0xFF);
            return; 
        }
        
        case 1:  // 一级报警
        {         
            AlmId = GetCounterAlarm(Index, NO1);
            Id = NO1;
            break;
        }
        
        case 2:  // 二级报警
        {
            AlmId = GetCounterAlarm(Index, NO2);
            Id = NO2;
            break;
        }
        
        case 3: // 低本底报警
        {
            AlmId = GetCounterAlarm(Index, BGL);
            Id = BGL;
            break;
        }
        
        case 4:  // 高本底报警
        {
            AlmId = GetCounterAlarm(Index, BGH);
            Id = BGH;
            break;
        }
        
        case 0xFF:  case 0x16:  // "F"   //过载报警
        {
            AlmId = GetCounterAlarm(Index, OVL);
            Id = OVL;
            break;
        }

        case 6:
        {
            AlmId = GetCounterAlarm(Index, LE);
            Id = LE;
            break;
        }

        case 7:
        {
            AlmId = GetCounterAlarm(Index, HE);
            Id = HE;
            break;
        }

        case 8:
        {
            AlmId = GetCounterAlarm(Index, DE);
            Id = DE;
            break;
        }
    }

    if (AlmId != 0xFF)
    {
        ClearOtherAlarm(Index,Id);
        SetAlarm(AlmId);
    }
}


// 剂量率报警
void UpdateYyAlarm(BYTE St)
{
    switch (St)
    {
        case 0:  // 无报警
        {
            ClrAlarm(Senser.Contxt->AlarmId[OVL]);
            ClrAlarm(Senser.Contxt->AlarmId[NO1]);
            ClrAlarm(Senser.Contxt->AlarmId[NO2]);
            ClrAlarm(Senser.Contxt->AlarmId[BGL]);
            ClrAlarm(Senser.Contxt->AlarmId[BGH]);
            break; 
        }

        case 1:  
        {
            ClrAlarm(Senser.Contxt->AlarmId[OVL]);
            SetAlarm(Senser.Contxt->AlarmId[NO1]);
            ClrAlarm(Senser.Contxt->AlarmId[NO2]);
            ClrAlarm(Senser.Contxt->AlarmId[BGL]);
            ClrAlarm(Senser.Contxt->AlarmId[BGH]);
            break; 
        }

        case 2:  
        {
            ClrAlarm(Senser.Contxt->AlarmId[OVL]);
            ClrAlarm(Senser.Contxt->AlarmId[NO1]);
            SetAlarm(Senser.Contxt->AlarmId[NO2]);
            ClrAlarm(Senser.Contxt->AlarmId[BGL]);
            ClrAlarm(Senser.Contxt->AlarmId[BGH]);
            break; 
        }

        case 3:  // 低本底
        {
            ClrAlarm(Senser.Contxt->AlarmId[OVL]);
            ClrAlarm(Senser.Contxt->AlarmId[NO1]);
            ClrAlarm(Senser.Contxt->AlarmId[NO2]);
            SetAlarm(Senser.Contxt->AlarmId[BGL]);
            ClrAlarm(Senser.Contxt->AlarmId[BGH]);
            break;
        }

        case 4:  // 高本地
        {
            ClrAlarm(Senser.Contxt->AlarmId[OVL]);
            ClrAlarm(Senser.Contxt->AlarmId[NO1]);
            ClrAlarm(Senser.Contxt->AlarmId[NO2]);
            ClrAlarm(Senser.Contxt->AlarmId[BGL]);
            SetAlarm(Senser.Contxt->AlarmId[BGH]);
            break;
        }

        case 0xFF:  // 过载
        {
            SetAlarm(Senser.Contxt->AlarmId[OVL]);
            ClrAlarm(Senser.Contxt->AlarmId[NO1]);
            ClrAlarm(Senser.Contxt->AlarmId[NO2]);
            ClrAlarm(Senser.Contxt->AlarmId[BGL]);
            ClrAlarm(Senser.Contxt->AlarmId[BGH]);
            break; 
        }
    }
}


// 累计剂量报警
void UpdateYmAlarm(BYTE St)
{
    switch (St)
    {
        case 0:  // 无报警
        {
            //ClrAlarm(Senser.Contxt->AlarmId[OVL]);
            //ClrAlarm(Senser.Contxt->AlarmId[BGL]);
            //ClrAlarm(Senser.Contxt->AlarmId[BGH]);
            ClrAlarm(ALARM_YL);
            ClrAlarm(ALARM_YH);
            break; 
        }

        case 1:  
        {
            //ClrAlarm(Senser.Contxt->AlarmId[OVL]);
            //SetAlarm(Senser.Contxt->AlarmId[BGL]);
            //ClrAlarm(Senser.Contxt->AlarmId[BGH]);
            SetAlarm(ALARM_YL);
            ClrAlarm(ALARM_YH);
            break; 
        }

        case 2:  
        {
            //ClrAlarm(Senser.Contxt->AlarmId[OVL]);
            //ClrAlarm(Senser.Contxt->AlarmId[BGL]);
            //SetAlarm(Senser.Contxt->AlarmId[BGH]);
            ClrAlarm(ALARM_YL);
            SetAlarm(ALARM_YH);
            break; 
        }

        #if 0
        case 0xFF:  // 过载
        {
            SetAlarm(Senser.Contxt->AlarmId[OVL]);
            ClrAlarm(Senser.Contxt->AlarmId[BGL]);
            ClrAlarm(Senser.Contxt->AlarmId[BGH]);
            break; 
        }
        #endif
        
    }
}
 

#if 0
void UpdateAlarm(BYTE St)
{
    switch (CheckRes.SenserType)
    {
        case SENSER_IN:   InAlarm(St);      break;
        case SENSER_PD:   PdAlarm(St);      break;
        case SENSER_GM:   GmAlarm(St);      break;
        case SENSER_A_A:  A_Alarm(St);      break;
        case SENSER_A_B:  B_Alarm(St);      break;
        case SENSER_A_AB: AbAlarm(St);      break;
        case SENSER_B_B:  B_Alarm(St);      break;
        case SENSER_B_A:  A_Alarm(St);      break;
        case SENSER_B_AB: AbAlarm(St);      break;
        case SENSER_G1:   G_Alarm(St);      break;
        case SENSER_G2:   G_Alarm(St);      break;
    }
}
#endif


BOOL GetRadCnt()
{
    BYTE ret;

    if (CheckRes.CommInterface == COMM_CAN)
    {
        CanSendCmd(SCMD_GET_RADCNT,CheckRes.SenserType,0);
        ret = CanWaitCmd(SCMD_GET_RADCNT);
        if (ret == 1)
        {
            RadCount = (BYTE)CanDataToFloat(&CanRecBuf[0][0]);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}

void HndRadInfo(BYTE id)
{
    BYTE i;
    pnuc[id].Valid = 1;
    for (i=0;i<6;i++)
    {
        switch(CanRecBuf[i][0])
        {
            case 1: pnuc[id].index        = (BYTE)CanDataToFloat(&CanRecBuf[i][0]); break;
            case 2: pnuc[id].Channel      = (BYTE)CanDataToFloat(&CanRecBuf[i][0]); break;
            case 3: pnuc[id].Efficiency   = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 4: pnuc[id].AlarmThres   = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 5: pnuc[id].Active       = CanDataToFloat(&CanRecBuf[i][0]); break;
            case 6: pnuc[id].Unit         = (char)CanDataToFloat(&CanRecBuf[i][0]); break;
            //case 5: memcpy(pnuc[index].Name, &CanRecBuf[i][1], 6);                 break;
        }
    }
}

BOOL ReadAllRad()
{
    BYTE i,ret;
    if (CheckRes.CommInterface == COMM_CAN)
    {
        for (i=0;i<RadCount;i++)
        {
            ret = SenserSendFloat(SCMD_READ_RAD, 1, (float)i, 0);
            if (ret == 6)
            {
                HndRadInfo(i+1);
            }
            else
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL HndRadDataBase()
{
    if (!GetRadCnt())
    {
        return FALSE;
    }

    if (!ReadAllRad())
    {
        return FALSE;
    }

    return TRUE;
}

BOOL HndSenserCounter(BYTE Cnt)
{
    BYTE i;
    if (Cnt == 0) return FALSE;

    
    for (i=0;i<Cnt;i++)
    {
        switch (CanRecBuf[i][0])
        {
            case 1:  
            {
                Senser.Conter.Aj   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.AjSt = GetAlarmBit(CanRecBuf[i][7]);  
                
                break;
            }
            case 2:  
            {
                Senser.Conter.Hv   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.HvSt = GetAlarmBit(CanRecBuf[i][7]);  
                if (Senser.Conter.HvSt)
                {
                    SetAlarm(ALARM_HV_ERR);
                }
                else
                {
                    ClrAlarm(ALARM_HV_ERR);
                }
                break;
            }
            case 3:   // （α计数）
            {
                Senser.Conter.C1   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.C1St = GetAlarmBit(CanRecBuf[i][7]);  
                UpdateAlarm(i,Senser.Conter.C1St);
                break;
            }
            case 4:  // （β计数）和 地面
            {
                Senser.Conter.C2   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.C2St = GetAlarmBit(CanRecBuf[i][7]); 
                UpdateAlarm(i,Senser.Conter.C2St);
                break;
            }
            case 5:   // (α+β)
            {
                Senser.Conter.C3   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.C3St = GetAlarmBit(CanRecBuf[i][7]); 
                UpdateAlarm(i,Senser.Conter.C3St);
                break;
            }
            case 6:  // （α计数）
            {
                Senser.Conter.P1   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.P1St = GetAlarmBit(CanRecBuf[i][7]);  
                UpdateAlarm(i,Senser.Conter.P1St);
                break;
            }
            case 7:  //（β计数）
            {
                Senser.Conter.P2   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.P2St = GetAlarmBit(CanRecBuf[i][7]);  
                UpdateAlarm(i,Senser.Conter.P2St);
                break;
            }
            case 8: 
            {
                Senser.Conter.P3   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.P3St = GetAlarmBit(CanRecBuf[i][7]);  
                UpdateAlarm(i,Senser.Conter.P3St);
                break;
            }
            
            case 9:  // Y 剂量率
            {
                Senser.Conter.Cr   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.CrSt = GetAlarmBit(CanRecBuf[i][7]);  
                UpdateYyAlarm(Senser.Conter.CrSt);
                break;
            }

            case 10:  // Y 累计剂量
            {
                Senser.Conter.Tc   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.TcSt = GetAlarmBit(CanRecBuf[i][7]);  
                UpdateYmAlarm(Senser.Conter.TcSt);
            }

            #if 0
            case 11:  // Y --  CPS 不在这里处理，改为增加电测试或者物理测试
            {
                Senser.Conter.Tc   = CanDataToFloat(&CanRecBuf[i][0]); 
                Senser.Conter.TcSt = CanRecBuf[i][7];  
                UpdateAlarm(Senser.Conter.TcSt);
                break;
            }
            #endif
        }
    }

    return TRUE;
}



// -----Senser CMD  -----------
BYTE FindSenser()
{
    BYTE tm=3;
    static BYTE Btcnt = 0; 
    if (CheckRes.CanSt)
    {
        // 先查询CAN总线有没有探头
        CanSendCmd(SCMD_FIND_SENSER, SENSER_BC, 0); 
        if (0 != CanWaitCmd(SCMD_FIND_SENSER))
        {
            CheckRes.CommInterface = COMM_CAN;
            //memcpy(&CheckRes.SwVer, &BTRecvBuf[6], 2);
            //CheckRes.SwVer = (WORD)atoi((char *)&CanRecBuf[0][1]);
            memcpy(&CheckRes.SwVer, &CanRecBuf[0][1], 4);
            // 有can 探头了把bt 休眠
            BT_Sleep();
            BT_PowerOff();
            return CanRecId.Src;
        }
    }

    if (CheckRes.WifiSt)
    {
        // 再查询BT 上有没有探头
        while(tm--)
        {
            BTSendCmd(SCMD_FIND_SENSER, SENSER_BC, 0);
            if (BTWaitCmd(SCMD_FIND_SENSER))
            {
                CheckRes.CommInterface = COMM_BT;

                // 有蓝牙了关闭8V 和CAN 的电源
                #ifndef UART_DOSE
                P8V_OFF;
                #endif
                
                CAN_EN(0);  // 关闭
                Btcnt = 1;
                memcpy(&CheckRes.SwVer, &BTRecvBuf[6], 4);
                return BTRecvBuf[4];   // 探头类型
                
            }
            Sleep(100);
        }

        // 没有探头关闭蓝牙
        if(Btcnt == 0)
        {
            BT_Sleep();
            BT_PowerOff();
        }
    }
    
    return SENSER_NONE;
}

BOOL ReConnectSenser()
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        CanBus_Init();
    }
    else
    {
        //BT_Init();
    }

    if (CheckRes.SenserType == FindSenser())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    
}


BOOL BTGetParam()
{
    BYTE tm = 10;
    while(tm--)
    {
        BTSendCmd(SCMD_READ_PARAM,CheckRes.SenserType,0);
        if (BTWaitCmd(SCMD_READ_PARAM))
        {
            BTHndParam();
            return TRUE;
        }
    }
    return FALSE;
}

BOOL GetSenserParam()
{
    BYTE ParamCnt;

    if (CheckRes.CommInterface == COMM_CAN)
    {
        memset((BYTE *)&Senser.Param,0,sizeof(Senser.Param));
        CanSendCmd(SCMD_READ_PARAM,CheckRes.SenserType,0);

        ParamCnt = CanWaitCmd(SCMD_READ_PARAM);
        
        return HndSenserParam(ParamCnt);
    }
    else
    {
        return BTGetParam();
    }

    //return FALSE;
}

BOOL CANSetSenserParam()
{
    // 根据传感器类型设置相应的参数
    BYTE  ret,i,id;
    float val;
    //BYTE  bv[4] = {0};

    for (i=0;i<Senser.Contxt->ParmCnt;i++)
    {
        //memset(CanSendData,0,8);
        id = GetValidParamLoc(i);
        if (id != 9)  
        {
            val = GetPararmVal(id);

            // 水下PD和GM的参数是直接发二进制
            if ( (Setting_SenserType == SENSER_PD) ||
                  (Setting_SenserType == SENSER_GM)
                )
            {
                ret = SenserSendFloatEx(SCMD_WRITE_PARAM,id+1,val, 2);
            }
            else
            {
                // 修正系数和校准因子要保留2位小数
                if ((id == 6) || (id == 7) || (id == 8) || (id == 0x0A))
                {
                    ret = SenserSendFloat(SCMD_WRITE_PARAM,id+1,val, 2);
                }
                else
                {
                    ret = SenserSendFloat(SCMD_WRITE_PARAM,id+1,val, 0);
                }
            }
            if (ret == 0)
            {
                //return (BYTE)(id + val);
                return FALSE;
            }
        }
        else  // 只有核素启用参数是BYTE 型的
        {
            ret = SenserSendByte(SCMD_WRITE_PARAM,id+1,ParamTmp.Hn);
            if (ret == 0)
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}


BOOL BTSetSenserParam()
{
    BOOL ret = FALSE;
    BYTE tm=3;
    while(tm--)
    {
        // 阈值要特殊处理
        ParamTmp.Z1 = 2500 - ParamTmp.Z1;
        ParamTmp.Z2 = 2500 - ParamTmp.Z2; 
        ParamTmp.Z3 = 2500 - ParamTmp.Z3;
        ParamTmp.Z4 = 2500 - ParamTmp.Z4; 
        
        memcpy(&BTSendBuf[4], &ParamTmp, sizeof(SENSERPARAM));
        BTSendCmd(SCMD_WRITE_PARAM,CheckRes.SenserType,sizeof(SENSERPARAM));

        // 发送完了要恢复
        ParamTmp.Z1 = 2500 - ParamTmp.Z1;
        ParamTmp.Z2 = 2500 - ParamTmp.Z2;
        ParamTmp.Z3 = 2500 - ParamTmp.Z3;
        ParamTmp.Z4 = 2500 - ParamTmp.Z4; 
        
        ret = BTWaitCmd(SCMD_WRITE_PARAM);
        if (ret) return ret;
    }
    return FALSE;

}

BOOL BTClearTotol()
{
    BTSendCmd(SCMD_CLEAR_TOTOL,CheckRes.SenserType,0);
    return BTWaitCmd(SCMD_CLEAR_TOTOL);
}

BOOL BTClearMax()
{
    BTSendCmd(SCMD_CLEAR_MAX,CheckRes.SenserType,0);
    return BTWaitCmd(SCMD_CLEAR_MAX);
}

BOOL BTSelfCheck()
{
    BTSendCmd(SCMD_SELF_CHECK,CheckRes.SenserType,0);
    return BTWaitCmd(SCMD_SELF_CHECK);
}

BOOL BTSenserPower(WORD On)
{
    BYTE Retry = 10;

    while(Retry--)
    {
        memcpy(&BTSendBuf[4], &On, 2);
        BTSendCmd(SCMD_SENSER_POWER,CheckRes.SenserType,2);
        if( BTWaitCmd(SCMD_SENSER_POWER))
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL BTGetBattary(BYTE Retry)
{
    while(Retry--)
    {
        BTSendCmd(SCMD_GET_BAT,CheckRes.SenserType,0);
        if (BTWaitCmd(SCMD_GET_BAT))
        {
            memcpy(&SenserBatery, &BTRecvBuf[4], sizeof(BATTERY));
            if (SenserBatery.Status == 0)
            {
                ClrAlarm(ALARM_SENSER_BATERY);
            }
            else
            {
                SetAlarm(ALARM_SENSER_BATERY);
            }
            return TRUE;
        }
    }
    return FALSE;
}

BOOL BTConfirmAlarm()
{
    BTSendCmd(SCMD_CONFIRM_ALARM,CheckRes.SenserType,0);
    return BTWaitCmd(SCMD_CONFIRM_ALARM);
}

BOOL BTGmSw(BYTE Gm)
{
    BTSendBuf[4] = Gm;
    BTSendCmd(SCMD_GM_SW,CheckRes.SenserType,1);
    return BTWaitCmd(SCMD_GM_SW);
}


BOOL SetSenserParam()
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        return CANSetSenserParam();
    }
    else
    {
        return BTSetSenserParam();
    }
}


// 启用核素
BOOL SetSenserNuc(BYTE id)
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        #if 0
        CanSendData[0] = 0x0A;   // 启用核素
        CanSendData[1] = id;
        CanSendCmd(SCMD_WRITE_PARAM,CheckRes.SenserType,2);
        return CanWaitCmd(SCMD_WRITE_PARAM);    
        #else
        return SenserSendFloat(SCMD_WRITE_PARAM,0x0A,(float)id,0);
        #endif
    }
    else
    {
        return BTSetSenserParam();
    }
}



BOOL SaveSenserParam()
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        memset(CanSendData,0,8);
        CanSendData[0] = 0;
        CanSendData[1] = 0;
        CanSendCmd(SCMD_SAVE_PARAM,CheckRes.SenserType,0);
        return CanWaitCmd(SCMD_SAVE_PARAM);
    }
    else
    {
        BTSendCmd(SCMD_SAVE_PARAM,CheckRes.SenserType,0);
        return BTWaitCmd(SCMD_SAVE_PARAM);
    }
}


BOOL SetSenserWork(BYTE St, BYTE Ch)
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        memset(CanSendData,0,8);
        CanSendData[0] = St;
        CanSendData[1] = Ch;
        CanSendCmd(SCMD_SET_STATUS,CheckRes.SenserType,2);
        return CanWaitCmd(SCMD_SET_STATUS);
    }
    else
    {
        BTSendBuf[4] = St;
        BTSendBuf[5] = 0;
        BTSendCmd(SCMD_SET_STATUS,CheckRes.SenserType,2);
        return BTWaitCmd(SCMD_SET_STATUS);
    }
    
}


BOOL GetSenserWork()
{
    BYTE ret;
    if (CheckRes.CommInterface == COMM_CAN)
    {
        CanSendCmd(SCMD_READ_STATUS,CheckRes.SenserType,0);
        ret = CanWaitCmd(SCMD_READ_STATUS);
        if (ret != 0)
        {
            Senser.WorkSt = CanRecBuf[0][0]; 
        }
        return ret;
    }
    else
    {
        BTSendCmd(SCMD_READ_STATUS,CheckRes.SenserType,0);
        if (BTWaitCmd(SCMD_READ_STATUS))
        {
            Senser.WorkSt = BTRecvBuf[4];
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}


void BTUpdateAlarm()
{
    if (CheckRes.SenserType == SENSER_LP)
    {
        UpdateAlarm(0,Senser.Conter.CrSt);
        //UpdateAlarm(Senser.Conter.TcSt);
    }
    //start: added by zhaozhenxiang, 20190911
    else if(CheckRes.SenserType == SENSER_LC)
    {
        UpdateAlarm(0,Senser.Conter.C1St);
        UpdateAlarm(0,Senser.Conter.C2St);
        UpdateAlarm(0,Senser.Conter.C3St);
        UpdateAlarm(0,Senser.Conter.P1St);
        UpdateAlarm(0,Senser.Conter.P2St);
        UpdateAlarm(0,Senser.Conter.P3St);
    }
    //emd: added by zhaozhenxiang, 20190911
}


BOOL BTReadCounter()
{
    if (BTValidFrame())
    {
        if (SCMD_READ_COUNTER == BTRecvBuf[1])
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return FALSE;
}

BOOL GetSenserCounter()
{
    BYTE ConterCnt;
    //BOOL ret;
    if (CheckRes.CommInterface == COMM_CAN)
    {
        //MCP2510_WakeUp();       

        
        memset((BYTE *)&Senser.Conter,0,sizeof(SENSERCOUNTER));
        CanSendCmd(SCMD_READ_COUNTER,CheckRes.SenserType,0);

        ConterCnt = CanWaitCmd(SCMD_READ_COUNTER);


        //MCP2510_Sleep();

        return HndSenserCounter(ConterCnt);
    }
    else
    {
        #if 1
        memset((BYTE *)&Senser.Conter,0,sizeof(SENSERCOUNTER));
        BTSendCmd(SCMD_READ_COUNTER,CheckRes.SenserType,0);
        if (BTWaitCmd(SCMD_READ_COUNTER))
        {
            memcpy(&Senser.Conter,&BTRecvBuf[4],sizeof(SENSERCOUNTER));
            BTUpdateAlarm();
            return TRUE;
        }
        else
        {
            return FALSE;
        }
        #else
        if (BTReadCounter())
        {
            memcpy(&Senser.Conter,&BTRecvBuf[4],sizeof(SENSERCOUNTER));
            BTUpdateAlarm();
            ret = TRUE;
        }
        else
        {
            ret = FALSE;
        }

        memset((BYTE *)&Senser.Conter,0,sizeof(SENSERCOUNTER));
        BTSendCmd(SCMD_READ_COUNTER,CheckRes.SenserType,0);
        Clear_BT_Buf();
        return ret;
        #endif
    }
}

BOOL SetSenserBaud()
{   
    // not use
    return TRUE;
}

BYTE GsThres()
{
    if (InnerParam.RadioSelIndex == 0)
    {
        return 0;
    }
    else if (InnerParam.RadioSelIndex >= 18)  // A
    {
        return 9;
    }
    else  // BY
    {
        return 10;
    }
}



BYTE GetThresId()
{
    BYTE id;
    switch (CheckRes.SenserType)
    {
        case SENSER_IN:   id = 1;      break;
        case SENSER_PD:   id = 1;      break;
        case SENSER_GM:   id = 1;      break;
        case SENSER_A_A:  id = 1;      break;
        case SENSER_A_B:  id = 5;      break;
        case SENSER_A_AB: id = 1;      break;
        case SENSER_B_B:  id = 5;      break;
        case SENSER_B_A:  id = 1;      break;
        case SENSER_B_AB: id = 1;      break;
        case SENSER_ABY:  id = 1;      break;
        case SENSER_G1:   id = GsThres();      break;
        case SENSER_G2:   id = GsThres();      break;
    }
    return id;
}


// 核素效率
BOOL SetPadXl(BYTE id, float xl)
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        if (!SenserSendFloat(SCMD_WRITE_RAD,1,id,0))
        {
            return FALSE;
        }
        return SenserSendFloat(SCMD_WRITE_RAD,3,xl,2);
    }
    else
    {
        // 长杆没有核素，暂时没有实现
        return FALSE;
    }
}


// 下发报警阈值
BOOL SetAlarmThres(float val)
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        if (!SenserSendFloat(SCMD_WRITE_RAD,1,pnuc[InnerParam.RadioSelIndex].index,0))
        {
            return FALSE;
        }
        return SenserSendFloat(SCMD_WRITE_RAD,4,val,2);
    }
    else
    {
        // 长杆没有核素，暂时没有实现
        return FALSE;
    }
}


// 活度响应
BOOL SetRadActive(float val)
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        if (!SenserSendFloat(SCMD_WRITE_RAD,1,pnuc[InnerParam.RadioSelIndex].index,0))
        {
            return FALSE;
        }
        return SenserSendFloat(SCMD_WRITE_RAD,5,val,2);
    }
    else
    {
        // 长杆没有核素，暂时没有实现
        return FALSE;
    }
}


// 下发剂量单位
BOOL SetRadUnit(BYTE val)
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        if (!SenserSendFloat(SCMD_WRITE_RAD,1,pnuc[InnerParam.RadioSelIndex].index,0))
        {
            return FALSE;
        }
        return SenserSendByte(SCMD_WRITE_RAD,6,val);
    }
    else
    {
        // 长杆没有核素，暂时没有实现
        return FALSE;
    }
}





BOOL CANSetSenserAlarm()
{   
    // 根据传感器类型设置相应的参数
    BYTE ret,i,id;
    float val;

    
    for (i=0;i<Senser.Contxt->AlarmCnt;i++)
    {
        //memset(CanSendData,0,8);
        id = GetValidAlarmLoc(i);
        val = GetAlarmVal(id);

        // NaI 探头没有 Y3 , 要特殊处理一下
        if (CheckRes.SenserType == SENSER_Y)
        {
            if (id > ALMID_Y3)
            {
                id --;
            }
        }
        ret = SenserSendFloat(SCMD_WRITE_ALARM,id+1,val,0);
        if (ret == 0)
        {
            return FALSE;
        }

        #if 0
        {
            CanSendData[0] = id+1;
            CanSendData[1] = 0;
            CanSendData[2] = 0;
            CanSendData[3] = (BYTE)(val>>24);
            CanSendData[4] = (BYTE)(val>>16);
            CanSendData[5] = (BYTE)(val>>8);
            CanSendData[6] = (BYTE)val;
            CanSendData[7] = 0;   // res
            CanSendCmd(SCMD_WRITE_ALARM,CheckRes.SenserType,8);
            ret = CanWaitCmd(SCMD_WRITE_ALARM);
            if (ret == 0)
            {
                return FALSE;
            }
        }
        #endif
        
    }
    return TRUE;
}


BOOL BTSetSenserAlarm()
{
    BYTE tm=3;
    while(tm--)
    {
        memcpy(&BTSendBuf[4], &AlarmTmp, sizeof(SENSERALARM));
        BTSendCmd(SCMD_WRITE_ALARM,CheckRes.SenserType,sizeof(SENSERALARM));
        return BTWaitCmd(SCMD_WRITE_ALARM);
    }
    return FALSE;
}

BOOL SetSenserAlarm()
{
    if (CheckRes.CommInterface == COMM_CAN)
    {
        return CANSetSenserAlarm();
    }
    else
    {
        return BTSetSenserAlarm();
    }
}

BOOL BTGetAlarm()
{
    BYTE tm = 3;
    while(tm--)
    {
        BTSendCmd(SCMD_READ_ALARM,CheckRes.SenserType,0);
        if (BTWaitCmd(SCMD_READ_ALARM))
        {
            BTHndAlarm();
            return TRUE;
        }
    }
    return FALSE;
}


BOOL GetSenserAlarm()
{
    BYTE AlarmCnt;
    if (CheckRes.CommInterface == COMM_CAN)
    {
        memset((BYTE *)&Senser.Alarm,0,sizeof(Senser.Alarm));
        CanSendCmd(SCMD_READ_ALARM,CheckRes.SenserType,0);

        AlarmCnt = CanWaitCmd(SCMD_READ_ALARM);
        
        return HndSenserAlarm(AlarmCnt);
    }
    else
    {
        return BTGetAlarm();
    }
}


BOOL SenserBackTest()
{
    //BYTE ret;
    memset(CanSendData,0,8);
    CanSendData[0] = 1;
    if (IsAscllCode())
    {
        CanSendData[1] = Senser.Gnd.TestTime + 0x30;
    }
    else
    {
        CanSendData[1] = Senser.Gnd.TestTime;
    }
    CanSendCmd(SCMD_BACK_TEST,CheckRes.SenserType,2);
    return TRUE;

    #if 0
    // 这个命令不等返回
    ret = CanWaitCmd(SCMD_BACK_TEST);
    return ret;
    #endif
}



BOOL SenserXlFix(BYTE Id, float Val)
{
    //static BYTE ret;
    BYTE bv[4] = {0};
    //*(float *)bv = Val;
    memcpy(bv,&Val,4);
    
    CanSendData[0] = Id;   // 参数ID
    CanSendData[1] = 0;
    CanSendData[2] = 0;
    CanSendData[3] = bv[0];  
    CanSendData[4] = bv[1];  
    CanSendData[5] = bv[2];  
    CanSendData[6] = bv[3];  
    CanSendData[7] = 0; //res
    CanSendCmd(SCMD_XL_FIX,CheckRes.SenserType,8);
    return CanWaitCmd(SCMD_XL_FIX);
}

BOOL GetSenserXl(BYTE i)
{
    #if 0
    static BYTE ret;
    static float x;
    CanSendData[0] = i;
    CanSendCmd(SCMD_READ_XL,CheckRes.SenserType,1);
    ret = CanWaitCmd(SCMD_READ_XL);
    if (ret != 0)
    {
        if (CanRecBuf[0][0] == i)
        {
            x = CanDataToFloat(&CanRecBuf[0][0]); 
            if ((x <= 100) && (x >= 0))
            {
                pnuc[i].Efficiency = x*100;
            }
            else
            {
                pnuc[i].Efficiency = 0;
            }
        }
    }
    return ret;
    #else
    // 新的处理方式是直接从数据库中取值
    return TRUE;
    #endif 
}


// 获取第Index 个参数的位置
BYTE GetValidCounterLoc(BYTE Index)
{
    BYTE j,k;
    k = 0;
    for (j=0;j<10;j++)  // 最多10个
    {
        if (Senser.Contxt->CounterLoc[j] == TRUE)
        {
            if (Index == k) break;
            k++;
        }
    }
    return j;
}

float GetCounterVal(BYTE Index)
{
    float ret;
    switch(Index)
    {
        case 0: ret = Senser.Conter.Aj;   break;
        case 1: ret = Senser.Conter.Hv;   break;
        case 2: ret = Senser.Conter.C1;   break;
        case 3: ret = Senser.Conter.C2;   break;
        case 4: ret = Senser.Conter.C3;   break;
        case 5: ret = Senser.Conter.P1;   break;
        case 6: ret = Senser.Conter.P2;   break;
        case 7: ret = Senser.Conter.P3;   break;
        case 8: ret = Senser.Conter.Cr;   break;
        case 9: ret = Senser.Conter.Tc;   break;
        case 10:ret = Senser.Conter.Mr;   break;
    }
    return ret;
}

#if 0
void ShowSenserPower()
{
    if (!BTGetBattary(3))
    {
        ShowHint("获取探测器电压失败",InterMainMenu);
    }
    else
    {
        memset(StrHint,0,24);
        sprintf(StrHint,"电池电压:%dmV-%d%%",
                         SenserBatery.Voltage,SenserBatery.percent);
        ShowHint(StrHint,InterMainMenu);
    }
}
#endif

static BYTE WorkIndex = 1;
#define WORKSTCNT  2
void ShowSsWorkSetUI()
{
    //GT_Show16X16Char(16,0,"探头工作模式",0);    
    PopWindow(20,30,88,50,"维护模式");
    GT_Show12X12Char(30,44,"电测试",  WorkIndex == 1);
    GT_Show12X12Char(30,60,"物理测试",WorkIndex == 2);
    //GT_Show12X12Char(30,72,"辐射测量",WorkIndex == 3);
    
    DisplayRefresh();
}

void InterSsWorkSetUI()
{
    #if 1
    if (CheckRes.CommInterface == COMM_BT)
    {
        EnterBTSenserSrv();
        //ShowSenserPower();
    }
    else
    #endif
    {
        currUI = SENSERWORK;
        //WorkIndex = Senser.WorkSt;
        ShowMainMenu1(); // 背景
        ShowSsWorkSetUI();
    }
}

void SsWorkSetKey(PRESSKEY key)
{
    switch (key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterMainMenu(); break;

        case UP:
        {
            if(--WorkIndex < 1)
            {
                WorkIndex = WORKSTCNT;
            }
            ShowSsWorkSetUI();
        }
        break;
        
        case DOWN:
        {
            if(++WorkIndex > WORKSTCNT)
            {
                WorkIndex = 1;
            }
            ShowSsWorkSetUI();
        }
        break;
        
        case OKKEY:
        {
            //ShowMainMenu();
            if (SetSenserWork(WorkIndex, 0))
            {          
                Senser.WorkSt = WorkIndex;
                ElecTestChannel = 0;
                
                switch (WorkIndex)
                {
                    case 1:   StartElecTest();  break;
                    case 2:   StartPhyTest();   break;
                }
            }
            else
            {
                ShowHint("模式设置失败",InterMainMenu);
            }
        }
        break;
    }
}

