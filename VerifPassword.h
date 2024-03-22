#ifndef VERIPASSWORD_H
#define VERIPASSWORD_H

#include "CommDataDefine.h"

#ifdef __cplusplus
extern  "C" {
#endif

void RightKey(PRESSKEY  dir);
void InterPasswordUI();
void RefreshPasswordUI(char pos);
void PasswordKey(PRESSKEY  dir);
void ErrorInfo();

#ifdef __cplusplus
}
#endif
#endif  