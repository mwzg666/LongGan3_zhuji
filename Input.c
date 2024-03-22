#include <stdio.h>
#include <string.h>
#include <msp430x552x.h>
#include "system.h"
#include "CommDataDefine.h"

#include "Keyboard.h"
#include "oled.h"

#include "OtherParam.h"
#include "Measinter.h"
#include "RadioSet.h"
#include "DataStore.h"
#include "Background.h"
#include "MainMenu.h"
#include "SsParam.h"
#include "Senser.h"
#include "Main.h"
#include "Input.h"

#define INPUTTYPE_BYTE   0
#define INPUTTYPE_WORD   1
#define INPUTTYPE_DWORD  2
#define INPUTTYPE_FLOAT  3

extern INTERFACE currUI;
extern WORD EventTimer;
extern char StrTemp[24];
char StrHint[24] = {0};    

INPUTBYTE   InByte;
INPUTWORD   InWord;
INPUTDWORD  InDword;
INPUTFLOAT  InFloat;
INPUTEXT    InEx;
FLOATEX     FloatEx;


static BYTE  TempByte;
static WORD  TempWord;
static DWORD TempDword;
static float TempFloat;
static ReturnFun HintRf;
static ReturnFun CancelRf;


const unsigned char Hline[] = {
      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    };

const unsigned char Vline[] = {    // 128*2
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0
    };

void PopRect(BYTE x, BYTE y, BYTE w, BYTE h, char *Title)
{
    BYTE tw = 13*(strlen(Title)/2);   // 标题宽度
    BYTE tx = x+(w-tw)/2;
    
    Clear(x, y,   w+2, h+2);
    Diplay(x, y,   8, h,Vline,0);  // 左
    Diplay(x, y,   w, 2,Hline,0);  // 上

    //标题居中显示
    Clear(tx, y-4,   tw, 12);
    Show12X12String(tx,y-4,Title,0);
    Diplay(tx+tw-1,y,  8,2,Hline,0);  // 上

    
    Diplay(x, y+h,    w+2,2,Hline,0);  // 下
    //Diplay(x+4,y+h+2, w+2,2,Hline,0);
    //Diplay(x+4,y+h+4, w+2,2,Hline,0);
    
    Diplay(x+w,y,     8,h,Vline,0);  // 右
    //Diplay(x+w+2,y+4, 8,h,Vline,0);
    //Diplay(x+w+4,y+4, 8,h,Vline,0);

}


// x>8 y>8 w<112 h<112  四周留8个点的位置
void PopWindow(BYTE x, BYTE y, BYTE w, BYTE h, char *Title)
{
    #if 0
    const unsigned char Hline[] = {
      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    };

    const unsigned char Vline[] = {    // 128*2
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
      0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0
    };
    

    BYTE tw = 13*(strlen(Title)/2);   // 标题宽度
    BYTE tx = x+(w-tw)/2;
    
    Clear(x-8, y-8,   w+18, h+16);
    Diplay(x, y,   8, h,Vline,0);  // 左
    Diplay(x, y,   w, 2,Hline,0);  // 上

    //标题居中显示
    Clear(tx, y-4,   tw, 12);
    Show12X12String(tx,y-4,Title,0);
    Diplay(tx+tw-1,y,  8,2,Hline,0);  // 上

    
    Diplay(x, y+h,    w+2,2,Hline,0);  // 下
    Diplay(x+4,y+h+2, w+2,2,Hline,0);
    Diplay(x+4,y+h+4, w+2,2,Hline,0);
    
    Diplay(x+w,y,     8,h,Vline,0);  // 右
    Diplay(x+w+2,y+4, 8,h,Vline,0);
    Diplay(x+w+4,y+4, 8,h,Vline,0);

    #endif
    Clear(x-8, y-8,   w+18, h+16);
    PopRect(x,y,w,h,Title);

    Diplay(x+4,y+h+2, w+2,2,Hline,0);
    Diplay(x+4,y+h+4, w+2,2,Hline,0);
    
    Diplay(x+w+2,y+4, 8,h,Vline,0);
    Diplay(x+w+4,y+4, 8,h,Vline,0);
}


