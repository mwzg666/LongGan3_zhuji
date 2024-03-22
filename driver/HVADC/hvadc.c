#include "CommDataDefine.h"
#include "system.h"
#include "msp430x552x.h"
#include "oled.h"
#include "hvadc.h"
#include "mcp4725.h"

//#define SAMPLE_NUM 5//采样平均次数
#define AD_VPP 3300//参考电源为系统电源，系统电源变化需要修改该值,AD的参考电压只是作为AD本身使用，不涉及计算

#define VOL_RATION 1651//分压比,根据电阻计算出来

/*
函数名称:Get_HV_Val()
函数功能:获取高压电压
输入:无
输出:高压电压，单位为mV
作者:毕成军
日期:2014.8.22
*/

/*
unsigned long Get_HV_Val()
{
	unsigned char i;
	unsigned int adc_buf[SAMPLE_NUM],value;
	long sum_adc=0,result=0;
    unsigned int tmpvol;

	P6SEL |= BIT7;// Enable A/D channel inputs
	REFCTL0 &= ~REFMSTR;   //需要使用内部参考，必须用改值
	
	ADC12CTL0 = ADC12ON+ADC12MSC+ADC12SHT0_2+ADC12REFON+ADC12REF2_5V; // Turn on ADC12, set sampling time
	
	ADC12CTL1 = ADC12SHP+ADC12CONSEQ_2;       // Use sampling timer, single sequence
	ADC12MCTL0 |= ADC12INCH_7+ADC12EOS;                 // ref+=AVcc, channel = P6.7
	
    delay_ms(10);//delay for ref stable
	ADC12CTL0 |= ADC12ENC;                    // Enable conversions
	ADC12CTL0 |= ADC12SC;			   // Start convn - software trigger
	
    tmpvol = ADC12MEM0;
    
	for(i=0;i<SAMPLE_NUM;i++)
	{
	  	delay_ms(10);
		adc_buf[i] = ADC12MEM0;   //Get Result AD valude,单通道切换IO口不需要修改此处
	}

	for(i=0;i<SAMPLE_NUM;i++)
	{
		sum_adc+= adc_buf[i];   //Get Result AD valude
	}
	 
	ADC12CTL0 &= ~ADC12ENC;

	value=(unsigned int)(sum_adc/SAMPLE_NUM);
	result=((long)value*AD_VPP/4096)*VOL_RATION;
	return result/1000;
	
}
*/


/*

函数名称:Out_HV_Val(unsigned int voltage)
函数功能:设置高压电压
输入:高压值
输出:高压电压，单位为mV
作者:毕成军
日期:2014.10.24
函数内部可能有较长延时，调用的时候要注意
*/

#if 0
void Out_HV_Val(unsigned int voltage)
{
    unsigned int uiClassVol = 900;  // from 400V  400*2.44 = 976;   --- 2.44 -- 61/25
    
	if(voltage>1000) 
	{
		return;
	}
	else if((voltage<=1000) && (voltage>=400))
	{
        // 电压逐步升高
		do
        {
            MCP4725_OutVol(MCP4725_HV_ADDR,uiClassVol);
            uiClassVol += 100;
            Sleep(100);
        }while(uiClassVol < voltage*61/25);
        
        MCP4725_OutVol(MCP4725_HV_ADDR,voltage*61/25);
	}
    else
    {
        MCP4725_OutVol(MCP4725_HV_ADDR,voltage*61/25);
    }
}


void HV_Read_Test()
{
	//unsigned long adc_val;
        
	//adc_val = Get_HV_Val();
	
	//delay_us(10);
}
#endif
