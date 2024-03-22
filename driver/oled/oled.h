#ifndef OLED_H
#define OLED_H
#endif


#define OLED_HIGH 64
#define OLED_WIDTH 128

#define OLED_PAGES  (OLED_HIGH/8)// 1 page =8 rows
#define OLED_COLUMNS 128

#define OLED_MAX_ROWS OLED_HIGH
#define OLED_MAX_COLUMNS OLED_WIDTH



void Oled_InitReg();
void Oled_Init();
void Write_CMD(unsigned char cmd);
void Write_Data(unsigned char data);
void Oled_RST();
void Set_Col_Address(unsigned char start_col, unsigned char end_col);
void Set_Row_Address(unsigned char start_row, unsigned char end_row);
void Set_Contrast_Current(unsigned char Current);
void Set_Current_Range(unsigned char Range);
void Set_Remap_Format(unsigned char data);
void Set_Start_Line(unsigned char line);
void Set_Display_Offset(unsigned char data);
void Set_Display_Mode(unsigned char mode);
void Set_Multiplex_Ratio(unsigned char ratio);
void Set_Master_Config(unsigned char data);
void Set_Display_On_Off(unsigned char data);
void Set_Phase_Length(unsigned char data);
void Set_Frame_Frequency(unsigned char data);
void Set_Display_Clock(unsigned char data);
void Set_Precharge_Compensation(unsigned char a, unsigned char b);
void Set_Precharge_Voltage(unsigned char voltage);
void Set_VCOMH(unsigned char d);
void Set_VSL(unsigned char d);
void GA_Option(unsigned char d);
void Set_Gray_Scale_Table();

void Oled_Test(char x);
void Draw_Rectangle(unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned char e);
void Show_String(unsigned char a, unsigned char *Data_Pointer, unsigned char b, unsigned char c);
void Show_Font57(unsigned char a, unsigned char b, unsigned char c, unsigned char d);