// 每行6个汉字，12个字符
// 返回长度
BYTE GetOneLine(BYTE StartLoc, char *str)
{
    BYTE i,len = strlen(str),ret=0,j=0;
    for (i=StartLoc;i<len;i++)
    {
        if (str[i] > 0x80) // 中文
        {
            if (j >= 11)
            {
                return ret;
            }
            else
            {
                i++;
                ret += 2;
                j+=2;
            }
        }
        else
        {
            if (str[i] == '@')
            {
                i++;
                ret += 2;
                j++;
            }
            else
            {
                ret ++;
                j++;
            }
        }

        if (j >= 12)
        {
            return ret;
        }
    }

    return ret;
}



void PopHint(char *Hint)
{
    BYTE len;
    BYTE i;
    
    // 一行最多显示6个汉字
    len = strlen(Hint);
    if (len > 24)
    {
        PopWindow(16,30,96,76,"提示");
        
        len = GetOneLine(0,Hint);
        memset(StrTemp,0,24);
        memcpy(StrTemp,Hint,len);
        Show12X12String(24,46,StrTemp,0);

        i = len;
        len = GetOneLine(i,Hint);
        memset(StrTemp,0,24);
        memcpy(StrTemp,&Hint[i],len);
        Show12X12String(24,62,StrTemp,0);

        i += len;
        Show12X12String(24,78,&Hint[i],0);
    }
    else if (len > 12) 
    {
        PopWindow(16,36,96,60,"提示");

        len = GetOneLine(0,Hint);
        memset(StrTemp,0,24);
        memcpy(StrTemp,Hint,len);
        Show12X12String(24,52,StrTemp,0);
        
        Show12X12String(24,68,&Hint[len],0);
    }
    else
    {
        PopWindow(16,40,96,44,"提示");
        Show12X12String(24,56,Hint,0);
    }
    DisplayRefresh();
}


void ShowHint(char *Hint, ReturnFun Rf)
{
    currUI = HINT;
    HintRf = Rf;
    EventTimer = 0;
    PopHint(Hint);
}

void HintKey(PRESSKEY key)
{
    // 任意键或者超时返回
    HintRf();

    #if 0
    switch (key)
    {
        case TIMEOUT: HintRf();  break;
        case OKKEY:   
    }
    #endif
}

void ShowComfirm(char *Hint, ReturnFun Rf_Ok, ReturnFun Rf_Cancel)
{
    currUI = COMFIRM;
    HintRf = Rf_Ok;
    CancelRf = Rf_Cancel;
    PopHint(Hint);
}

void ComfirmKey(PRESSKEY key)
{
    // 任意键或者超时返回
    //HintRf();

    switch (key)
    {
        case TIMEOUT: CancelRf();  break;
        case OKKEY:   HintRf();    break;
        case RETURN:  CancelRf();  break;
    }
    
}



void PopInput(BYTE Type)
{
    //char Src[32] = {0}; 
    memset(StrTemp,0,24);
    PopWindow(12,40,100,46,"请输入");

    if (Type == INPUTTYPE_FLOAT) 
    {
        Show12X12String(20,52,InFloat.Title,0);
        switch (InFloat.Bits)
        {
            case 0: sprintf(StrTemp,"%0.0f %s",TempFloat,InFloat.Spec); break;
            case 1: sprintf(StrTemp,"%0.1f %s",TempFloat,InFloat.Spec); break;
            case 2: sprintf(StrTemp,"%0.2f %s",TempFloat,InFloat.Spec); break;
            case 3: sprintf(StrTemp,"%0.3f %s",TempFloat,InFloat.Spec); break;
        }
        Show5X7String(20,72,StrTemp,0);
    }
    else if (Type == INPUTTYPE_DWORD)
    {
        Show12X12String(20,52,InDword.Title,0);
        sprintf(StrTemp,"%lu %s",TempDword,InDword.Spec);
        GT_Show8X16Ascii(20,68,StrTemp,0);
    }
    else if (Type == INPUTTYPE_WORD)
    {
        Show12X12String(20,52,InWord.Title,0);
        sprintf(StrTemp,"%lu %s",(DWORD)TempWord,InWord.Spec);
        GT_Show8X16Ascii(20,68,StrTemp,0);
    }
    else if (Type == INPUTTYPE_BYTE)
    {
        Show12X12String(20,52,InByte.Title,0);
        sprintf(StrTemp,"%lu %s",(DWORD)TempByte,InByte.Spec);
        GT_Show8X16Ascii(20,68,StrTemp,0);        
    }
        

    DisplayRefresh();
}


