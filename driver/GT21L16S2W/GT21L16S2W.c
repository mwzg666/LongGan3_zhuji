#include "system.h"

#include "GT21L16S2W.h"
#include "msp430x552x.h"

unsigned char  DZ_Data[32];

void Init_GT21L16S2W()
{
    //GT_CS --- P1.7
    //GT_SO --- P2.0
    //GT_SI  --- P2.1
    //GT_SLCK -- P2.2
	P2DIR |= BIT1+BIT2;
    P1DIR |= BIT7;
	P2DIR &= ~BIT0;

    
	SPI_CS_H;
	SPI_SCLK_H;
	SPI_SI_H;
}

void Spi_Send_Byte(unsigned long int cmd)
{
	unsigned char i;

	cmd=cmd|0x03000000;
	for(i=0;i<32;i++)
	{
		SPI_SCLK_L;
		if(cmd&0x80000000)
			SPI_SI_H;
		else
			SPI_SI_L;

		SPI_SCLK_H;

		cmd=cmd<<1;
	}	
}

unsigned char Spi_Read_Byte(void)
{
	unsigned char i=0,data=0;

	SPI_SCLK_H;

	for(i=0;i<8;i++)
	{
		SPI_SCLK_L;
		data=data<<1;
		if(P2IN&BIT0)
			data=data|0x01;
		else 
			data&=0xfe;

		SPI_SCLK_H;	
	}	
        
	return data;
}

/***************************************
ASCII 调用
ASCIICode：表示ASCII 码（8bits）
BaseAdd：说明该套字库在芯片中的起始地址。
r_dat_bat： 是读点阵数据函数。
DZ_Data：是保存读出的点阵数据的数组。
****************************************/
unsigned char  ASCII_GetData(unsigned char ASCIICode,unsigned long int BaseAdd)
{
	if  ((ASCIICode >= 0x20)&&(ASCIICode<=0x7e))
	{	
	  switch(BaseAdd)
	  {
	  case 0x3bfc0:  r_dat_bat((ASCIICode-0x20)*8+BaseAdd,8,DZ_Data); //5X7
	                 break ;
	  case 0x66c0:   r_dat_bat((ASCIICode-0x20)*8+BaseAdd,8,DZ_Data); //7X8
	                 break ;
	  case 0x66d40:  r_dat_bat((ASCIICode-0x20)*12+BaseAdd,12,DZ_Data);//6X12
	                 break ;
	  case 0x3b7c0:  r_dat_bat((ASCIICode-0x20)*16+BaseAdd,16,DZ_Data); //8X16
	                 break ;
	  case 0x67340:  r_dat_bat((ASCIICode-0x20)*26+BaseAdd+2,24,DZ_Data); //12X12 Arial
	                 break ;
	  case 0x3c2c0:  r_dat_bat((ASCIICode-0x20)*34+BaseAdd+2,32,DZ_Data); //16X16 Arial
	                 break ;
	  default:       break;
	  }
	 return 1;
	}
    else  return 0; 
}


/***************************************************
12 点GB2312 标准点阵字库
参数说明：
GBCode：表示汉字内码。
MSB ：表示汉字内码GBCode 的高8bits。
LSB ：表示汉字内码GBCode 的低8bits。
Address： 表示汉字或ASCII字符点阵在芯片中的字节地址。
BaseAdd：说明点阵数据在字库芯片中的起始地址。
r_dat_bat： 是读点阵数据函数。
DZ_Data：是保存读出的点阵数据的数组。
*****************************************************/
void gt_12_GetData (unsigned char MSB,unsigned char LSB)
{
unsigned long int BaseAdd=0x3cf80,Address;

if(MSB >=0xA1 && MSB <= 0xA3 && LSB >=0xA1)
   Address =( (MSB - 0xA1) * (unsigned long)94 + (LSB - 0xA1))*(unsigned long)24+ BaseAdd;
else if(MSB == 0xA9 && LSB >=0xA1)
       Address =(282 + (LSB - 0xA1))*(unsigned long)24+ BaseAdd;
     else if(MSB >=0xB0 && MSB <= 0xF7 && LSB >=0xA1)
          Address = ((MSB - 0xB0) * (unsigned long)94 + (LSB - 0xA1)+ 376)*24+ BaseAdd;
r_dat_bat(Address,24,DZ_Data);
}

