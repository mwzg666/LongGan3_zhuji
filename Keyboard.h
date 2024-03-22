#ifndef KEYBORAD_H
#define KEYBORAD_H

#ifdef __cplusplus
extern  "C" {
#endif

#ifdef DOUBLE_DELAY
#define KEYDELAY 200 // 大于200ms才响应新的按键
#endif

void KeyBoard_Init();  
PRESSKEY GetKey(void);
  
#ifdef __cplusplus
}
#endif
#endif  