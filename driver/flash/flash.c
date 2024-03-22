#include "msp430x552x.h"
#include "system.h"
#include "CommDataDefine.h"

#include "flash.h"
#include "string.h"
#include "oled.h"
/*

��������:Erase_Para_Seg()
��������:���������δ洢����,Ŀǰ�õ���INFO D�����øú����ὫINFO Dȫ������
����:��
���:��
����:�ϳɾ�
����:2014.8.18

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
��������:Erase_Data_Seg()
��������:�������ݴ洢�Σ�Ŀǰʹ����BANK C ��BANK D
����:��
���:��
����:�ϳɾ�
����:2014.8.18
*/
void Erase_Data_Seg(BYTE Bank)
{
    _DINT(); 
    
    #if 0   
    // BANK_C ���ڲ���ɾ�˺���Ĵ���������
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

    // �������Ǵ��벻��ɾ��
	_EINT();
}


// ����һ����
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
��������:Write_Pare_Seg(unsigned char *data,unsigned char length)
��������:д�����Σ�д�볤�Ȳ��ܳ���128byte�����������Ҫ��չһ��
����:   data:��д������ݣ�length:д��ĳ���
���:��
����:�ϳɾ�
����:2014.8.18
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
��������:Write_Data_Seg(long addr,unsigned char *data,unsigned char length)
��������:д���ݶκ�����д����������
����:   addr: д��ʼ��ַ��data:��д������ݽṹ��length:��д��ĳ���
���:��
����:�ϳɾ�
����:2014.8.18
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
��������:Flash_Read(long addr,unsigned char *data,unsigned char length)
��������:��FLASH������ָ�����ȵ�FLASH
����:   addr: ����ʼ��ַ��data:����������ݴ洢������length:�������ĳ���
���:��
����:�ϳɾ�
����:2014.8.18
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

