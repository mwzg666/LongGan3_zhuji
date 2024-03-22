#ifndef MEASINTER_H
#define MEASINTER_H

#include "CommDataDefine.h"

#ifdef __cplusplus
extern  "C" {
#endif


#define SUNIT_uSv_h  0
#define SUNIT_mSv_h  1
#define SUNIT_Sv_h   2

#define SUNIT_uSv  0
#define SUNIT_mSv  1
#define SUNIT_Sv   2

#define SUNIT_cps      0
#define SUNIT_Bq       1
#define SUNIT_Bq_cm2  2

#define RAD_MODE_MIX  0  // 混合模式
#define RAD_MODE_A    1  // 只显示α
#define RAD_MODE_B    2  // 只显示β

void Init_Cache();
BOOL NeedHv();
void OpenHv(WORD vol);
void CloseHv();
void ShowStatusBar();
void ShowSenser(BYTE x, BYTE y, BYTE type);
void EnterMeasuInter();
void MeasuInterKey(PRESSKEY  dir);
//void ShowCPS();
void StorageLog(BYTE Event);
void StorageMeterData();
void Storage30Sec();
void StaryArrData(unsigned char count);
void ShowCurrProgress(float DnVal, float UpVal, float CuVal);
void ShowProgress(BYTE y, float a1, float a2, float a3, float CuVal);
void ShowTotalProgress(DWORD Max, DWORD Loc);
void Cache_CurrDataBlock();
void Cache_CurrLogBlock();
void Save_CurrBlock();
void Show_InCounter(BYTE x, BYTE y, float val);
void Show_InCounterMid(BYTE x, BYTE y, float val);
void Show_InCounterSm(BYTE x, BYTE y, float val);
void Show_InTotal(BYTE x, BYTE y, float val);
void ShowCounter(BYTE x, BYTE y, char *Title, float Counter, BYTE Unit);
void ShowCounterMid(BYTE x, BYTE y, char *Title, float Counter, BYTE Unit);
void ShowCounterSm(BYTE x, BYTE y, char *Title, float Counter, BYTE Unit);
void ShowMeteTitle();
void Check_InAlarmEx();
void Check_InAlarm();
void Check_InAlarmLC();
float GetVal1(QStorageData *data);
float GetVal2(QStorageData *data);
BYTE GetInUnit0(QStorageData *data);
BYTE GetInUnit1(QStorageData *data);
BYTE GetInUnit2(QStorageData *data);
BYTE GetOutUnit0(QStorageData *data);
BYTE GetOutUnit1(QStorageData *data);
BYTE GetOutUnit2(QStorageData *data);

void TotalAdd();
float Get_InCounter();
void GetOutCounter();
void StorageAvData();
void StorageMaxData();
void StorageRtData();

BOOL VlotPerChanged();
BOOL CounterChanged();
void ConfirmAlarm();
void ClearCounter();
void ShowDebugInfo();
void ClearAlarm();
BYTE AlarmCnt();
BYTE GetAlarmIndex(BYTE no);

#ifdef __cplusplus
}
#endif
#endif  