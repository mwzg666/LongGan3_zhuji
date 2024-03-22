#ifndef MAINMENU_H
#define MAINMENU_H

#include "CommDataDefine.h"

#ifdef __cplusplus
extern  "C" {
#endif

void InterMainMenu();
void MainMenuKey(PRESSKEY  dir);  
//void ShowMainMenu();
void ShowMainMenu1();
void OutSenserSetUI();
void OutSenserSetKey(PRESSKEY  dir);
void InSenserSetUI();
void InSenserSetKey(PRESSKEY  dir);
void MeterManKey(PRESSKEY key);
void InSenserTestKey(PRESSKEY key);
void InCntFactorKey(PRESSKEY  dir);
void StartSetOutAlarm(BYTE Type);
#ifdef __cplusplus
}
#endif
#endif  