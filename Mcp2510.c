#include <msp430.h>
#include "CommDataDefine.h"
#include "system.h"
#include "MCP2510.h"
#include "SelfCheck.h"

/********************** MCP2510 Instruction *********************************/
#define MCP2510INSTR_RESET      0xc0
#define MCP2510INSTR_READ       0x03
#define MCP2510INSTR_WRITE      0x02
#define MCP2510INSTR_RTS        0x80    //request to send
#define MCP2510INSTR_RDSTAT     0xa0   //read status
#define MCP2510INSTR_BITMDFY    0x05   //bit modify


#define SPI_CS(x)   (x)?(P3OUT|=BIT7):(P3OUT&=~BIT7)
#define SPI_CLK(x)  (x)?(P4OUT|=BIT0):(P4OUT&=~BIT0)
#define SPI_MOSI(x) (x)?(P5OUT|=BIT6):(P5OUT&=~BIT6)
#define SPI_MISO    (P3IN&(1<<6))?1:0


#define MCP2510_Enable()    SPI_CS(0)
#define MCP2510_Disable()   SPI_CS(1)

extern CHECKRES CheckRes;

BYTE Can_RxInt = 0;    // Can �����жϱ�־

void Can_IoInit()
{
    
    //                        VerB                     VerC
    // CAN_MISO -- P4.5  --input  --- P3.6
    // CAN_MOSI -- P4.4               --- P5.6
    // CAN_EN     -- P4.3
    // CAN_CLK   -- P4.0
    // CAN_NCS  -- P3.7
    // CAN_SLOPE - P3.6
    P4DIR |= BIT3 | BIT0;  // CAN_EN   CAN_CLK
    P5DIR |= BIT6;   // CAN_MOSI
    P3DIR |= BIT7;   // CAN_NCS
    P3DIR &= ~BIT6;  // CAN_MISO

    
    // CAN_INT     -- P2.5
    // CAN_RX1BF -- P2.4
    // CAN_RX0BF -- P2.3
    P2DIR &= ~(BIT3 | BIT4 | BIT5);
    #ifdef BATTERY_VER
    P2IES |= BIT3 | BIT4;
    P2IE  |= BIT3 | BIT4;
    #else
    P2IES |= BIT5;
    P2IE  |= BIT5;
    #endif
}

BOOL CanBus_Init(void)
{
    CAN_SHDN(1);
    
    if (init_MCP2510(BandRate_100kbps))
    {
        canSetup();//���öԱ�Ž��й���
        return TRUE;
    }
    //_EINT();

    return FALSE;
}


void SendSIOData(U8 dat)  
{
    int i=0;

    for(i=0;i<8;i++)
    {
        SPI_CLK(0); //P3OUT &= ~0x01;
        if(dat&0x80)
        {
            SPI_MOSI(1); //P3OUT|=0x10;
        }
        else
        {
            SPI_MOSI(0);  //P3OUT &=~0x10;
        }
        dat <<= 1;
        SPI_CLK(1);  //P3OUT |= 0x01;     
    }
    SPI_CLK(0); //P3OUT &= ~0x01;
}

U8 ReadSIOData()
{
    U8 x = 0;

    int i=0;
    SPI_CLK(0); //P3OUT &= ~0x01;
    for(i=0;i<8;i++)
    {

        SPI_CLK(1); //P3OUT |= 0x01;      
        x <<= 1;
        //if(P3IN&0x020)
        if (SPI_MISO)
        {
            x |=0x01;
        }

        SPI_CLK(0); //P3OUT &= ~0x01;     
    }
    SPI_CLK(0); //P3OUT &= ~0x01;

    return x;

}




/*******************************************************************************
���ܣ�ʹ�ܻ��ֹCAN�������
���룺char Enable  1:�������
                   0:��ֹ���
�������
*******************************************************************************/
void CAN_EN(char Enable)
{
    if (Enable == 1)
    {
        #ifdef BATTERY_VER
        CAN_SHDN(1);
        #else
        P4OUT |= BIT3;
        #endif
    }
    else
    {
        #ifndef BATTERY_VER
        P4OUT &= ~BIT3;
        #endif

        SPI_CS(0);
        SPI_CLK(0);
        SPI_MOSI(0);

        CAN_SHDN(0);
    }
}


