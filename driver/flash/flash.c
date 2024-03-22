#include "msp430x552x.h"
#include "system.h"
#include "CommDataDefine.h"

#include "flash.h"
#include "string.h"
#include "oled.h"
/*

函数名称:Erase_Para_Seg()
函数功能:擦出参数段存储区域,目前用的是INFO D，调用该函数会将INFO D全部擦除
输入:无
输出:无
作者:毕成军
日期:2014.8.18

*/

void Erase_Para_Seg()
{
    char *pErase; //notice  deal point var 
    unsigned i;
    _DINT(); 

    for(i=0;i<4;i++)
    {
        pErase =(char *)(FLASH_PARA_START_ADDR+i*0x80);
        FCTL3 = FWKEY;                            // Clear Lock bit
        FCTL1 = FWKEY+ERASE;                      // Set Erase bit
        *pErase = 0;                           // Dummy erase byte        
        while(FCTL3 & BUSY);                          // Dummy erase byte
        FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
    }	

    _EINT();
}

/*
函数名称:Erase_Data_Seg()
函数功能:擦除数据存储段，目前使用了BANK C 和BANK D
输入:无
输出:无
作者:毕成军
日期:2014.8.18
*/
void Erase_Data_Seg(BYTE Bank)
{
    _DINT(); 
    
    #if 0   
    // BANK_C 现在不能删了后面的代码在里面
    char *pErase;
    if (Bank == BANK_C)
    {
    	pErase=(char *)FLASH_DATA_START_ADDR; //notice  deal point var 

    	FCTL3 = FWKEY;                            // Clear Lock bit
    	FCTL1 = FWKEY+MERAS;                      // Set Bank Erase bit
      	*pErase = 0;                              // Dummy erase byte
        while(FCTL3 & BUSY);                          // Dummy erase byte
    	FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
    	
    }
    else 
    #endif

    #ifdef DATA_IN_INNER_FALSH
    char *pErase;
    if (Bank == BANK_D)
    {
        pErase = (char *)FLASH_DATA_START_ADDR; // FLASH_LOG_START_ADDR;
        
        FCTL3 = FWKEY;                            // Clear Lock bit
    	FCTL1 = FWKEY+MERAS;                      // Set Bank Erase bit
      	*pErase = 0;                              // Dummy erase byte
        while(FCTL3 & BUSY);                          // Dummy erase byte
    	FCTL3 = FWKEY+LOCK;                       // Set LOCK bit

    }
    #endif

    // 其他块是代码不能删除
	_EINT();
}


// 擦除一个块
void Erase_Block(unsigned char *pErase)
{
    _DINT(); 
    
    FCTL3 = FWKEY;                            // Clear Lock bit
    FCTL1 = FWKEY+ERASE;                      // Set Erase bit
    *pErase = 0;                           // Dummy erase byte        
    while(FCTL3 & BUSY);                          // Dummy erase byte
    FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
    
    _EINT();
}


void Erase_Data_Per_Seg(char *pErase)
{
    _DINT(); 
    
    FCTL3 = FWKEY;                            // Clear Lock bit
    FCTL1 = FWKEY+ERASE;                      // Set Erase bit
    *pErase = 0;                           // Dummy erase byte        
    while(FCTL3 & BUSY);                          // Dummy erase byte
    FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
    
    _EINT();
}


/*
函数名称:Write_Pare_Seg(unsigned char *data,unsigned char length)
函数功能:写参数段，写入长度不能超过128byte，如果不够需要扩展一下
输入:   data:待写入的数据，length:写入的长度
输出:无
作者:毕成军
日期:2014.8.18
*/
void Write_Pare_Seg(unsigned char *data,int length)
{
	int i;
	char *Flash_ptr;

	Erase_Para_Seg();

	Flash_ptr = (char *)FLASH_PARA_START_ADDR;
    _DINT(); 
	FCTL3 = FWKEY;                            // Clear Lock bit
	FCTL1 = FWKEY+WRT; 		// Set WRT bit for write operation

	for(i=0;i<length;i++)
	{
        *Flash_ptr++=*data++;
	}

	FCTL1 = FWKEY;                            // Clear WRT bit
	FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
	_EINT(); 
}

/*
函数名称:Write_Data_Seg(long addr,unsigned char *data,unsigned char length)
函数功能:写数据段函数，写入测量结果用
输入:   addr: 写开始地址，data:待写入的数据结构，length:待写入的长度
输出:无
作者:毕成军
日期:2014.8.18
*/
void Write_Data_Seg(unsigned char *paddr,unsigned char *data,unsigned int length)
{
	unsigned int i;
	unsigned char *Flash_ptr;

	Flash_ptr = paddr;

	//_DINT(); 
	FCTL3 = FWKEY;                            // Clear Lock bit
	FCTL1 = FWKEY+WRT; 		// Set WRT bit for write operation

	for(i=0;i<length;i++)
	{
		*Flash_ptr++=*data++;
	}

	FCTL1 = FWKEY;                            // Clear WRT bit
	FCTL3 = FWKEY+LOCK;                       // Set LOCK bit
	//_EINT(); 
}

/*
函数名称:Flash_Read(long addr,unsigned char *data,unsigned char length)
函数功能:读FLASH，读出指定长度的FLASH
输入:   addr: 读开始地址，data:读出后的数据存储变量，length:待读出的长度
输出:无
作者:毕成军
日期:2014.8.18
*/
void Flash_Read(unsigned char *paddr,unsigned char *data,int length)
{
	int i;
	unsigned char *flash_ptr;

	flash_ptr = paddr;

	for(i=0;i<length;i++)
	{
		*data++=*flash_ptr++;
	}
}

#if 0
void Flash_Test()
{
	const unsigned char buf1[]={'a','b','c','d','e','f','g','h','i','j'};
    //unsigned char buf[501]={'0','1','2','3','4','5','6','7','8','9'};
	//char temp_data[128];
 
#if 0
	Write_Pare_Seg((unsigned char *)buf,sizeof(buf));
	//memset(temp_data,0,sizeof(temp_data));
	//Flash_Read((unsigned char *)FLASH_PARA_START_ADDR, (unsigned char *)temp_data, sizeof(buf));
        Erase_Para_Seg();
#else
	Write_Data_Seg((unsigned char *)FLASH_DATA_START_ADDR,(unsigned char*)buf1,sizeof(buf1));
    //Write_Data_Seg((unsigned char *)FLASH_DATA_START_ADDR+10240,(unsigned char*)buf1,sizeof(buf1));
    //Write_Data_Seg((unsigned char *)FLASH_DATA_START_ADDR+0x8000,(unsigned char*)buf1,sizeof(buf1));
	//memset(temp_data,0,sizeof(temp_data));	
	//Flash_Read((unsigned char *)FLASH_DATA_START_ADDR, (unsigned char*)temp_data, sizeof(buf1));	
    //Erase_Data_Seg();
    //Erase_Data_Per_Seg((unsigned char *)0x14405);
    //_NOP();
#endif
}
#endif

