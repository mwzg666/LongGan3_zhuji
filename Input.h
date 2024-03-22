#ifndef __INPUT_H__
#define __INPUT_H__


typedef struct __INPUTBYTE
{
    char *Title;
    char *Spec;  // ��λ
    BYTE *Val;
    BYTE Max;
    BYTE Min;
    BYTE Step;
    ReturnFun Rf;
}INPUTBYTE;


typedef struct __INPUTWORD
{
    char *Title;
    char *Spec;  // ��λ
    WORD *Val;
    WORD Max;
    WORD Min;
    WORD Step;
    ReturnFun Rf;
}INPUTWORD;


typedef struct __INPUTDWORD
{
    char *Title;
    char *Spec;  // ��λ
    DWORD *Val;
    DWORD Max;
    DWORD Min;
    DWORD Step;
    ReturnFun Rf;
}INPUTDWORD;

typedef struct __INPUTFLOAT
{
    char *Title;
    char *Spec;  // ��λ
    char  Bits;  // ��ʾС������漸λ
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
    char  Bp;  // С����ǰ���λ��     
    char  Bn;  // ��ʾС��������λ��
    float *Val; // ����ֵ
    float Max;
    float Min;
    int  Bits[12];  // ������ÿλֵ 6.6 
    ReturnFun Rf;
}FLOATEX;


// ��չ���룬����λ�л�
typedef struct __INPUTEXT
{
    char  * Title;     
    BYTE  UnitCount;  // ��λ����
    BYTE  CurrUnit;   // ��ǰ��λ
    float *RetVal;    // ���շ���ֵ
    float InputVal;   // �����ֵ��û�е�λ
    char **UnitList;  // ��λ�б�
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