#ifndef __UART_H__
#define __UART_H__


#define UART_BUF_LEN   64

//ÅäÖÃ´®¿Ú²¨ÌØÂÊ£¬
#define UART_USE_BAUDRATE 9600
#define UART_MAX_BAUDRATE 115200
#define UART_MIN_BAUDRATE 9600

#define ENABLE_IR_IRQ   UCA1IE |= UCRXIE
#define DISABLE_IR_IRQ  UCA1IE &= ~UCRXIE;   


void InitUart(void);
void Clear_Uart_Buf();
void Handle_Uart_Rec();
void Uart_Send_Data(BYTE *Data, int Len);

void PrintFloat(float val);
void PrintDword(DWORD val);
void PrintCounter(float Cnt, BYTE Unit);



#endif
