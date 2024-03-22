/*

函数名称:DS1337_I2C_Start()
函数功能:产生IIC的开始信号
输入:无
输出:无
作者:毕成军
日期:2014.8.16

*/
#ifndef DS1337_H_
#define DS1337_H_

#define DS1337_ADDR 0x68//DS1337的地址

#define DS1337_SDA BIT1//定义IIC 的SDA  IO口
#define DS1337_SCL BIT2//定义IIC 的SCL  IO口

#define DS1337_ACK 0
#define DS1337_NACK 1<<1//因为此处的NACK需要给到SDA上去，而写NACK是用的IOWRIT，所以需要给掩码

typedef struct _DS1337_TIME
{
	char second;
	char minute;
	char hour;
	char week;
	char day;
	char month;
	unsigned int year;	
}DS1337_QTIME;




void DS1337_I2C_Init(void);
void DS1337_I2C_Start();
void DS1337_I2C_Stop();
unsigned char DS1337_I2C_Write_Byte(BYTE out_data);
unsigned char DS1337_I2C_Read_Byte(BYTE ack);
unsigned char BCDToDEC(unsigned char bcd);
unsigned char DECToBCD(unsigned char dec);
void DS1337_Set_Time(DS1337_QTIME *time);
void DS1337_Get_Time(DS1337_QTIME *time);
void GetTimer(DS1337_QTIME *time);
void SetTimer(DS1337_QTIME *time);
unsigned char ds1337_self_test();
void ds1337_test();

#endif