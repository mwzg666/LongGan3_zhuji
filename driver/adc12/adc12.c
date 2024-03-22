#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "msp430x552x.h"
#include "CommDataDefine.h"
#include "system.h"
#include "adc12.h"



#define SAMPLE_NUM 20 //采样平均次数
#define REMOVE_CNT 200

unsigned int bat_result[SAMPLE_NUM];
unsigned int hv_result; //[SAMPLE_NUM];
WORD index=0;



#define BAT_AD_VPP 3000   //参考电源为系统电源，系统电源变化需要修改该值,AD的参考电压只是作为AD本身使用，不涉及计算
#define BAT_VOL_RATION 6  //分压比

#define HV_AD_VPP 3300//参考电源为系统电源，系统电源变化需要修改该值,AD的参考电压只是作为AD本身使用，不涉及计算
#define HV_VOL_RATION 1651//分压比,根据电阻计算出来

extern char StrTemp[24];

void Init_ADC12()
{
    P7DIR &= ~(BIT2);
    P7SEL |= BIT2;

    //REFCTL0 &= ~REFMSTR;   //需要使用内部参考，必须用该值
    //REFCTL0 |= REFON | REFVSEL1 | REFVSEL0 | REFTCOFF | REFOUT;
    ADC12CTL2 = ADC12REFOUT | ADC12RES_2;
    //ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT0_2; // Turn on ADC12, set sampling time
    ADC12CTL0 = ADC12ON | ADC12MSC | ADC12SHT0_2 | ADC12REFON | ADC12REF2_5V;
    //ADC12CTL0 = ADC12ON | ADC12SHT0_2 | ADC12REFON | ADC12REF2_5V;
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_3;       // Use sampling timer, single sequence
    //ADC12MCTL0 = ADC12INCH_10;
    ADC12MCTL0 = ADC12INCH_14 | ADC12EOS;                 // ref+=AVcc, channel = A0
    //ADC12MCTL1 = ADC12SREF_1 | ADC12INCH_10 | ADC12EOS;        // ref+=AVcc, channel = A1, end seq.
    ADC12IE = 0x01;                           // Enable ADC12IFG.3
    ADC12CTL0 |= ADC12ENC;                    // Enable conversion
    ADC12CTL0 |= ADC12SC;
    _EINT();
}

void ADC12_Start()
{
    ADC12CTL0 |= ADC12ENC;
    ADC12CTL0 |= ADC12SC;

    index = 0;
}

float GetVlotValu()
{
    //#define DEBUG_BAT//测试
    #define K_BAT 0.00531
    #define B_BAT -0.78836
    
    unsigned char i;
    float value,result;
    
    float temp = 0;

    for(i=0;i<SAMPLE_NUM;i++)
    {
        temp += bat_result[i];   //Get Result AD valude
    }

    #if 0
    value=(unsigned int)(temp/SAMPLE_NUM);
    temp = (float)value*BAT_AD_VPP*BAT_VOL_RATION;
    result=(WORD)(temp/4095);
    #else
    //result = (unsigned int)(temp * 50 / 242);
    value = temp/SAMPLE_NUM;

    // debug result = (value - 234)/202;
    //result = (value - 785)/125;  // ---- my
    //result =  (value - 82)/218; 
    //------------ result =  (value - 40)/223;    // 老版本分压电阻
    result =  K_BAT*value + B_BAT;    // 老版本分压电阻
    //result =  (value - 16.4)/205; 

	//result = ((float)gtestADValue-40)/207.7;//yao测试++++
    #endif

    #ifdef DEBUG_BAT
    memset(StrTemp,0,24);
    sprintf(StrTemp,"%.0f:%.2fV",value,result);
    Clear(0,10,70,8);
    Show5X7String(0,10,StrTemp,0);
    #endif

    return result;
}

unsigned long Get_HV_Vol()
{
    #if 0
    unsigned char i;
    unsigned int value;
    unsigned long sum_adc=0,result1=0;

    for(i=0;i<SAMPLE_NUM;i++)
    {
        sum_adc+= hv_result[i];   //Get Result AD valude
    }

    value=(unsigned int)(sum_adc/SAMPLE_NUM);
    #if 0
    Show5X7Int(80,30,(unsigned int)value,0);
    #endif
    result1=((long)value*HV_AD_VPP/4096)*HV_VOL_RATION;
    #if 0
    Show5X7Int(100,10,(unsigned int)(result1/1000),0);
    #endif
    return result1/1000;

    #else
    return 800;
    #endif
    
}

#if 0
void Test_ADC12()
{
  delay_ms(10000);
  volatile unsigned int bat_vol=0,tmp1=0;
  volatile unsigned long hv_vol=0,tmp2=0;
  //Init_ADC12();
  delay_ms(1000);
  bat_vol = Get_Bat_Vol();
  hv_vol = Get_HV_Vol();
  tmp1 = bat_vol;
  tmp2 = hv_vol;
  _NOP();
}
#endif


#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR (void)
{
    //_BIC_SR_IRQ(LPM3_bits);   // 唤醒
    //switch(__even_in_range(ADC12IV,34))
    //{
    //case  8:                            // Vector  8:  ADC12IFG1

    
    if (index < REMOVE_CNT)
    {
        // 去掉前面n次转换数据
        hv_result = ADC12MEM0;                 // Move results, IFG is cleared
        index++;
    }
    else 
    {
        bat_result[index-REMOVE_CNT] = ADC12MEM0;                 // Move results, IFG is cleared

        index++;
        if(index >= SAMPLE_NUM+REMOVE_CNT)
        {
            index = 0;
            ADC12CTL0 &= ~ADC12ENC;  // 停止ADC
        }
    }

    

    //  __bic_SR_register_on_exit(LPM4_bits);   // Exit active CPU, SET BREAKPOINT HERE  
    //  break;
    //default: break; 
    //}
}