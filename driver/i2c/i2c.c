#include "CommDataDefine.h"

#include "system.h"

#include "msp430x552x.h"
#include "io.h"
#include "i2c.h"
#include "oled.h"
//

/*

��������:I2C_Init()
��������:��ʼ��IIC��IO�ڣ����÷���Ϊ���
����:��
���:��
����:�ϳɾ�
����:2014.8.16

*/
void I2C_Init()
{
    P3DIR |= (BIT0+BIT1);
    P3OUT |= (BIT0+BIT1);
}


/*

��������: I2C_Start()
��������:����IIC�Ŀ�ʼ�ź�
����:��
���:��
����:�ϳɾ�
����:2014.8.16

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

��������:I2C_Stop()
��������:����IIC�Ľ����ź�
����:��
���:��
����:�ϳɾ�
����:2014.8.16

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

��������:I2C_Write_Byte(char out_data)
��������:IIC���͵��ֽ�
����:�����͵�����
���:�ɹ�����1
����:�ϳɾ�
����:2014.8.16

*/
unsigned char I2C_Write_Byte(BYTE out_data)
{
    WORD t;
	unsigned char cnt,io_data,ret;
	for(cnt=0;cnt<8;cnt++)
	{
		IOWrite(P3,I2C_SCL_MASK,0);
		delay_us(5);
        
		if(out_data&(0x80>>cnt))  // ��λ��ǰ
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

��������:I2C_Read_Byte(char ack)
��������:IIC���յ��ֽ�
����:������ACK��NACK�ź�
���:���ض���������
����:�ϳɾ�
����:2014.8.16

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

