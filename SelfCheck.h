#ifndef __SELFCHECK_H__
#define __SELFCHECK_H__

#define COMM_CAN          1    // can接口
#define COMM_BT           2    // 蓝牙接口

typedef struct __CHECKRES
{
    BYTE  SenserType;      // 传感器类型
    BYTE  CommInterface;   // 传感器通信接口
    BOOL  SenserSt;         // 传感器状态
    float BatVol;        
    BOOL  BatSt;
    WORD  HvVol;
    BOOL  HvSt;
    BOOL  E2St;
    BOOL  TimerSt;
    BOOL  WifiSt;
    BOOL  CanSt;
    DWORD  SwVer;    //  传感器软件版本
}CHECKRES;

BOOL HvValid();
void Init_SelfCheck();
void SelfCheckKey(PRESSKEY Key);
void SelfCheck(void);
void CheckLed();
void Init_InSenser();

#endif
