#ifndef I2C_H_
#define I2C_H_

#define I2C_SDA_MASK BIT0//定义IIC 的SDA  IO口
#define I2C_SCL_MASK BIT1//定义IIC 的SCL  IO口

#define I2C_HIGH 1
#define I2C_LOW  0

#define I2C_ACK 0
#define I2C_NACK 1


//函数声明
void I2C_Init();
void I2C_Start();
void I2C_Stop();
unsigned char I2C_Write_Byte(BYTE out_data);
unsigned char I2C_Read_Byte(BYTE ack);


#endif
