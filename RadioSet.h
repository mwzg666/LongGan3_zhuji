#ifndef RADIOSET_H
#define RADIOSET_H

#include "CommDataDefine.h"

#ifdef __cplusplus
extern  "C" {
#endif


void RadInit();
BYTE GetCurChannel();
BYTE GetCurRadIndex();
BOOL HaveRadSet();
void RadUnitKey(PRESSKEY key);

//核素选择界面
void ShowRadSetUI();
void InterRadSetMenu();
void RefreshRadSet();
void RadSetMoveCursor();
void RadSetKey(PRESSKEY  dir);

//核素设置界面
void InterRadDetailSetMenu();
void RadDetailUpdateSel();
void RadDetailSetKey(PRESSKEY  dir);

//效率标定界面
void InterEffiDemarMenu();
void RefreshEffiDemar();
void EffiDemarKey(PRESSKEY  dir);
void DemaringKey(PRESSKEY key);
void Demaring();

//标定界面
void BeginDemar();
void InterDemarUIMenu();
void ShowEffiDemar();
void DemarUIKey(PRESSKEY  dir);

//报警阈值
void InterRadAlarmMenu();
void RefreshRadAlarm();
void RadAlarmKey(PRESSKEY  dir);

//启用该核素测量
void UseThisRad();
void RefreshUseRad();
void UseRadKey(PRESSKEY  dir);
void InterDemerResUI();
void DemerResKey(PRESSKEY key);
#ifdef __cplusplus
}
#endif
#endif  