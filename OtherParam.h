#ifndef OTHERPARAM_H
#define OTHERPARAM_H

#include "CommDataDefine.h"

#ifdef __cplusplus
extern  "C" {
#endif

//��������
void ShowParamSetUI();
void InterParamSetUI();
void ParamSetUIKey(PRESSKEY key);


  
//�����������ý���
//void InterOtherParamSetUI();
//void RefreshOtherParamSel();
//void OthrtParamUIKey(PRESSKEY  dir);

//�޸��������
void InterChngPSWUI();
void RefreshChngPSWUI();
void ChngPSWUIKey(PRESSKEY  dir);

//�޸�ϵͳʱ��
void InterModSysTimeUI();
void ModSysTimeKey(PRESSKEY  dir);
void ShowModTime();
void SetModTime();

//�˵��˳�ʱ��
void InterMenuRtnTimeUI();
void RefreshMenuRtnSel();
void MenuRtnTimeKey(PRESSKEY  dir);

//��������
//void InterMeterSetMenu();
//void MeterSetKey(PRESSKEY  dir);

//��λѡ��
void InterUnitSelMenu();
void RefreshUnitSelMenu();
void UnitSelKey(PRESSKEY  dir);

//�߱��ײ�������
void InterHigBottomSet();
void RefreshHigBottomSet();
void HigBottomSetKey(PRESSKEY  dir);

//������������
void InterAlarmThresSet();
void RefreshAlarmThresSet();
void AlarmThresSetKey(PRESSKEY  dir);

//��ѹ����
void InterHigVoltSetMenu();
void RefreshHigVoltSetSel();
void HigVoltSetKey(PRESSKEY  dir);

//�������ֵ
void InterDiscriSetMenu();
void DiscriSetKey(PRESSKEY  dir);

//������ �������ֵ
void InterADiscriSetMenu();
void RefreshADiscriSetSel();
void ADiscriSetKey(PRESSKEY  dir);

//���� ٤�� �������ֵ
void InterBDiscriSetMenu();
void RefreshBDiscriSetSel();
void BDiscriSetKey(PRESSKEY  dir);

//������ ���� ٤�� �������ֵ
void InterABDiscriSetMenu();
void RefreshABDiscriSetSel();
void ABDiscriSetKey(PRESSKEY  dir);

//��������
void InterAlarmParamMenu();
void AlarmParamKey(PRESSKEY  dir);

void SelectDevType();
void DevTypeKey(PRESSKEY  dir);

void SelectComLog();
void ComLogKey(PRESSKEY key);

#ifdef __cplusplus
}
#endif
#endif  
