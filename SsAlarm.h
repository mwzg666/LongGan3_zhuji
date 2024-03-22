#ifndef __SENSERALARM_H__
#define  __SENSERALARM_H__

void SsAlarmKey(PRESSKEY  key);
void InterSsAlarmSetUI();
void GetAlarmValStr(BYTE Index);

BYTE GetValidAlarmLoc(BYTE Index);
BYTE GetValidAlarmLocEx(BYTE SsType, BYTE Index);

float GetAlarmVal(BYTE Loc);
float GetAlarmValEx(BYTE Loc);

#endif

