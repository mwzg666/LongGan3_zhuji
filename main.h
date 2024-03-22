
#ifndef __MAIN_H__
#define __MAIN_H__




void LogoFaceKey(PRESSKEY  dir);


void StartBeep(WORD OnTime, WORD OffTime);
void StopBeep(void);
void BeepOn(void);
void BeepOff(void);

void StartAlarmLed(WORD OnTime, WORD OffTime);
void StopAlarmLed(void);
void AlarmLedOn(void);
void AlarmLedOff(void);

void StartRunLed(WORD OnTime, WORD OffTime);
void StopRunLed(void);
void RunLedOn(void);
void RunLedOff(void);

void BkLightCtl();


void ShowTitle();
void ShowComName();
void ShowVer(BOOL sTime);
void ShowTime(BYTE Line);
void ShowTime2(BYTE Line);
void ShowOptHint();

void OpenBkLight();
void CloseBkLight();
void SetBkParam();


void HV_CTL(BYTE lv);

#endif

