#ifndef __INPUT_H__
#define __INPUT_H__


typedef struct __INPUTBYTE
{
    char *Title;
    char *Spec;  // 单位
    BYTE *Val;
    BYTE Max;
    BYTE Min;
    BYTE Step;
    ReturnFun Rf;
}INPUTBYTE;


typedef struct __INPUTWORD
{
    char *Title;
    char *Spec;  // 单位
    WORD *Val;
    WORD Max;
    WORD Min;
    WORD Step;
    ReturnFun Rf;
}INPUTWORD;


typedef struct __INPUTDWORD
{
    char *Title;
    char *Spec;  // 单位
    DWORD *Val;
    DWORD Max;
    DWORD Min;
    DWORD Step;
    ReturnFun Rf;
}INPUTDWORD;

typedef struct __INPUTFLOAT
{
    char *Title;
    char *Spec;  // 单位
    char  Bits;  // 显示小数点后面几位
    float *Val;
    float Max;
    float Min;
    float Step;
    ReturnFun Rf;
}INPUTFLOAT;


typedef struct __FLOATEX
{
    BYTE  x;
    BYTE  y;
    char  Bp;  // 小数点前面的位数     
    char  Bn;  // 显示小数点后面的位数
    float *Val; // 返回值
    float Max;
    float Min;
    int  Bits[12];  // 浮点数每位值 6.6 
    ReturnFun Rf;
}FLOATEX;


// 扩展输入，带单位切换
typedef struct __INPUTEXT
{
    char  * Title;     
    BYTE  UnitCount;  // 单位个数
    BYTE  CurrUnit;   // 当前单位
    float *RetVal;    // 最终返回值
    float InputVal;   // 输入的值，没有单位
    char **UnitList;  // 单位列表
    ReturnFun Rf_Ok;
    ReturnFun Rf_Cancel;
}INPUTEXT;

void InputByte();
void InputByteKey(PRESSKEY key);
void InputWord();
void InputWordKey(PRESSKEY key);
void InputDword();
void InputDwordKey(PRESSKEY key);
void InputFloat();
void InputFloatKey(PRESSKEY key);
void ShowComfirm(char *Hint, ReturnFun Rf_Ok, ReturnFun Rf_Cancel);
void ComfirmKey(PRESSKEY key);
void HintKey(PRESSKEY key);
void ShowHint(char *Hint, ReturnFun Rf);
void PopHint(char *Hint);
void PopRect(BYTE x, BYTE y, BYTE w, BYTE h, char *Title);
void PopWindow(BYTE x, BYTE y, BYTE w, BYTE h, char *Title);
void InputFloatEx();
void FloatExKey(PRESSKEY key);
void InputFloatUnit();
void FloatUnitKey(PRESSKEY key);
void SelectUnit();
void SelectUnitKey(PRESSKEY key);
#endif