void InputByte()
{
    currUI = INBYTE;
    TempByte = *InByte.Val;
    
    PopInput(INPUTTYPE_BYTE);
}

void InputByteKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InByte.Rf();   break;

        case UP:
        {
            TempByte += InByte.Step;
            if (TempByte > InByte.Max)
            {
                TempByte = InByte.Min;
            }
            PopInput(INPUTTYPE_BYTE);
        }
        break;
        
        case DOWN:
        {
            // 要考虑溢出的情况
            if (TempByte < InByte.Step)
            {
                TempByte = InByte.Max;
            }
            else
            {
                TempByte -= InByte.Step;
                if (TempByte < InByte.Min)
                {
                    TempByte = InByte.Max;
                }
            }
            PopInput(INPUTTYPE_BYTE);
        }
        break;
        
        case OKKEY:
        {
            *InByte.Val = TempByte;
            InByte.Rf();
        }
        break;
        
    }
    
}


void InputWord()
{
    currUI = INWORD;
    TempWord = *InWord.Val;
    
    PopInput(INPUTTYPE_WORD);
}

void InputWordKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InWord.Rf();   break;

        case UP:
        {
            TempWord += InWord.Step;
            if (TempWord > InWord.Max)
            {
                TempWord = InWord.Min;
            }
            PopInput(INPUTTYPE_WORD);
        }
        break;
        
        case DOWN:
        {
            if (TempWord < InWord.Step)
            {
                TempWord = InWord.Max;
            }
            else
            {
                TempWord -= InWord.Step;
                if (TempWord < InWord.Min)
                {
                    TempWord = InWord.Max;
                }
            }
            PopInput(INPUTTYPE_WORD);
        }
        break;
        
        case OKKEY:
        {
            *InWord.Val = TempWord;
            InWord.Rf();
        }
        break;
        
    }
    
}

#if 0
void InputDword()
{
    currUI = INDWORD;
    TempDword = *InDword.Val;
    
    PopInput(INPUTTYPE_DWORD);
}

void InputDwordKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InDword.Rf();   break;

        case UP:
        {
            TempDword += InDword.Step;
            if (TempDword > InDword.Max)
            {
                TempDword = InDword.Min;
            }
            PopInput(INPUTTYPE_DWORD);
        }
        break;
        
        case DOWN:
        {
            if (TempDword < InDword.Step)
            {
                TempDword = InDword.Max;
            }
            else
            {
                TempDword -= InDword.Step;
                if (TempDword < InDword.Min)
                {
                    TempDword = InDword.Max;
                }
            }
            PopInput(INPUTTYPE_DWORD);
        }
        break;
        
        case OKKEY:
        {
            *InDword.Val = TempDword;
            InDword.Rf();
        }
        break;
        
    }
    
}
#endif



void InputFloat()
{
    currUI = INFLOAT;
    TempFloat = *InFloat.Val;

    PopInput(INPUTTYPE_FLOAT);
}

void InputFloatKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InFloat.Rf();       break;

        case UP:
        {
            TempFloat += InFloat.Step;
            if (TempFloat > InFloat.Max)
            {
                TempFloat = InFloat.Min;
            }
            PopInput(INPUTTYPE_FLOAT);
        }
        break;
        
        case DOWN:
        {
            if (TempFloat < InFloat.Step)
            {
                TempFloat = InFloat.Max;
            }
            else
            {
                TempFloat -= InFloat.Step;
                if (TempFloat < InFloat.Min)
                {
                    TempFloat = InFloat.Max;
                }
            }
            PopInput(INPUTTYPE_FLOAT);
        }
        break;
        
        case OKKEY:
        {
            *InFloat.Val = TempFloat;
            InFloat.Rf();
        }
        break;
        
    }
    
}



