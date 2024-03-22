#ifndef __EEPROM_H__
#define __EEPROM_H__


// 24AA1026 -- 128KB , 只用了前面的64K 所以地址是A0
#define EEPROM_ADDR   0xA0

// 前16个字节用于标识和自检
// 参数/数据从16 后开始存储
#define PARAM_OFFSET  16      // 1024-16=1008
#define LOG_OFFSET    1024    // 15k
#define DATA_OFFSET   16384   // 32K 

BOOL Eeprom_ReadByte(WORD Addr, BYTE *Data);

BOOL Eeprom_Test();
BOOL Eeprom_WriteByte(WORD Addr, BYTE Dat);

BOOL Eeprom_ReadBuff(WORD Addr, BYTE *Buff, WORD Len);

#endif
