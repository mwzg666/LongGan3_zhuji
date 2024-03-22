#include <msp430.h>
#include "CommDataDefine.h"
#include "system.h"

#ifdef PKUR

//#include  "MSP430x16x.H"

//PUK_RST  -- P8.2
//PKU_AE1 -- P8.1
//PKU_AE2 -- P8.0
//PUK_CONT_EN -- P2.6
//PKU_CLK -- P2.7
//PKU_MOSI -- P3.3
//PKU_MISO -- P3.4


#define Spi_Sck1   (P2OUT |=  BIT7)
#define Spi_Sck0   (P2OUT &=(~BIT7))
#define Spi_Si1    (P3OUT |=  BIT3)
#define Spi_Si0    (P3OUT &=(~BIT3))   
#define Spi_So     ((P3IN>>4) & 0X01) 

#define PUK_RST1   (P8OUT |= BIT2)
#define PUK_RST0   (P8OUT &= ~BIT2)
#define PUK_AE1_1   (P8OUT |= BIT1)
#define PUK_AE1_0   (P8OUT &= ~BIT1)
#define PUK_AE2_1   (P8OUT |= BIT0)
#define PUK_AE2_0   (P8OUT &= ~BIT0)


#define PUK_EN1    (P2OUT |= BIT6)
#define PUK_EN0    (P2OUT &= ~BIT6)



//unsigned char spitx_buf[6],spirx_buf[6];

void Pkur_IoInit(void)
{
    P8DIR |= (BIT0 | BIT1 | BIT2);
    P2DIR |= (BIT6 | BIT7);
    P3DIR |= BIT3;
    P3DIR &= ~BIT4;
}


/*******************************************************************************************************************
** ��������: void SD_HardWareInit()				Name:	  void SD_HardWareInit()
** ��������: ��ʼ������SD����Ӳ������			Function: initialize the hardware condiction that access sd card
** �䡡��: ��									Input:	  NULL
** �䡡��: ��									Output:	  NULL
********************************************************************************************************************/
void Pkur_HwInit(void)
{
    //P5DIR |= BIT6;
    //P3OUT &= (~BIT1)&(~BIT3);//IO����
    //P3DIR |=  BIT0+BIT1+BIT3;//IO����
    PUK_RST1;       // high reset
    delay_ms(10);
    PUK_RST0;
    
    PUK_AE1_1;   // 
    PUK_AE2_0;
        
    Spi_Si0;
    
    PUK_EN1;

    #if 0
    Spi_Sck0;
    Spi_Sck1;
    Spi_Sck0;
    Spi_Sck1;
    Spi_Sck0;
    PUK_EN0;
    #endif
}
/*******************************************************************************************************************
** ��������: void SPI_SendByte()				Name:	  void SPI_SendByte()
** ��������: ͨ��SPI�ӿڷ���һ���ֽ�			Function: send a byte by SPI interface
** �䡡��: INT8U byte: ���͵��ֽ�				Input:	  INT8U byte: the byte that will be send
** �䡡��: ��									Output:	  NULL
********************************************************************************************************************/
unsigned char SPI_SendByte(unsigned char byte)
{

    unsigned char i,j;
    j=0;
    //P3IN |=BIT2;
    Spi_Sck0;
    for(i=0;i<8;i++)           //"������
    { 
        if(byte&0x80)Spi_Si1;
        else Spi_Si0;
        byte=byte<<1;

        #if 0
        for(int z = 0;z < 60;z++)//60
        {
            _NOP();                 //"����оƬ�ٶ��뵥Ƭ��ʱ�ӵ�����ʱ
        }
        #endif
        
        Spi_Sck1;
        //_NOP();
        //_NOP();
        Spi_Sck0;//�½��ض�����
        j=j<<1;
        if(Spi_So)j=j|0x01;     //"������
    }
    return j;
}
/*******************************************************************************************************************
** ��������: INT8U SPI_RecByte()				Name:	  INT8U SPI_RecByte()
** ��������: ��SPI�ӿڽ���һ���ֽ�				Function: receive a byte from SPI interface
** �䡡��: ��									Input:	  NULL
** �䡡��: �յ����ֽ�							Output:	  the byte that be received
********************************************************************************************************************/
unsigned char SPI_RecByte(void)
{
    return SPI_SendByte(0XFF);
}
//2.2   оƬ����
/************************************************************************************************/
/*ָ�����                                                                                      */
/************************************************************************************************/
//����ͨ������
const unsigned char  tunnel_SW[2] = {0XD8,0X00};
//����ͨ����ֵ
const unsigned char  tunnel_WriteLM[] = {0XC0,0XC8,0XD0};
//������ͨ������������
const unsigned char  tunnel_CLR[] = {0XE0,0XE8,0XF0};
//��ȡ����ͨ������
const unsigned char  tunnel_READ[4][2] = {{0X20,0X00},{0X24,0X00},{0X28,0X00},{0X2C,0X00}};
/************************************************************************************************/
//��ȡ����ͨ������
//data:�����ļ���ֵ�����ڴ����׵�ַ,��СΪ6�ֽڣ�
//��0��1�ֽ�Ϊ��һͨ���ļ���ֵ������ֵΪ16λ�޷�������������0�ֽ�Ϊ����ֵ�ĵ�8λ����1�ֽ�Ϊ��8λ��
//��2��3�ֽ�Ϊ�ڶ�ͨ���ļ���ֵ����ʽͬ�ϣ�
//��4��5�ֽ�Ϊ����ͨ���ļ���ֵ����ʽͬ�ϣ�
/************************************************************************************************/
void SGetCount(unsigned char *data)
{
    unsigned char databuf[2];
    for(int i = 0;i < 1;i++)
    {
        SPI_SendByte(tunnel_READ[i][0]);
        SPI_SendByte(tunnel_READ[i][1]);
        databuf[0] = SPI_SendByte(0X00);
        databuf[1] = SPI_SendByte(0X00);
        SPI_SendByte(0X00);
        SPI_SendByte(0X00);
        data[i*2] = (databuf[1]);
        data[i*2+1] = (databuf[0]);        
    }    
}