//  Input Float EX  任意浮点数  ----------->
void InitBits()
{
    long val = (long)(*FloatEx.Val);
    switch (FloatEx.Bp)
    {
        case 1: 
        {
            FloatEx.Bits[0] = (int)(val%10);
            break;
        }
        
        case 2: 
        {
            FloatEx.Bits[0] = (int)(val%100/10);
            FloatEx.Bits[1] = (int)(val%10);
            break;
        }
        
        case 3: 
        {
            FloatEx.Bits[0] = (int)(val%1000/100);
            FloatEx.Bits[1] = (int)(val%100/10);
            FloatEx.Bits[2] = (int)(val%10);
            break;
        }
        
        case 4: 
        {
            FloatEx.Bits[0] = (int)(val%10000/1000);
            FloatEx.Bits[1] = (int)(val%1000/100);
            FloatEx.Bits[2] = (int)(val%100/10);
            FloatEx.Bits[3] = (int)(val%10);
            break;
        }

        
        case 5: 
        {
            FloatEx.Bits[0] = (int)(val%100000/10000);
            FloatEx.Bits[1] = (int)(val%10000/1000);
            FloatEx.Bits[2] = (int)(val%1000/100);
            FloatEx.Bits[3] = (int)(val%100/10);
            FloatEx.Bits[4] = (int)(val%10);
            break;
        }
        
        case 6: 
        {
            FloatEx.Bits[0] = (int)(val%1000000/100000);
            FloatEx.Bits[1] = (int)(val%100000/10000);
            FloatEx.Bits[2] = (int)(val%10000/1000);
            FloatEx.Bits[3] = (int)(val%1000/100);
            FloatEx.Bits[4] = (int)(val%100/10);
            FloatEx.Bits[5] = (int)(val%10);
            break;
        }
        
    }


    if (FloatEx.Bn > 0)
    {
        switch (FloatEx.Bn)
        {
            case 1:
            {
                val = (long)(*FloatEx.Val * 10);
                FloatEx.Bits[FloatEx.Bp] = (int)(val % 10);
                break;
            }

            case 2:
            {
                val = (long)(*FloatEx.Val * 100);
                FloatEx.Bits[FloatEx.Bp]   = (int)(val % 100 / 10);
                FloatEx.Bits[FloatEx.Bp+1] = (int)(val % 10);
                break;
            }

            case 3:
            {
                val = (long)(*FloatEx.Val * 1000);
                FloatEx.Bits[FloatEx.Bp]   = (int)(val % 1000 / 100);
                FloatEx.Bits[FloatEx.Bp+1] = (int)(val % 100 / 10);
                FloatEx.Bits[FloatEx.Bp+2] = (int)(val % 10);
                break;
            }

            case 4:
            {
                val = (long)(*FloatEx.Val * 10000);
                FloatEx.Bits[FloatEx.Bp]   = (int)(val % 10000 / 1000);
                FloatEx.Bits[FloatEx.Bp+1] = (int)(val % 1000 / 100);
                FloatEx.Bits[FloatEx.Bp+2] = (int)(val % 100 / 10);
                FloatEx.Bits[FloatEx.Bp+3] = (int)(val % 10);
                break;
            }

            case 5:
            {
                val = (long)(*FloatEx.Val * 100000);
                FloatEx.Bits[FloatEx.Bp]   = (int)(val % 100000 / 10000);
                FloatEx.Bits[FloatEx.Bp+1] = (int)(val % 10000 / 1000);
                FloatEx.Bits[FloatEx.Bp+2] = (int)(val % 1000 / 100);
                FloatEx.Bits[FloatEx.Bp+3] = (int)(val % 100 / 10);
                FloatEx.Bits[FloatEx.Bp+4] = (int)(val % 10);
                break;
            }

            case 6:
            {
                val = (long)(*FloatEx.Val * 1000000);
                FloatEx.Bits[FloatEx.Bp]   = (int)(val % 1000000 / 100000);
                FloatEx.Bits[FloatEx.Bp+1] = (int)(val % 100000 / 10000);
                FloatEx.Bits[FloatEx.Bp+2] = (int)(val % 10000 / 1000);
                FloatEx.Bits[FloatEx.Bp+3] = (int)(val % 1000 / 100);
                FloatEx.Bits[FloatEx.Bp+4] = (int)(val % 100 / 10);
                FloatEx.Bits[FloatEx.Bp+5] = (int)(val % 10);
                break;
            }
        }
    }
}

