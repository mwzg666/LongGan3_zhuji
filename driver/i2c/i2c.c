#include "CommDataDefine.h"

#include "system.h"

#include "msp430x552x.h"
#include "io.h"
#include "i2c.h"
#include "oled.h"
//

/*

函数名称:I2C_Init()
函数功能:初始化IIC的IO口，设置方向为输出
输入:无
输出:无
作者:毕成军
日期:2014.8.16

*/
void I2C_Init()
{
    P3DIR |= (BIT0+BIT1);
    P3OUT |= (BIT0+BIT1);
}


/*

函数名称: I2C_Start()
函数功能:产生IIC的开始信号
输入:无
输出:无
作者:毕成军
日期:2014.8.16

*/
void I2C_Start()
{
	IOWrite(P3,I2C_SDA_MASK,I2C_SDA_MASK);
	IOWrite(P3,I2C_SCL_MASK,I2C_SCL_MASK);
	delay_us(5);
	IOWrite(P3,I2C_SDA_MASK,0);
	delay_us(5);
	IOWrite(P3,I2C_SCL_MASK,0);
	delay_us(5);
}

/*

函数名称:I2C_Stop()
函数功能:产生IIC的结束信号
输入:无
输出:无
作者:毕成军
日期:2014.8.16

*/
void I2C_Stop()
{
	IOWrite(P3,I2C_SCL_MASK,0);
	IOWrite(P3,I2C_SDA_MASK,0);	
	delay_us(5);
	IOWrite(P3,I2C_SCL_MASK,I2C_SCL_MASK);
	delay_us(5);
	IOWrite(P3,I2C_SDA_MASK,I2C_SDA_MASK);
	delay_us(5);
}

/*

函数名称:I2C_Write_Byte(char out_data)
函数功能:IIC发送单字节
输入:待发送的数据
输出:成功返回1
作者:毕成军
日期:2014.8.16

*/
unsigned char I2C_Write_Byte(BYTE out_data)
{
    WORD t;
	unsigned char cnt,io_data,ret;
	for(cnt=0;cnt<8;cnt++)
	{
		IOWrite(P3,I2C_SCL_MASK,0);
		delay_us(5);
        
		if(out_data&(0x80>>cnt))  // 高位在前
			IOWrite(P3,I2C_SDA_MASK,I2C_SDA_MASK);
		else
			IOWrite(P3,I2C_SDA_MASK,0);
        
		delay_us(5);
		IOWrite(P3,I2C_SCL_MASK,I2C_SCL_MASK);
	  	delay_us(5);
	}

	IOWrite(P3, I2C_SCL_MASK,0);//this code is necesary
	IODireIn(P3,I2C_SDA_MASK); //change direction into input
	delay_us(5);
	IOWrite(P3,I2C_SCL_MASK,I2C_SCL_MASK); //release sda bus
	delay_us(5);	

    //wait ack
    t = 50000;
    ret = 0;
    while(t--)
    {
	    io_data=IORead(P3);  // read ack
	    if((io_data & I2C_SDA_MASK) == 0)
	    {
            ret = 1;
            break;
	    }
    }

	IOWrite(P3,I2C_SCL_MASK,0);	
	delay_us(5);
	
	IODireOut(P3, I2C_SDA_MASK);//ADD BY KEVIN

    return ret;
	
}


/*

函数名称:I2C_Read_Byte(char ack)
函数功能:IIC接收单字节
输入:给出的ACK或NACK信号
输出:返回读出的数据
作者:毕成军
日期:2014.8.16

*/
unsigned char I2C_Read_Byte(BYTE ack)
{
	unsigned char rd_data=0,cnt=0;
	unsigned char io_data=0,data;
	
    /*
     release SDA bus
   */
	IOWrite(P3,I2C_SDA_MASK,I2C_SDA_MASK);
	IODireIn(P3, I2C_SDA_MASK);	
	delay_us(5);
	for(cnt=0;cnt<8;cnt++)
	{
    	IOWrite(P3,I2C_SCL_MASK,0);
    	delay_us(20);
    	IOWrite(P3,I2C_SCL_MASK,I2C_SCL_MASK);
    	delay_us(20);
    	io_data=IORead(P3);
    	rd_data<<=1;
    	if(io_data&(I2C_SDA_MASK))
       	rd_data|=1;
	}
	
	IOWrite(P3,I2C_SCL_MASK,0);	
	delay_us(5);
	IODireOut(P3, I2C_SDA_MASK);
	IOWrite(P3,I2C_SDA_MASK,ack);
	delay_us(5);
	IOWrite(P3,I2C_SCL_MASK,I2C_SCL_MASK);	
	delay_us(5);
	IOWrite(P3,I2C_SCL_MASK,0);
	delay_us(5);
	data=rd_data;
	return data;
}