/************************************************************************************************/
//������ͨ������������
/************************************************************************************************/
void SCLR(void)
{
    for(int i = 0;i < 1;i++)
    {
        SPI_SendByte(tunnel_CLR[i]);
        SPI_SendByte(tunnel_READ[i][0]);
        SPI_SendByte(tunnel_READ[i][1]);
        SPI_SendByte(0X00);
        SPI_SendByte(0X00);
        SPI_SendByte(0X00);
        SPI_SendByte(0X00);
    }
}

/************************************************************************************************/
//��ͨ��
//data:Ҫ�򿪵�ͨ����
//04��һͨ����
//02�ڶ�ͨ����
//01����ͨ����
/************************************************************************************************/
void SMeasreStart(unsigned char data)
{
    SPI_SendByte(tunnel_SW[0]|(data&0X07));
    SPI_SendByte(tunnel_SW[1]);
}

/************************************************************************************************/
//�ر�����ͨ��
/************************************************************************************************/
void SMeasreStop(void)
{
    SPI_SendByte(tunnel_SW[0]);
    SPI_SendByte(tunnel_SW[1]);
}

/************************************************************************************************/
//������ֵ
//datain:Ҫ������ֵ�����ڵ��ڴ��׵�ַ������Ϊ3���ֽ�
//��0��1��2�ֽڷֱ�Ϊ��һ��������ͨ����ֵ
/************************************************************************************************/
void SSetLimen(unsigned char *datain)
{
    for(int i = 0;i < 1;i++)//������ֵ
    {
        SPI_SendByte(tunnel_WriteLM[i]|(datain[i]>>5));
        SPI_SendByte(datain[i]<<3);
    }

}


void Pkur_Init()
{
    BYTE dd[3] = {200,2,2};
    
    Pkur_IoInit();
    Pkur_HwInit();

    SMeasreStart(4);
    SSetLimen(dd);
    SCLR();
}

#endif