float BitsToFloat()
{
    float ret;
    switch (FloatEx.Bp)
    {
        case 1: 
        {
            ret = (float)FloatEx.Bits[0];
            break;
        }
        
        case 2: 
        {
            ret = (float)FloatEx.Bits[0]*10 +
                  (float)FloatEx.Bits[1];
            break;
        }
        
        case 3: 
        {
            ret = (float)FloatEx.Bits[0]*100 +
                  (float)FloatEx.Bits[1]*10+
                  (float)FloatEx.Bits[2];
            break;
        }
        
        case 4: 
        {
            ret = (float)FloatEx.Bits[0]*1000 +
                  (float)FloatEx.Bits[1]*100 +
                  (float)FloatEx.Bits[2]*10+
                  (float)FloatEx.Bits[3];
            break;
        }

        
        case 5: 
        {
            ret = (float)FloatEx.Bits[0]*10000 +
                  (float)FloatEx.Bits[1]*1000 +
                  (float)FloatEx.Bits[2]*100 +
                  (float)FloatEx.Bits[3]*10+
                  (float)FloatEx.Bits[4];
            break;
        }
        
        case 6: 
        {
            ret = (float)FloatEx.Bits[0]*100000 +
                  (float)FloatEx.Bits[1]*10000 +
                  (float)FloatEx.Bits[2]*1000 +
                  (float)FloatEx.Bits[3]*100 +
                  (float)FloatEx.Bits[4]*10+
                  (float)FloatEx.Bits[5];
            break;
        }
        
    }

    if (FloatEx.Bn > 0)
    {
        switch (FloatEx.Bn)
        {
            case 1:
            {
                ret += (float)FloatEx.Bits[FloatEx.Bp] * 0.1;
                break;
            }

            case 2:
            {
                ret += (float)FloatEx.Bits[FloatEx.Bp] * 0.1;
                ret += (float)FloatEx.Bits[FloatEx.Bp+1] * 0.01;
                break;
            }

            case 3:
            {
                ret += (float)FloatEx.Bits[FloatEx.Bp] * 0.1;
                ret += (float)FloatEx.Bits[FloatEx.Bp+1] * 0.01;
                ret += (float)FloatEx.Bits[FloatEx.Bp+2] * 0.001;
                break;
            }

            case 4:
            {
                ret += (float)FloatEx.Bits[FloatEx.Bp] * 0.1;
                ret += (float)FloatEx.Bits[FloatEx.Bp+1] * 0.01;
                ret += (float)FloatEx.Bits[FloatEx.Bp+2] * 0.001;
                ret += (float)FloatEx.Bits[FloatEx.Bp+3] * 0.0001;
                break;
            }

            case 5:
            {
                ret += (float)FloatEx.Bits[FloatEx.Bp] * 0.1;
                ret += (float)FloatEx.Bits[FloatEx.Bp+1] * 0.01;
                ret += (float)FloatEx.Bits[FloatEx.Bp+2] * 0.001;
                ret += (float)FloatEx.Bits[FloatEx.Bp+3] * 0.0001;
                ret += (float)FloatEx.Bits[FloatEx.Bp+4] * 0.00001;
                break;
            }

            case 6:
            {
                ret += (float)FloatEx.Bits[FloatEx.Bp] * 0.1;
                ret += (float)FloatEx.Bits[FloatEx.Bp+1] * 0.01;
                ret += (float)FloatEx.Bits[FloatEx.Bp+2] * 0.001;
                ret += (float)FloatEx.Bits[FloatEx.Bp+3] * 0.0001;
                ret += (float)FloatEx.Bits[FloatEx.Bp+4] * 0.00001;
                ret += (float)FloatEx.Bits[FloatEx.Bp+5] * 0.000001;
                break;
            }
            
        }
    }
    
    return ret;
}


char FloatExIndex = 0;
void RefreshFloatEx()
{
    BYTE i;
    PopRect(FloatEx.x,FloatEx.y,(FloatEx.Bp+FloatEx.Bn+1)*6+8,20,"");

    for (i=0;i<FloatEx.Bp;i++)
    {
        Show6X12Int(FloatEx.x+4 + i*6, FloatEx.y+6, FloatEx.Bits[i], FloatExIndex == i);
    }

    if (FloatEx.Bn > 0)
    {
        Show5X7String(FloatEx.x+4 + i*6 , FloatEx.y+7 , "." , 0);
        //i++;

        for (BYTE j=0;j<FloatEx.Bn;j++)
        {
            Show6X12Int(FloatEx.x+10 + i*6, FloatEx.y+6, FloatEx.Bits[i], FloatExIndex == i);
            i++;
        }
    }

    DisplayRefresh();
}



void InputFloatEx()
{
    currUI = INFLOATEX;
    FloatExIndex = 0;
    InitBits();
    
    RefreshFloatEx();
}

