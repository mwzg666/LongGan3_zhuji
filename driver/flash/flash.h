#ifndef FLASH_H_
#define FLASH_H_

#define DATA_COUNT  2048
#define LOG_COUNT   512

// BLD 区域存参数有问题了(0x180后面不能存储)，改到EEPROM
#define FLASH_PARA_START_ADDR 0x1800  //INFO A start address,used to storage para
#define FLASH_PARA_END_ADDR   0x19ff   //INFO A end address,used to storage para

#ifdef LOG_IN_INNER_FALSH
#define FLASH_DATA_START_ADDR   0x01C400   //BANK D ,used to storage mesure data  -- 32K
#define FLASH_LOG_START_ADDR    0x01B400  //BANK C 的后面4K used to storage log  -- 4K
#define BOOTLOADER_START_ADDR   0x1A400     // 4K for bootloader
#else
#define BOOTLOADER_START_ADDR   0x1B400     // 4K for bootloader
#endif



#define FLASH_UPDATE_ADDR   0xE9FE

#define BANK_C  2
#define BANK_D  3

void Erase_Data_Seg(BYTE Bank);
void Erase_Para_Seg();
void Write_Pare_Seg(unsigned char *data,int length);
void Write_Data_Seg(unsigned char *paddr,unsigned char *data,unsigned int length);
void Flash_Read(unsigned char *paddr,unsigned char *data,int length);
void Erase_Block(unsigned char *pErase);
void Flash_Test();
void Erase_Data_Per_Seg(char *pErase);

#endif