/*******************************************************************************
���ܣ�����CAN��������б��
���룺char HighSpeed  1:����CAN
                      0:����CAN
�������
*******************************************************************************/
void CAN_SLOPE(char HighSpeed)
{
    #if 0   // donot use
    if (HighSpeed == 1)
    {
        P3OUT |= BIT6;
    }
    else
    {
        P3OUT &= ~BIT6;
    }
    #endif
}


// ��ƽת��оƬ����
void CAN_SHDN(BYTE St)
{
    if (St)
    {
        P5OUT |= BIT1;
    }
    else
    {
        P5OUT &= ~BIT1;
    }
}


void MCP2510_Sleep()
{
    MCP2510_Write(CLKCTRL, MODE_SLEEP| CLK1);
    CAN_SHDN(0);
}

void MCP2510_WakeUp()
{
    CAN_SHDN(1);
    MCP2510_WriteBits(CANINTF, WAKINT,WAKINT);
    delay_ms(2);
    MCP2510_WriteBits(CANINTF, ~WAKINT,WAKINT);
    delay_ms(2);
    MCP2510_Write(CLKCTRL, MODE_NORMAL| CLK1);
    delay_ms(2);

    //����̽ͷ
    //CanSendCmd(SCMD_READ_STATUS,CheckRes.SenserType,0);
    //delay_ms(20);
}

#if 0
void MCP2510_Enable()
{
    MCP2510_WakeUp();
    SPI_CS(0);
}

void MCP2510_Disable()
{
    SPI_CS(1);
    MCP2510_Sleep();
}
#endif


/*******************************************************************************
���ܣ���λCAN(ԭ����)
���룺��
�������
*******************************************************************************/
void MCP2510_Reset()
{
    MCP2510_Enable();

    SendSIOData(MCP2510INSTR_RESET);

    MCP2510_Disable();
}






/*******************************************************************************
���ܣ�дCANоƬ(ԭ����)
���룺int address:д���ַ
      int value  :д��ֵ,8λ��Ч��
�������
*******************************************************************************/
void MCP2510_Write(int address, int value)
{
    MCP2510_Enable();

    SendSIOData(MCP2510INSTR_WRITE);
    SendSIOData((unsigned char)address);
    SendSIOData((unsigned char)value);


    MCP2510_Disable();
}

/*******************************************************************************
���ܣ���CANоƬ(ԭ����)
���룺int address:д���ַ
      
�����unsigned char value  :��ȡ���,8λ��Ч��
*******************************************************************************/
unsigned char MCP2510_Read(int address)
{
    unsigned char result;

    //U8 Dat[2] = {0};

    MCP2510_Enable();

    SendSIOData(MCP2510INSTR_READ);
    SendSIOData((unsigned char)address);


    //SendSIOData(0);
    result=ReadSIOData();

    MCP2510_Disable();

    return result;
}



/*******************************************************************************
���ܣ���CAN״̬(ԭ����)
���룺��
      
�����unsigned char value  :��ȡ���,8λ��Ч��
*******************************************************************************/
unsigned char MCP2510_ReadStatus()
{
	unsigned char result1;
    unsigned char result2;

	MCP2510_Enable();

	SendSIOData(MCP2510INSTR_RDSTAT);

	result1=ReadSIOData();//SendSIOData(0);
	result2=ReadSIOData();

	MCP2510_Disable();

    if(result1 == result2)
    {
	   return result1;
    }
    else
    {
       return 0x00;
    }
}



/*******************************************************************************
���ܣ�CANоƬλд��(ԭ����)
���룺int address:д���ַ
      int data  :д��ֵ
      int mask  :λ���Σ�д1��λ�ñ�ʾд�룬0��λ�ñ�ʾ��д��
�������
*******************************************************************************/
void MCP2510_WriteBits( int address, int data, int mask )
{
	MCP2510_Enable();

	SendSIOData(MCP2510INSTR_BITMDFY);
	SendSIOData((unsigned char)address);
	SendSIOData((unsigned char)mask);
	SendSIOData((unsigned char)data);

	MCP2510_Disable();
}