void FloatExKey(PRESSKEY key)
{
    float ret;
    
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  FloatEx.Rf();       break;

        case MODE:  
        {
            if (++FloatExIndex >= FloatEx.Bp + FloatEx.Bn)
            {
                FloatExIndex = 0;
            }
            RefreshFloatEx();
            break;
        }

        case UP:
        {
            if (++FloatEx.Bits[FloatExIndex]>9)
            {
                FloatEx.Bits[FloatExIndex] = 0;
            }
            RefreshFloatEx();
        }
        break;
        
        case DOWN:
        {
            if (--FloatEx.Bits[FloatExIndex]<0)
            {
                FloatEx.Bits[FloatExIndex] = 9;
            }
            RefreshFloatEx();
        }
        break;
        
        case OKKEY:
        {
            ret = BitsToFloat();
            if ((ret > FloatEx.Max) || (ret < FloatEx.Min))
            {
                memset(StrHint,0,24);
                sprintf(StrHint,"超出范围(%0.2f--%0.2f)",FloatEx.Min,FloatEx.Max);
                ShowHint(StrHint,FloatEx.Rf);
            }
            else
            {
                *FloatEx.Val = BitsToFloat();
                FloatEx.Rf();
            }
        }
        break;
        
    }
    
}


// --- 带单位的浮点数输入----
char FloatUnitIndex = 1;
void RefreshFloatUnitUI()
{
    PopWindow(12,30,100,74,"请输入");
    
    Show12X12String(20,40,InEx.Title,0);

    Show12X12String(20, 56 ,"单位", FloatUnitIndex == 1);
    Show5X7String(46, 59 ,":", 0);
    Show5X7String(52, 59 ,InEx.UnitList[InEx.CurrUnit], 0);

    Show12X12String(20, 72 ,"阈值", FloatUnitIndex == 2);
    Show5X7String(46, 75 ,":", 0);
    Show5X7Float(52, 75, InEx.InputVal ,0);

    Show12X12String(50, 88 ,"确认", FloatUnitIndex == 3);
    
    DisplayRefresh();
}

void InputFloatUnit()
{
    currUI = INFLOATUNIT;
    RefreshFloatUnitUI();
}



void FloatUnitKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InEx.Rf_Cancel();       break;

        case DOWN:
        {
            if (++FloatUnitIndex > 3)
            {
                FloatUnitIndex = 1;
            }
            RefreshFloatUnitUI();
        }
        break;
        
        case UP:
        {
            if (--FloatUnitIndex < 1)
            {
                FloatUnitIndex = 3;
            }
            RefreshFloatUnitUI();
        }
        break;
        
        case OKKEY:
        {
            switch (FloatUnitIndex)
            {
                case 1: SelectUnit(); break;
                
                case 2: 
                {
                    FloatEx.Bp = 3;
                    FloatEx.Bn = 2;
                    FloatEx.Max = 1000;
                    FloatEx.Min = 0;
                    FloatEx.Val = &(InEx.InputVal);
                    FloatEx.x = 50;
                    FloatEx.y = 68;
                    FloatEx.Rf = InputFloatUnit;
                    InputFloatEx();
                    break;
                }
                
                case 3: 
                {
                    
                    InEx.Rf_Ok(); 
                    break;
                }
            }
        }
        break;
        
    }
}

char SelectUnitIndex = 1;
void RefreshSelectUnit()
{
    BYTE i;
    PopWindow(52,56,40,9*InEx.UnitCount+4,"");
    for (i=0;i<InEx.UnitCount;i++)
    {
        Show5X7String(58,60+i*9,InEx.UnitList[i], SelectUnitIndex == i+1);
    }
    DisplayRefresh();
}

void SelectUnit()
{
    currUI = SELUNIT;
    SelectUnitIndex = InEx.CurrUnit+1;
    RefreshSelectUnit();
}

void SelectUnitKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InputFloatUnit();       break;

        case UP:
        {
            if (--SelectUnitIndex < 1)
            {
                SelectUnitIndex = InEx.UnitCount;
            }
            RefreshSelectUnit();
        }
        break;
        
        case DOWN:
        {
            if (++SelectUnitIndex > InEx.UnitCount)
            {
                SelectUnitIndex = 1;
            }
            RefreshSelectUnit();
        }
        break;
        
        case OKKEY:
        {
            InEx.CurrUnit = SelectUnitIndex - 1;
            InputFloatUnit();
        }
        break;
        
    }
}


