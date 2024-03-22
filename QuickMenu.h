#ifndef QUICKMENU_H
#define QUICKMENU_H

#include "CommDataDefine.h"

#ifdef __cplusplus
extern  "C" {
#endif

void InterQuickMenu();
void RefreshQuickMenu();
void QuickMenuSele(PRESSKEY  dir);
void MeterViewKey(PRESSKEY key);
void ShowAlmKey(PRESSKEY  dir);
void ClearMax();
void SaveDose();
#ifdef __cplusplus
}
#endif
#endif  