/*******************************************************************************
���ܣ�����CAN ������(ԭ����)
���룺CanBandRate bandrate:������,
        BandRate_100kbps/BandRate_125kbps/BandRate_250kbps/BandRate_500kbps/BandRate_1Mbps
      BOOL IsBackNormal  :��ȡ���,8λ��Ч��
�������
*******************************************************************************/
void MCP2510_SetBandRate(CanBandRate bandrate, BOOL IsBackNormal)
{
	//
	// Bit rate calculations.
	//
	//Input clock fre=16MHz
	// In this case, we'll use a speed of 125 kbit/s, 250 kbit/s, 500 kbit/s.
	// If we set the length of the propagation segment to 7 bit time quanta,
	// and we set both the phase segments to 4 quanta each,
	// one bit will be 1+7+4+4 = 16 quanta in length.
	//
	// setting the prescaler (BRP) to 0 => 500 kbit/s.
	// setting the prescaler (BRP) to 1 => 250 kbit/s.
	// setting the prescaler (BRP) to 3 => 125 kbit/s.
	//
	// If we set the length of the propagation segment to 3 bit time quanta,
	// and we set both the phase segments to 1 quanta each,
	// one bit will be 1+3+2+2 = 8 quanta in length.
	// setting the prescaler (BRP) to 0 => 1 Mbit/s.

	// Go into configuration mode
	MCP2510_Write(MCP2510REG_CANCTRL, MODE_CONFIG);

	switch(bandrate){
        case BandRate_100kbps:
          //Tq = 2*(BRP10+1)/16MHz = 1.25us
          //Tbit = (1+1+3+3)*Tq = 10us
          //Baut = 100Kbps
		MCP2510_Write(CNF1, SJW1|BRP10);	//Synchronization Jump Width Length =1 TQ
		MCP2510_Write(CNF2, BTLMODE_CNF3|(SEG3<<3)|SEG1); // Phase Seg 1 = 3, Prop Seg = 1
		MCP2510_Write(CNF3, SEG3);// Phase Seg 2 = 3
		break;  
	case BandRate_125kbps:
		MCP2510_Write(CNF1, SJW1|BRP4);	//Synchronization Jump Width Length =1 TQ
		MCP2510_Write(CNF2, BTLMODE_CNF3|(SEG4<<3)|SEG7); // Phase Seg 1 = 4, Prop Seg = 7
		MCP2510_Write(CNF3, SEG4);// Phase Seg 2 = 4
		break;
	case BandRate_250kbps:
		MCP2510_Write(CNF1, SJW1|BRP2);	//Synchronization Jump Width Length =1 TQ
		MCP2510_Write(CNF2, BTLMODE_CNF3|(SEG4<<3)|SEG7); // Phase Seg 1 = 4, Prop Seg = 7
		MCP2510_Write(CNF3, SEG4);// Phase Seg 2 = 4
		break;
	case BandRate_500kbps:
		MCP2510_Write(CNF1, SJW1|BRP1);	//Synchronization Jump Width Length =1 TQ
		MCP2510_Write(CNF2, BTLMODE_CNF3|(SEG4<<3)|SEG7); // Phase Seg 1 = 4, Prop Seg = 7
		MCP2510_Write(CNF3, SEG4);// Phase Seg 2 = 4
		break;
	case BandRate_1Mbps:
		MCP2510_Write(CNF1, SJW1|BRP1);	//Synchronization Jump Width Length =1 TQ
		MCP2510_Write(CNF2, BTLMODE_CNF3|(SEG3<<3)|SEG2); // Phase Seg 1 = 2, Prop Seg = 3
		MCP2510_Write(CNF3, SEG2);// Phase Seg 2 = 1
		break;
	}

	if(IsBackNormal){
		//Enable clock output
		MCP2510_Write(CLKCTRL, MODE_NORMAL | CLK1);
	}
}


/*******************************************\
*	���ж�ȡMCP2510����				*
\*******************************************/
void MCP2510_SRead( int address, unsigned char* pdata, int nlength )
{
	int i;

	MCP2510_Enable();
	SendSIOData(MCP2510INSTR_READ);
	SendSIOData((unsigned char)address);

	for (i=0; i<nlength; i++) {
		//SendSIOData(0);
		*pdata=ReadSIOData();
		pdata++;
	}
	MCP2510_Disable();
}


/*******************************************\
*	����д��MCP2510����				*
\*******************************************/
void MCP2510_Swrite(int address, unsigned char* pdata, int nlength)
{
	int i;
	MCP2510_Enable();

	SendSIOData(MCP2510INSTR_WRITE);
	SendSIOData((unsigned char)address);

	for (i=0; i < nlength; i++) 
    {
		SendSIOData((unsigned char)*pdata);
		pdata++;
	}
	MCP2510_Disable();
}

