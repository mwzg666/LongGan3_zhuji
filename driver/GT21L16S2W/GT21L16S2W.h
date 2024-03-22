#ifndef GT21L16S2W_H_
#define GT21L16S2W_H_


#define SPI_CS_H      P1OUT |= BIT7
#define SPI_CS_L      P1OUT &= ~BIT7
#define SPI_SCLK_H  P2OUT |= BIT2
#define SPI_SCLK_L  P2OUT &= ~BIT2
#define SPI_SI_H     P2OUT |= BIT1
#define SPI_SI_L     P2OUT &= ~BIT1

void Init_GT21L16S2W();
void Spi_Send_Byte(unsigned long int cmd);
unsigned char Spi_Read_Byte(void);
unsigned char  ASCII_GetData(unsigned char ASCIICode,unsigned long int BaseAdd);
void gt_12_GetData(unsigned char MSB,unsigned char LSB);
void gt_16_GetData(unsigned char MSB,unsigned char LSB);
//void GB_EXT_612(unsigned int FontCode);
//void GB_EXT_816(unsigned int FontCode);
//unsigned char UnicodeToGB2312(unsigned char *srcCode,unsigned char *destCode);
//unsigned char GB2312ToUnicode(unsigned char *srcCode,unsigned char *destCode);
//unsigned long int Gb2311ToIndex(unsigned char *incode);
unsigned char r_dat_bat(unsigned long int address,unsigned char byte_long,unsigned char *p_arr);

#endif