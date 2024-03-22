#ifndef ADC12_H_
#define ADC12_H_

#include "oled.h"

void Init_ADC12();

float GetVlotValu();

unsigned long Get_HV_Vol();
void ADC12_Start();
void Test_ADC12();
#endif
