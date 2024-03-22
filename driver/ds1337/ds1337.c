#include <string.h>
#include "msp430x552x.h"
#include "CommDataDefine.h"

#include "system.h"

#include "ds1337.h"
#include "io.h"
#include "oled.h"


#define DecToBcdS(dec) (dec%10| (dec/10%10)<<4 | (dec/100%10)<<8 |(dec/1000%10)<<12)
#define BcdToDecS(bcd) (((bcd>>12)&0xf) *1000 +((bcd>>8)&0xf)*100+((bcd>>4)&0xf)*10+(bcd&0xf))

/*

函数名称:DS1337_I2C_Init()
函数功能:初始化DS1337的IIC的IO口，设置方向为输出
输入:无
输出:无
作者:毕成军
日期:2014.8.16

*/
void DS1337_I2C_Init(void)
{
    P4DIR |= (BIT1+BIT2);
}

/*

函数名称:DS1337_I2C_Start()
函数功能:产生IIC的开始信号
输入:无
输出:无
作者:毕成军
日期:2014.8.16

*/

void DS1337_I2C_Start()
{
	IOWrite(P4,DS1337_SDA,DS1337_SDA);
	IOWrite(P4,DS1337_SCL,DS1337_SCL);
	delay_us(5);
	IOWrite(P4,DS1337_SDA,0);
	delay_us(5);
	IOWrite(P4,DS1337_SCL,0);
	delay_us(5);
}

/*

函数名称:DS1337_I2C_Stop()
函数功能:产生IIC的结束信号
输入:无
输出:无
作者:毕成军
日期:2014.8.16

*/
void DS1337_I2C_Stop()
{
	IOWrite(P4,DS1337_SCL,0);
	IOWrite(P4,DS1337_SDA,0);	
	delay_us(5);
	IOWrite(P4,DS1337_SCL,DS1337_SCL);
	delay_us(5);
	IOWrite(P4,DS1337_SDA,DS1337_SDA);
	delay_us(5);
}


/*

函数名称:DS1337_I2C_Write_Byte(char out_data)
函数功能:IIC发送单字节
输入:待发送的数据
输出:成功返回1
作者:毕成军
日期:2014.8.16

*/

unsigned char DS1337_I2C_Write_Byte(BYTE out_data)
{
	unsigned char cnt,io_data;
	for(cnt=0;cnt<8;cnt++)
	{
		IOWrite(P4,DS1337_SCL,0);
		delay_us(5);
		if(out_data&(0x80>>cnt))
			IOWrite(P4,DS1337_SDA,DS1337_SDA);
		else
			IOWrite(P4,DS1337_SDA,0);
			delay_us(5);
			IOWrite(P4,DS1337_SCL,DS1337_SCL);
  	  		delay_us(5);
	}

	IOWrite(P4, DS1337_SCL,0);//this code is necesary
	IODireIn(P4,DS1337_SDA); //change direction into input
	delay_us(5);
	IOWrite(P4,DS1337_SCL,DS1337_SCL); //release sda bus
	delay_us(5);	
		
	io_data=IORead(P4);
	IOWrite(P4,DS1337_SCL,0);	
	delay_us(5);
	
	IODireOut(P4, DS1337_SDA);//ADD BY KEVIN
	if(io_data & DS1337_SDA)
	{
	  //printf("ack error!\n");
          return 0;
	}		
	else
    	{
	  //printf("ack ok!\n");	
          return 1;
    	}
}


/*

函数名称:DS1337_I2C_Read_Byte(char ack)
函数功能:IIC接收单字节
输入:给出的ACK或NACK信号
输出:返回读出的数据
作者:毕成军
日期:2014.8.16

*/
unsigned char DS1337_I2C_Read_Byte(BYTE ack)
{
	unsigned char rd_data=0,cnt=0;
	unsigned char io_data=0,data;
	
    /*
     release SDA bus
   */
	IOWrite(P4,DS1337_SDA,DS1337_SDA);
	IODireIn(P4, DS1337_SDA);	
	delay_us(5);
	for(cnt=0;cnt<8;cnt++)
    	{
		IOWrite(P4,DS1337_SCL,0);
		delay_us(5);
		IOWrite(P4,DS1337_SCL,DS1337_SCL);
		delay_us(5);
		io_data=IORead(P4);
		rd_data<<=1;
		if(io_data&(DS1337_SDA))
	   	rd_data|=1;
    	}
	
	IOWrite(P4,DS1337_SCL,0);	
	delay_us(5);
	IODireOut(P4, DS1337_SDA);
	IOWrite(P4,DS1337_SDA,ack);
	delay_us(5);
	IOWrite(P4,DS1337_SCL,DS1337_SCL);	
	delay_us(5);
	IOWrite(P4,DS1337_SCL,0);
	delay_us(5);
	data=rd_data;
	return data;
}

/*

函数名称:BCDToDEC(unsigned char bcd)
函数功能:将BCD码转换为十进制码
输入:BCD码
输出:十进制码
作者:毕成军
日期:2014.8.16

*/

unsigned char BCDToDEC(unsigned char bcd)
{
  unsigned char tmp1,tmp2;
  tmp1 = bcd & 0x0f;
  tmp2 = (bcd & 0xf0) >> 4;
  return(tmp2*10+tmp1);
}