/*******************************************\
*	��ȡMCP2510 CAN����ID				*
*	����: addressΪMCP2510�Ĵ�����ַ*
*			can_idΪ���ص�IDֵ			*
*	����ֵ								*
*	TRUE����ʾ����չID(29λ)			*
*	FALSE����ʾ����չID(11λ)		*
\*******************************************/
BOOL MCP2510_Read_Can_ID( int address, U32* can_id)
{
	U32 tbufdata;
       
	unsigned char* p=(unsigned char*)&tbufdata;

	MCP2510_SRead(address, p, 4);
      
	*can_id = (tbufdata<<3)|((tbufdata>>13)&0x7);
	*can_id &= 0x7ff;

	if ( (p[MCP2510LREG_SIDL] & TXB_EXIDE_M) ==  TXB_EXIDE_M ) {
          /*
		*can_id = (*can_id<<2) | (p[MCP2510LREG_SIDL] & 0x03);
		*can_id <<= 16;
		*can_id |= tbufdata>>16;
          */
          
		*can_id = (*can_id<<2) | (p[MCP2510LREG_SIDL] & 0x03);
		*can_id <<= 8;
                
		*can_id |= p[MCP2510LREG_EID8]; 
                *can_id <<= 8;
                *can_id |= p[MCP2510LREG_EID0]; 
          /*
           EID = (p[MCP2510LREG_SIDL] & 0x03);
           EID <<= 8;
           EID |= p[MCP2510LREG_EID8]; 
           EID <<= 8;
           EID |= p[MCP2510LREG_EID0];      
           
           
           *can_id = (EID << 11) | *can_id;
            */    
		return TRUE;
	}
	return FALSE;
}

/***********************************************************\
*	��ȡMCP2510 ���յ�����							*
*	����: nbufferΪ�ڼ�������������Ϊ0����1	*
*			can_idΪ���ص�IDֵ							*
*			rxRTR��ʾ�Ƿ���RXRTR						*
*			data��ʾ��ȡ������						*
*			dlc��ʾdata length code							*
*	����ֵ												*
*		TRUE����ʾ����չ����						*
*		FALSE����ʾ����չ����						*
\***********************************************************/
BOOL MCP2510_Read_Can(U8 nbuffer, BOOL* rxRTR, U32* can_id, U8* data , U8* dlc)
{

	U8 mcp_addr = (nbuffer<<4) + 0x61, ctrl;
	BOOL IsExt;

	IsExt=MCP2510_Read_Can_ID( mcp_addr, can_id);

	ctrl=MCP2510_Read(mcp_addr-1);
	*dlc=MCP2510_Read( mcp_addr+4);
	if ((ctrl & 0x08)) {
		*rxRTR = TRUE;
	}
	else{
		*rxRTR = FALSE;
	}
	*dlc &= DLC_MASK;
	MCP2510_SRead(mcp_addr+5, data, *dlc);

	return IsExt;
}


/***********************************************************\
*	д��MCP2510 ���͵�����				    *
*	����: nbufferΪ�ڼ�������������Ϊ0��1��2	    *
*			ext��ʾ�Ƿ�����չ����		    *
*			can_idΪ���ص�IDֵ		    *
*			rxRTR��ʾ�Ƿ���RXRTR		    *
*			data��ʾ��ȡ������		    *
*			dlc��ʾdata length code		    *
*		FALSE����ʾ����չ����			    *
\***********************************************************/
void MCP2510_Write_Can( U8 nbuffer, BOOL ext, U32 can_id, BOOL rxRTR, U8* data,U8 dlc )
{
	U8 mcp_addr = (nbuffer<<4) + 0x31;
	MCP2510_Swrite(mcp_addr+5, data, dlc );  // write data bytes
	MCP2510_Write_Can_ID( mcp_addr, can_id,ext);  // write CAN id
	if (rxRTR)
		dlc |= RTR_MASK;  // if RTR set bit in byte
	MCP2510_Write((mcp_addr+4), dlc);            // write the RTR and DLC
}