/***************************************************
16 点GB2312 标准点阵字库
参数说明：
GBCode：表示汉字内码。
MSB ：表示汉字内码GBCode 的高8bits。
LSB ：表示汉字内码GBCode 的低8bits。
Address： 表示汉字或ASCII字符点阵在芯片中的字节地址。
BaseAdd：说明点阵数据在字库芯片中的起始地址。
r_dat_bat： 是读点阵数据函数。
DZ_Data：是保存读出的点阵数据的数组。
*****************************************************/

void gt_16_GetData (unsigned char MSB,unsigned char LSB)
{
unsigned long int BaseAdd=0,Address;
if(MSB == 0xA9 && LSB >=0xA1)
   Address = (282 + (LSB - 0xA1))*32+BaseAdd;
else if(MSB >=0xA1 && MSB <= 0xA3 && LSB >=0xA1)
       Address =( (MSB - 0xA1) * (unsigned long)94 + (LSB - 0xA1))*32+ BaseAdd;
else if(MSB >=0xB0 && MSB <= 0xF7 && LSB >=0xA1)
        Address = ((MSB - 0xB0) * (unsigned long)94 + (LSB - 0xA1)+ 846)*32+ BaseAdd;
r_dat_bat(Address,32,DZ_Data);
}


/****************************************************
6X12 点国标扩展字符
说明：
BaseAdd：说明本套字库在字库芯片中的起始字节地址。
FontCode：表示字符内码（16bits）.
ByteAddress：表示字符点阵在芯片中的字节地址。
r_dat_bat： 是读点阵数据函数。
DZ_Data：是保存读出的点阵数据的数组。
*****************************************************/
/*
void GB_EXT_612(unsigned int FontCode)
{
unsigned long int BaseAdd=0x66d4c,Address;
if (FontCode>= 0xAAA1 && FontCode<=0xAAFE ) 
Address = (FontCode-0xAAA1 ) * 12+BaseAdd;
else if(FontCode>= 0xABA1&&FontCode<=0xABC0 )
Address = (FontCode-0xABA1 + 95) * 12+BaseAdd;
r_dat_bat(Address,12,DZ_Data);
}
*/

