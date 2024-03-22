#include "system.h"
#include "CommDataDefine.h"
#include "msp430x552x.h"
#include "mcp4725.h"
#include "i2c.h"
#include "io.h"
#include "oled.h"
#include "hvadc.h"

/*
��������:MCP4725_OutVol(unsigned char addr,unsigned int voltage)
��������:����MCP4725��ѹ
����:MCP4725�ĵ�ַ����ѹֵ(��λΪ����)
���:�ɹ�����1
����:�ϳɾ�
����:2014.8.16
*/
unsigned int MCP4725_OutVol(unsigned char addr,unsigned int voltage)
{
	long uiVolTemp;
	if(voltage > MCP4725_REF_VOL)
	{
		//printf("input voltage > ref voltage\r\n");
		return 0;
	}
		
	uiVolTemp = voltage*4096l/MCP4725_REF_VOL;
	I2C_Start();
	I2C_Write_Byte(addr<<1);
	I2C_Write_Byte(((uiVolTemp>>8)&0xf)|MCP4725_PD_MODE);
	I2C_Write_Byte(uiVolTemp & 0xff);
	I2C_Stop();
	return 1;
}

/*
��������:MCP4725_Read(unsigned char addr)
��������:��ȡMCP4725���õĵ�ѹֵ
����:MCP4725�ĵ�ַ
���:�ɹ����ض���������
����:�ϳɾ�
����:2014.8.16
*/
#if 0
unsigned int MCP4725_Read(unsigned char addr)
{
	unsigned int uiTemp1,uiTemp2,data;

	I2C_Start();
	I2C_Write_Byte((addr<<1) | 1);
	I2C_Read_Byte(I2C_ACK);
	uiTemp1 = I2C_Read_Byte(I2C_ACK);
	uiTemp2 = I2C_Read_Byte(I2C_NACK);
	I2C_Stop();
	data = ((uiTemp1&0xf)<<4 |(uiTemp2>>4) ) | (uiTemp1<<4);	
	return data;
}
#endif

/*
��������:MCP4725_Test(void)
��������:����MCP4725�Ķ�д����
����:��
���:��
����:�ϳɾ�
����:2014.8.16
*/

#if 0
extern QPara Param;
void MCP4725_Write(void)
{
    //_DINT();
    if((Param.AVvalu < 1) || (Param.AVvalu > 9999))
    {
        Param.AVvalu = 500;
    }
    
    if((Param.BHvalu < 1) || (Param.BHvalu > 9999))
    {
        Param.BHvalu = 2400;
    }
    
    if((Param.BLvalu < 1) || (Param.BLvalu > 9999))
    {
        Param.BLvalu = 100;
    }
    
    if((Param.HVvalu < 1) || (Param.HVvalu > 9999))
    {
        Param.HVvalu = 800;
    }
    
	MCP4725_OutVol(MCP4725_aV_ADDR,Param.AVvalu);//aerfa
	MCP4725_OutVol(MCP4725_BH_ADDR,Param.BHvalu);
	MCP4725_OutVol(MCP4725_BL_ADDR,Param.BLvalu);//beyta
	//Out_HV_Val(Param.HVvalu);
	//data = MCP4725_Read(MCP4725_BH_ADDR);

	delay_ms(100);
}
#endif
