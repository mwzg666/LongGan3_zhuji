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

//����ѡ�����
void ShowRadSetUI();
void InterRadSetMenu();
void RefreshRadSet();
void RadSetMoveCursor();
void RadSetKey(PRESSKEY  dir);

//�������ý���
void InterRadDetailSetMenu();
void RadDetailUpdateSel();
void RadDetailSetKey(PRESSKEY  dir);

//Ч�ʱ궨����
void InterEffiDemarMenu();
void RefreshEffiDemar();
void EffiDemarKey(PRESSKEY  dir);
void DemaringKey(PRESSKEY key);
void Demaring();

//�궨����
void BeginDemar();
void InterDemarUIMenu();
void ShowEffiDemar();
void DemarUIKey(PRESSKEY  dir);

//������ֵ
void InterRadAlarmMenu();
void RefreshRadAlarm();
void RadAlarmKey(PRESSKEY  dir);

//���øú��ز���
void UseThisRad();
void RefreshUseRad();
void UseRadKey(PRESSKEY  dir);
void InterDemerResUI();
void DemerResKey(PRESSKEY key);
#ifdef __cplusplus
}
#endif
#endif  