/****************************************************
8X16 点国标扩展字符
说明：
BaseAdd：说明本套字库在字库芯片中的起始字节地址。
FontCode：表示字符内码（16bits）.
Address：表示字符点阵在芯片中的字节地址。
r_dat_bat： 是读点阵数据函数。
DZ_Data：是保存读出的点阵数据的数组。
*****************************************************/
/*
void GB_EXT_816(unsigned int FontCode)
{
unsigned long int BaseAdd=0x3b7d0,Address;
if (FontCode>= 0xAAA1 && FontCode<=0xAAFE ) 
Address = (FontCode-0xAAA1 ) * 16+BaseAdd;
else if(FontCode>= 0xABA1&&FontCode<=0xABC0 )
Address = (FontCode-0xABA1 + 95) * 16+BaseAdd;
r_dat_bat(Address,16,DZ_Data);
}
*/
/*********************************************
Unicode 到GB2312 内码转换程序
功能： 把Unicode 内码转换为GB2312 内码
参数： srcCode：输入Unicode 内码串。
destCode：转换后得到的GB2312 内码串。
r_dat_bat： 是读点阵数据函数。
UToGb2312Buff =0x67d70; // UToGb2312Buff 为Unicode 到GB2312 内码转换表在ROM中的起
始地址.

**********************************************/
/*
unsigned char UnicodeToGB2312(unsigned char *srcCode,unsigned char *destCode)
{
unsigned char result=0;
unsigned long int Address;
unsigned long int UToGb2312Buff =0x67d70;
unsigned long int h;
unsigned int code;
code = *srcCode++;
code = (code<<8) + *srcCode;
if(code<0xa0) result=1;
else if(code<=0xf7) h=code-160;
else if(code<0x2c7) result=1;
else if(code<=0x2c9) h=code-160-463;
else if(code<0x2010) result=1;
else if(code<=0x2312) h=code-160-463-7494;
else if(code<0x2460) result=1;
else if(code<=0x2642) h=code-160-463-7494-333;
else if(code<0x3000) result=1;
else if(code<=0x3017) h=code-160-463-7494-333-2493;
else if(code<0x3220) result=1;
else if(code<=0x3229) h=code-160-463-7494-333-2493-520;
else if(code<0x4e00) result=1;
else if(code<=0x9b54) h=code-160-463-7494-333-2493-520-7126;
else if(code<0x9c7c) result=1;
else if(code<=0x9ce2) h=code-160-463-7494-333-2493-520-7126-295;
else if(code<0x9e1f) result=1;
else if(code<=0x9fa0) h=code-160-463-7494-333-2493-520-7126-295-316;
else if(code<0xe76c) result=1;
else if(code<=0xe774) h=code-160-463-7494-333-2493-520-7126-295-316-18379;
else if(code<0xff00) result=1;
else if(code<=0xff5f) h=code-160-463-7494-333-2493-520-7126-295-316-18379-6027;
else if(code<0xffe0) result=1;
else if(code<=0xffe5) h=code-160-463-7494-333-2493-520-7126-295-316-18379-6027-128;
else result=1;
if(result==0)
{
Address = UToGb2312Buff + (h<<1);
r_dat_bat(Address,2,destCode);
}
else
{
*destCode++ =0xa1;
*destCode =0xa1;
}
return 0;
}
*/
/**************************************************
GB2312 到Unicode 内码转换程序
功能： 把GB2312 内码转换为Unicode 内码
参数： srcCode：输入GB2312 内码串。
destCode：转换后得到的Unicode 内码串。
r_dat_bat： 是读点阵数据函数。
Gb2312ToUBuff =0x2f00; // Gb2312ToUBuff 为GB2312 到Unicode 内码转换表在ROM 中的起
始地址;
***************************************************/
/*
unsigned char GB2312ToUnicode(unsigned char *srcCode,unsigned char *destCode)
{
unsigned long int Gb2312ToUBuff =0x2f00;
unsigned long int Address;
unsigned long int h;
h=Gb2311ToIndex(srcCode);
Address = Gb2312ToUBuff + (h<<1); // +48 head
r_dat_bat(Address,2,destCode);
return 0;
}
*/
/*
unsigned long int Gb2311ToIndex(unsigned char *incode)
{
unsigned long int index;
unsigned char byte0,byte1;
byte0=incode[0];
byte1=incode[1];
if(byte0>=0xA1 && byte0<=0xa3 && byte1 >=0xA1)
{
index=(byte0 - 0xA1) * 94 + (byte1 - 0xA1);
}
else if(byte0==0xA9 && byte1 >=0xA1 && byte1<=0xf6)
index = 282 + (byte1 - 0xA1);
else if(byte0>=0xb0 && byte0<=0xf7 && byte1 >=0xA1)
index = (byte0 - 0xB0) * 94 + (byte1 - 0xA1)+ 368;
else
index=0;
return index;
}
*/
//附：从字库一个起始地址读点阵数据函数
/****************************************************
从字库中读数据函数
说明：
Address  ： 表示字符点阵在芯片中的字节地址。
byte_long： 是读点阵数据字节数。
*p_arr   ： 是保存读出的点阵数据的数组。
*****************************************************/
unsigned char r_dat_bat(unsigned long int address,unsigned char byte_long,unsigned char *p_arr)
{
	unsigned int j=0;
	SPI_CS_L;
	Spi_Send_Byte(address);
	for(j=0;j<byte_long;j++)
	{
	 p_arr[j]=Spi_Read_Byte();
	}
	SPI_CS_H;
	return p_arr[0];	
}
