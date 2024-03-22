/*

��������:DS1337_I2C_Start()
��������:����IIC�Ŀ�ʼ�ź�
����:��
���:��
����:�ϳɾ�
����:2014.8.16

*/
#ifndef DS1337_H_
#define DS1337_H_

#define DS1337_ADDR 0x68//DS1337�ĵ�ַ

#define DS1337_SDA BIT1//����IIC ��SDA  IO��
#define DS1337_SCL BIT2//����IIC ��SCL  IO��

#define DS1337_ACK 0
#define DS1337_NACK 1<<1//��Ϊ�˴���NACK��Ҫ����SDA��ȥ����дNACK���õ�IOWRIT��������Ҫ������

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