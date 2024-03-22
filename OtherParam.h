#ifndef OTHERPARAM_H
#define OTHERPARAM_H

#include "CommDataDefine.h"

#ifdef __cplusplus
extern  "C" {
#endif

//参数设置
void ShowParamSetUI();
void InterParamSetUI();
void ParamSetUIKey(PRESSKEY key);


  
//其他参数设置界面
//void InterOtherParamSetUI();
//void RefreshOtherParamSel();
//void OthrtParamUIKey(PRESSKEY  dir);

//修改密码界面
void InterChngPSWUI();
void RefreshChngPSWUI();
void ChngPSWUIKey(PRESSKEY  dir);

//修改系统时间
void InterModSysTimeUI();
void ModSysTimeKey(PRESSKEY  dir);
void ShowModTime();
void SetModTime();

//菜单退出时间
void InterMenuRtnTimeUI();
void RefreshMenuRtnSel();
void MenuRtnTimeKey(PRESSKEY  dir);

//测量参数
//void InterMeterSetMenu();
//void MeterSetKey(PRESSKEY  dir);

//单位选择
void InterUnitSelMenu();
void RefreshUnitSelMenu();
void UnitSelKey(PRESSKEY  dir);

//高本底参数设置
void InterHigBottomSet();
void RefreshHigBottomSet();
void HigBottomSetKey(PRESSKEY  dir);

//报警参数设置
void InterAlarmThresSet();
void RefreshAlarmThresSet();
void AlarmThresSetKey(PRESSKEY  dir);

//高压设置
void InterHigVoltSetMenu();
void RefreshHigVoltSetSel();
void HigVoltSetKey(PRESSKEY  dir);

//甄别器阈值
void InterDiscriSetMenu();
void DiscriSetKey(PRESSKEY  dir);

//阿尔法 甄别器阈值
void InterADiscriSetMenu();
void RefreshADiscriSetSel();
void ADiscriSetKey(PRESSKEY  dir);

//贝塔 伽马 甄别器阈值
void InterBDiscriSetMenu();
void RefreshBDiscriSetSel();
void BDiscriSetKey(PRESSKEY  dir);

//阿尔法 贝塔 伽马 甄别器阈值
void InterABDiscriSetMenu();
void RefreshABDiscriSetSel();
void ABDiscriSetKey(PRESSKEY  dir);

//报警参数
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
