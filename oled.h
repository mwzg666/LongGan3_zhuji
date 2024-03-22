#ifndef OLED_H
#define OLED_H
#endif


#define OLED_HIGH 64
#define OLED_WIDTH 128

#define OLED_PAGES  (OLED_HIGH/8)// 1 page =8 rows
#define OLED_COLUMNS 128

#define OLED_MAX_ROWS OLED_HIGH
#define OLED_MAX_COLUMNS OLED_WIDTH


#define	Brightness	0x7F


void ShowLogo(void);
void Oled_InitReg();
void Oled_Init();
void Write_CMD(unsigned char cmd);
void Write_Data(unsigned char data);
void Oled_RST();
void Set_Col_Address(unsigned char start_col, unsigned char end_col);
//void Set_Row_Address(unsigned char start_row, unsigned char end_row);
void Set_Contrast_Current(unsigned char Current);
void Set_Current_Range(unsigned char Range);
void Set_Remap_Format(unsigned char data);
void Set_Start_Line(unsigned char line);
void Set_Display_Offset(unsigned char data);
void Set_Display_Mode(unsigned char mode);
void Set_Multiplex_Ratio(unsigned char ratio);
void Set_Master_Config(unsigned char data);
void Set_Display_On_Off(unsigned char data);
void Set_Display_Clock(unsigned char data);
void Set_VCOMH(unsigned char d);
void Set_VSL(unsigned char d);
void GA_Option(unsigned char d);

void Oled_Test(char x);
void Draw_Rectangle(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e);

void ShowInitUI();
void Clear(int x, int y, int w, int h);

void Diplay(int x, int y, int w, int h, const unsigned char *InDotBuf,int Reverse);
void DisplayRefresh();
BYTE Show24X36Int( unsigned char row, unsigned char col, DWORD Data);
BYTE Show16X24float(unsigned char row, unsigned char col,float data, BYTE pc);
BYTE Show24X36float( unsigned char row, unsigned char col,float Data,BYTE pc);
void ShowSpecChar(unsigned char row, unsigned char col,char *Data_Pointer,char Reverse,int len);
void Show16X24int(unsigned char row, unsigned char col,unsigned long  data,char Reverse);
void Show5X7Int(unsigned char row, unsigned char col,unsigned int Data,char Reverse);
void Show5X7Float(unsigned char row, unsigned char col,float Data,char Reverse);
void Show7X8Int(unsigned char row, unsigned char col,unsigned int Data,char Reverse);
void Show7X8Float(unsigned char row, unsigned char col,float Data,char Reverse);
void Show6X12Int(unsigned char row, unsigned char col,unsigned int Data,char Reverse);
BYTE Show6X12Float(unsigned char row, unsigned char col,float Data,char Reverse);

void GT_Show5X7Ascii(unsigned char row, unsigned char col,char *Data_Pointer,char Reverse);
void GT_Show7X8Ascii(unsigned char row, unsigned char col,char *Data_Pointer,char Reverse);
void GT_Show6X12Ascii(unsigned char row, unsigned char col,char *Data_Pointer,char Reverse);
void GT_Show8X16Ascii(unsigned char row, unsigned char col,char *Data_Pointer,char Reverse);
void GT_Show12X12Char(unsigned char row, unsigned char col,unsigned char *srcCode,char Reverse);
void GT_Show16X16Char(unsigned char row, unsigned char col,unsigned char *srcCode,char Reverse);
void Show5X7String(unsigned char row, unsigned char col,char *srcCode,char Reverse);
void Show8X16String(unsigned char row, unsigned char col,char *srcCode,char Reverse);
void Show12X12String(unsigned char row, unsigned char col,char *srcCode,char Reverse);
void ShowLine(BYTE Row);
//void ControlKeyInter(char isopen);

void Clear_DispBuf();
void Set_Oled_Dir_IN();
void Set_Oled_Dir_OUT();
unsigned char Read_Oled_RamData();
unsigned char Self_Check_Oled();
void ShowUnit(unsigned char row, unsigned char col,char *srcCode,char Reverse);
