#ifndef __SELFCHECK_H__
#define __SELFCHECK_H__

#define COMM_CAN          1    // can�ӿ�
#define COMM_BT           2    // �����ӿ�

typedef struct __CHECKRES
{
    BYTE  SenserType;      // ����������
    BYTE  CommInterface;   // ������ͨ�Žӿ�
    BOOL  SenserSt;         // ������״̬
    float BatVol;        
    BOOL  BatSt;
    WORD  HvVol;
    BOOL  HvSt;
    BOOL  E2St;
    BOOL  TimerSt;
    BOOL  WifiSt;
    BOOL  CanSt;
    DWORD  SwVer;    //  ����������汾
}CHECKRES;

BOOL HvValid();
void Init_SelfCheck();
void SelfCheckKey(PRESSKEY Key);
void SelfCheck(void);
void CheckLed();
void Init_InSenser();

#endif