/*******************************************\
*	����MCP2510 CAN����ID				*
*	����: addressΪMCP2510�Ĵ�����ַ*
*			can_idΪ���õ�IDֵ			*
*			IsExt��ʾ�Ƿ�Ϊ��չID	*
\*******************************************/
void MCP2510_Write_Can_ID(int address, U32 can_id, BOOL IsExt)
{
	U32 tbufdata;
    U8 *pData = (U8 *)&tbufdata;
	
	if (IsExt) 
	{
          /*
		can_id&=0x1fffffff;	//29λ
		tbufdata=(can_id >> 11) &0xffff;  //ȡEID15...EID0λ,��ID26..ID11
		tbufdata<<=16;                    //�Ÿߵ�ַ
                
		tbufdata|=(can_id << 8)&0x0000E000;//ȡSID2...SID0
                tbufdata|=(can_id >> 19)&0x00000300;//ȡSID2...SID0
		tbufdata |= 0x0800;//TXB_EXIDE_M ;//������չ֡��־
          */
                can_id&=0x1fffffff;	//29λ
                pData[0] = (can_id >> 21) & 0xFF;//ID10 .. ID 3
                pData[1] = ((can_id >> 18) << 5)  & 0xE0;//ID2.. ID0
                pData[1] |=(can_id >> 16) & 0x03;//EID17 .. EID16
                pData[2] = (can_id >> 8) & 0xFF;//EID 15 .. EID8
                pData[3] = can_id  & 0xFF;//EID7 .. EID0
		
		pData[1] |= TXB_EXIDE_M;
	}
	else
	{
		can_id&=0x7ff;	//11λ
		tbufdata= (can_id>>3)|((can_id&0x7)<<13);
	}
	MCP2510_Swrite(address, (unsigned char*)&tbufdata, 4);
}



// Setup the CAN buffers used by the application.
// We currently use only one for reception and one for transmission.
// It is possible to use several to get a simple form of queue.
//
// We setup the unit to receive all CAN messages.
// As we only have at most 4 different messages to receive, we could use the
// filters to select them for us.
//
// mcp_init() should already have been called.
void canSetup(void)
{
    // (RXM<1:0> = 11).
    //���������� 
    // But there is a bug in the chip, so we have to activate roll-over.
	//MCP2510_WriteBits(RXB0CTRL, (RXB_BUKT+RXB_RX_ANY), 0xFF);
	//MCP2510_WriteBits(RXB1CTRL, RXB_RX_ANY, 0xFF);
  
  
  
    MCP2510_WriteBits(RXB0CTRL, RXB_RX_EXT + RXB_BUKT, 0xFF);
    //ֻ������չ֡���ݣ�������RXB0��ʱ���浽RXB1
    MCP2510_WriteBits(RXB1CTRL, RXB_RX_EXT, 0xFF);
    //ֻ������չ֡���ݣ�������RXB0��ʱ���浽RXB1
}

/***********************************************************************************\
								��������
	����:
		data����������

	Note: ʹ������������ѭ�����ͣ�û������������Ч���
\***********************************************************************************/
void canWrite(U32 id, U8 *pdata, unsigned char dlc, BOOL IsExt, BOOL rxRTR)
{
	int ntxbuffer=0;
	MCP2510_Write_Can(ntxbuffer, IsExt, id, rxRTR, pdata, dlc);

	switch(ntxbuffer){
	case 0:
		MCP2510_transmit(TXB0CTRL);
		ntxbuffer=1;
		break;
	case 1:
		MCP2510_transmit(TXB1CTRL);
		ntxbuffer=2;
		break;
	case 2:
		MCP2510_transmit(TXB2CTRL);
		ntxbuffer=0;
		break;
	}
}

