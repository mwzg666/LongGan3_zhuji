#ifndef __BLUETOOTH_H__
#define __BLUETOOTH_H__

#define BT_BUF_LEN 100


void BT_HwInit();
BOOL BT_Init();
BOOL BT_Test();
void BT_Sleep();
void BT_Rst(BYTE lv);

void BT_PowerOn();
void BT_PowerOff();


BOOL BTValidFrame();
BOOL BTWaitCmd(BYTE Cmd);
void BTSendCmd(BYTE Cmd, BYTE Addr, WORD Len);
void Clear_BT_Buf(void);
void BlueToothSetUI(void);
void BTSetKey(PRESSKEY key);

#endif
