#ifndef __SENSERPARAM_H__
#define  __SENSERPARAM_H__

void InputHv(ReturnFun rf, BYTE Line);
void InputZ1(ReturnFun rf, BYTE Line);
void InterSsParamSetUI();
void SsParamKey(PRESSKEY  key);
void GetPararmValStr(BYTE Index);
float GetPararmVal(BYTE Index);
BYTE GetValidParamLoc(BYTE Index);
void SaveParam(ReturnFun Rf);
#endif