BOOL init_MCP2510(CanBandRate bandrate)
{
	unsigned char i,j,a;
	U8 t;

	//pCan = p;
        
    MCP2510_Reset();
    //	for(i=0;i<255;i++)
    //		Uart_Printf("%x\n",MCP2510_Read(i));

	MCP2510_SetBandRate(bandrate,FALSE);

	// Disable interrups.
	MCP2510_Write(CANINTE, NO_IE);

	// Mark all filter bits as don't care:
	//MCP2510_Write_Can_ID(RXM0SIDH, 0x00FFFF00,1);//��������λ����Ŀ��Ϊ�����ĲŽ���
	//MCP2510_Write_Can_ID(RXM1SIDH, 0x00FFFF00,1);//

    
    MCP2510_Write_Can_ID(RXM0SIDH, 0x0000FF00,1);//��������λ����Ŀ��Ϊ�����ĲŽ���
    //MCP2510_Write_Can_ID(RXM1SIDH, 0x0000FF00,1);//

    //MCP2510_Write_Can_ID(RXM0SIDH, 0x00000000,1);//��������λ����Ŀ��Ϊ�����ĲŽ���
	//MCP2510_Write_Can_ID(RXM1SIDH, 0x00000000,1);//
        
    //RXB0�����˲���,���ڽ��չ㲥
    MCP2510_Write_Can_ID(RXF0SIDH, 0x00000000, 1);//��չ֡,���չ㲥֡
	//MCP2510_Write_Can_ID(RXF1SIDH, 0x00000000, 1);//����  ˮ�¸�����PD
	//MCP2510_Write_Can_ID(RXF1SIDH, 0x00000400, 1);
        
    //RXB1�����˲���,���ڽ����Լ���
	//MCP2510_Write_Can_ID(RXF2SIDH, 0x00000300, 1);//���� ˮ���и�����GM
	//MCP2510_Write_Can_ID(RXF3SIDH, 0x00000400, 1);//���� Alpha̽ͷ
	//MCP2510_Write_Can_ID(RXF4SIDH, 0x00000500, 1);//���� Beta̽ͷ
	//MCP2510_Write_Can_ID(RXF5SIDH, 0x00000100, 1);//���� ����PD

       
	//Enable clock output
	//MCP2510_Write(CLKCTRL, MODE_LOOPBACK| CLKEN | CLK1);//�ػ�ģʽ
	MCP2510_Write(CLKCTRL, MODE_NORMAL| CLK1);//��������ģʽ,1/2 CLKl���

        
    //�����������̨�豸����ʵ��Ļ�������ѡ��ػ�ģʽ
    //�����ڳ����ն��п�����ʾ���̵�����  
  
	// Clear, deactivate the three transmit buffers
	a = TXB0CTRL;
	for (i = 0; i < 3; i++) 
	{
		for (j = 0; j < 14; j++) 
		{
			MCP2510_Write(a, 0);
			a++;
	    }
       	a += 2; //���� CANSTAT �� CANCTRL�Ĵ�����ַ
	}
	

	// The two pins RX0BF and RX1BF are used to control two LEDs; set them as outputs and set them as 00.
	//MCP2510_Write(BFPCTRL, 0x3C);
        
    MCP2510_Write(BFPCTRL, 0x3F);//ʹ��RX0BF��RX1BF״̬������յ�����ʱ����

	//Open Interrupt
	MCP2510_Write(CANINTE, RX0IE|RX1IE|WAKIE);

	//CAN_EN(1);
	CAN_SLOPE(0);

	t = MCP2510_Read(CANINTE);
	if (t != 0x43)
	{
        return FALSE;
	}
    
    return TRUE;
}


/***********************************************************************************\
								��ѯ�Ƿ��յ�����
	����ֵ:���û�����ݣ��򷵻�-1��
			���򣬷����յ����ݵĻ�������
	Note: ����������������յ����ݣ��򷵻ص�һ��������
\***********************************************************************************/
int canPoll()
{
	if(MCP2510_ReadStatus()&RX0INT)
		return 0;
	
	if(MCP2510_ReadStatus()&RX1INT)
		return 1;

	return -1;
}

BOOL canRead(int n, U32* id, U8 *pdata,  U8*dlc, BOOL* rxRTR, BOOL *isExt)
{
    U8 byte;
    byte=MCP2510_Read(CANINTF);//��ȡ�ж�״̬���жϱ�־��CPU���

    if(n==0)
    {
        if(byte & RX0INT)
        {
            *isExt=MCP2510_Read_Can(n, rxRTR, id, pdata, dlc);//��ȡRBX0                        
            MCP2510_WriteBits(CANINTF, ~RX0INT, RX0INT); // Clear interrupt
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if(n ==1 )
    {
        if(byte & RX1INT)
        {
            *isExt=MCP2510_Read_Can(n, rxRTR, id, pdata, dlc);//��ȡRBX1
            MCP2510_WriteBits(CANINTF, ~RX1INT, RX1INT); // Clear interrupt
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}



#if 0

void CAN_Test()
{

  unsigned char address = 0x31;
 
 
	int i;
	U32 id;
	unsigned char dlc;
	BOOL rxRTR, isExt;
	BOOL temp;
	
	U8 data[8]={1,2,3,4,5,6,7,8};

	//while(1)
	{
		canWrite(0x123,	 //ID��
                 data,  //����
                 8,     //���ݳ���
                 TRUE, //��չ֡��־
                 FALSE  //Զ��֡��־
                 );

		Sleep(1000);
	}

	#if 0
	while((i=canPoll())==-1);

	temp=canRead(i, &id, data, &dlc, &rxRTR, &isExt);

	RETAILMSG(CAN_MSG,(TEXT("%x %x %x %x %x %x %x %x"),
		                             data[0],data[1],
		                             data[2],data[3],
		                             data[4],data[5],
		                             data[6],data[7]));

    #endif
}
#endif       



