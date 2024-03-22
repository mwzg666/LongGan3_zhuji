#ifndef CALCCPS_H
#define CALCCPS_H

#ifdef __cplusplus
extern  "C" {
#endif

void EmtyBendi(void);

// ��ͨ��
//��ʼ��  
void InitCalcLow(void);
void EmtyCalcLowQ(void);
// ÿһ�뱻����һ��
float CalcLow(float RealCPS, float OlduSvh, float *NewuSvh);
// ÿ100ms����һ��
void FilterLow(unsigned long nNewCnt);

//High
//��ʼ��  
void InitCalcHigh(void);
void EmtyCalcHighQ(void);

// ÿһ�뱻����һ��
float CalcHigh(float RealCPS, float OlduSvh, float *NewuSvh);
// ÿ100ms����һ��
void FilterHigh(unsigned long nNewCnt);

#ifdef __cplusplus
}
#endif
#endif  