/*

函数名称:DECToBCD(unsigned char dec)
函数功能:将十进制码转换为BCD码
输入:十进制码
输出:BCD码
作者:毕成军
日期:2014.8.16

*/

unsigned char DECToBCD(unsigned char dec)
{
  unsigned char tmp1,tmp2;
  tmp1 = ((dec / 10) % 10);
  tmp2 = dec % 10;
  return((tmp2 | (tmp1 << 4)));
}

/*

函数名称:DS1337_Set_Time(DS1337_QTIME *time)
函数功能:设置DS1337的时间
输入:时间结构体，包含年、月、日、周、时、分、秒
输出:无
作者:毕成军
日期:2014.8.16

*/
void DS1337_Set_Time(DS1337_QTIME *time)
{
	int i=0;
	DS1337_I2C_Start();
	DS1337_I2C_Write_Byte(DS1337_ADDR<<1);
	DS1337_I2C_Write_Byte(0);

	for(i=0;i<sizeof(DS1337_QTIME)-1;i++)
  	{
		DS1337_I2C_Write_Byte(((char *)time)[i]);
  	}
	DS1337_I2C_Stop();
}

/*

函数名称:DS1337_Get_Time(DS1337_QTIME *time)
函数功能:读取DS1337的时间
输入:定义时间结构体，包含年、月、日、周、时、分、秒
输出:返回读取到的邋值
作者:毕成军
日期:2014.8.16

*/




void DS1337_Get_Time(DS1337_QTIME *time)
{
	DS1337_I2C_Start();
	DS1337_I2C_Write_Byte(DS1337_ADDR<<1);
	DS1337_I2C_Write_Byte(0);
  
	DS1337_I2C_Start();
	DS1337_I2C_Write_Byte(DS1337_ADDR<<1 | 1);

	time->second = DS1337_I2C_Read_Byte(DS1337_ACK);
	time->minute = DS1337_I2C_Read_Byte(DS1337_ACK);
	time->hour= DS1337_I2C_Read_Byte(DS1337_ACK);
	time->week=DS1337_I2C_Read_Byte(DS1337_ACK);
	time->day= DS1337_I2C_Read_Byte(DS1337_ACK);
	time->month= DS1337_I2C_Read_Byte(DS1337_ACK);
	time->year= DS1337_I2C_Read_Byte(DS1337_NACK);
	
	DS1337_I2C_Stop();
}


/*

函数名称:GetTimer(DS1337_QTIME *time)
函数功能:读取DS1337的时间并将其转换成十进制形式
输入:时间结构体，包含年、月、日、周、时、分、秒
输出:无
作者:毕成军
日期:2014.8.16

*/

void GetTimer(DS1337_QTIME *time)
{
  DS1337_Get_Time(time);
  
  time->second=BCDToDEC(time->second);
  time->minute=BCDToDEC(time->minute);
  time->hour=BCDToDEC(time->hour);
  time->week=BCDToDEC(time->week);
  time->day=BCDToDEC(time->day);
  time->month=BCDToDEC(time->month);
  time->year=BCDToDEC(time->year)+2000;
  
}

/*

函数名称:SetTimer(DS1337_QTIME *time)
函数功能:将输入的十进制格式的时间转换为BCD码格式再写入到DS1337
输入:时间结构体，包含年、月、日、周、时、分、秒
输出:无
作者:毕成军
日期:2014.8.16

*/

void SetTimer(DS1337_QTIME *time)

{

  time->second=DECToBCD(time->second);
  time->minute=DECToBCD(time->minute);
  time->hour=DECToBCD(time->hour);
  time->week=DECToBCD(time->week);
  time->day=DECToBCD(time->day);
  time->month=DECToBCD(time->month);
  time->year=DecToBcdS(time->year);
  DS1337_Set_Time(time);
}


#if 0
/*

函数名称:ds1337_self_test()
函数功能:测试DS1337是否存在，
输入:无
输出:1表示存在DS1337,0表示DS1337有问题
作者:毕成军
日期:2015.7.30

*/

unsigned char ds1337_self_test()
{
   DS1337_I2C_Init();
   DS1337_I2C_Start();
   if(DS1337_I2C_Write_Byte(DS1337_ADDR<<1)==1)
   {
      //printf("ds1337 exist!\n");
     DS1337_I2C_Stop();
     return 1;
   }
   else
   {
     //printf("ack error,ds1337 may error!\n");
     DS1337_I2C_Stop();
     return 0;
   }
}


/*

函数名称:ds1337_test()
函数功能:测试DS1337的读写功能
输入:无
输出:无
作者:毕成军
日期:2014.8.16

*/
void ds1337_test()
{
	DS1337_QTIME localtime,localtime2;
	localtime.year=2007;
	localtime.month=8;
	localtime.day=16;
	localtime.week=6;
	localtime.hour=12;
	localtime.minute=29;
	localtime.second=1;

	DS1337_I2C_Init();

	SetTimer(&localtime);
	delay_ms(10);
	memset((void *)&localtime2,0,sizeof(localtime2));
	GetTimer(&localtime2);
	delay_ms(1);
	
}

#endif


