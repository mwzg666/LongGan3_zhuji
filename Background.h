#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "CommDataDefine.h"

#ifdef __cplusplus
extern  "C" {
#endif

//void InterBackUI();  
//void ShowBack();
void BackResKey(PRESSKEY  key);
void InterBackResUI();
void InterBackSetUI();
void RefrehBackSet();
void BackSetKey(PRESSKEY  dir);
void TimeSelKey(PRESSKEY key);
void ShowBackSetHead();
void RefreshBackUI();
void BackTestKey(PRESSKEY key);
void StartBackTest();
#ifdef __cplusplus
}
#endif
#endif  