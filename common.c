#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "CommDataDefine.h"

extern QPara Param;
extern SENSER Senser;
extern QInnerParam InnerParam;
extern METEFACEINFO FaceInfo;


#if 0
tiny FloatToTiny(float val)
{
    tiny ret;
    ret.m = (WORD)val;
    ret.e = 0;
    return ret;
}

float TinyToFloat(tiny val)
{
    float ret;
    ret = (float)val.m;
    ret += pow(2,val.e);
    return ret;
}
#endif

// 内部探头: 根据计数时间计算CPS
DWORD CounterToCps(DWORD Cnt)
{
    BYTE t = (BYTE)(Param.InSenser.Param.Ct/100); 
    return (Cnt*10/t);
}

#if 0
float CpsToUsv_h(float count)
{
    
    #if 0
    return count;
    #else
    float DRreturn;
    float count2;
    float count3;
    unsigned char State;

    const float lowfactor0 = -0.1150896707;
    const float lowfactor1 = 10.48925651;
    const float lowfactor2 = -0.3584771495;
    const float lowfactor3 = 0.3935024371e-1;

    const float midfactor0 = 4.175559983;
    const float midfactor1 = 13.59064852;
    const float midfactor2 = 0.5482365533e-3;
    const float midfactor3 = -0.1990246458e-6;

    const float highfactor0 = 971.2739047;
    const float highfactor1 = 12.99644062;
    const float highfactor2 = 0.1530853217e-3;
    const float highfactor3 = 0.1215235629e-8;

    DRreturn = 0.0f;
    count2 = count * count;
    count3 = count2 * count;

    #if 1
    if(count >= 2540)
    {
        State=2;  //high
    }
    else if(count >= 8.6)
    {   
        State=1;    //mid
    }
    else
    {
        State=0;  //low
    }
    #else
    State=0;  //low
    #endif

    
    if (count >= 0.14) /* cps :0.068333333 对应剂量率是:0.885738234uSv/h */
    {
        switch(State)
        {
            case 0:  //低剂量     
            {
                #if 1      
                DRreturn = lowfactor0 
                           + count * lowfactor1 
                           + count2 * lowfactor2 
                           + count3 * lowfactor3;
                #else

                DRreturn =    -1.387933839 
                            + count * 21.46101642
                            + count2 * -7.696467625 
                            + count3 * 2.613823815
                            + count3 * count *-0.3595228660 
                            + count3 * count2 *0.1713606062e-1;
                #endif
                break;
            }


            case 1:
            {
                DRreturn =    midfactor0 
                            + count * midfactor1 
                            + count2 * midfactor2 
                            + count3 * midfactor3;                
                break;
            }
            
            case 2:
            {
                DRreturn =    highfactor0 
                            + count * highfactor1 
                            + count2 * highfactor2 
                            + count3 * highfactor3;
                break;
            }


        } 
        //DRreturn = DRreturn / 1000.0; // to mSv
    }
    else
    {
        
        if (count <= 0.03)
        {
            //DRreturn = 0.000099 + count * 0.001;  //mSv
            DRreturn = 0.099 + count * 0.001;  //uSv
        }
        else
        {
            //DRreturn = 0.000099 + count * 0.01131;  // mSv
            DRreturn = 0.099 + count * 0.01131;  // uSv
        }
    }

    if (DRreturn > 99999000000)
    {
        DRreturn = 99999000000;
    }

    // 保留两位小数，后面的数据直接舍弃，不四舍五入
    DRreturn *= Param.InSenser.Param.Cr;
    
    return (DRreturn <= 0 ? 0:DRreturn);
    
    #endif
}
#else

float CpsToUsv_h(float count)
{
    float ret;
    float a,b,c,d;
    if (InnerParam.DevType == DEV_MPRP_S2L)
    {
        // 单GM管剂量率到100mSv/h，重新分段拟合 -- 2023.9.18
        //ret = InnerParam.Yc * InnerParam.Ya * count * exp(InnerParam.Yb*count);
        if (count < 10)
        {
            a = -0.263076951469851;
            b = 0.945100305103219;
            c = -0.0960098581507102;
            d = 0.00622510519475919;
        }
        else if (count < 80)
        {
            a = 0.813655319616721;
            b = 0.51653025115826;
            c = 0.00189574473728847;
            d = -9.24109112566575E-07;
        }
        else if (count < 1000)
        {
            a = -28.9870075105939;
            b = 1.02150302224239;
            c = -0.000927287042957496;
            d = 6.56491217318852E-07;
        }
        else if (count < 7000)
        {
            a = -591.697048055268;
            b = 1.31638175470045;
            c = -8.23708836161717E-05;
            d = 4.14410642676932E-08;
        }
        else
        {
            a = 104818.833743884;
            b = 6.14796662883708;
            c = -0.00660061202494967;
            d = 5.69005615883038E-07;
        }
        
        ret = InnerParam.Yc*(a + b*count + c*count*count + d*count*count*count);
    }
    else
    {
        //if (FaceInfo.In_Rt > 10000.0)
        if (FaceInfo.Gm == 1)
        {
            ret = InnerParam.Hc * InnerParam.Ha * count * exp(InnerParam.Hb*count);
        }
        else
        {
            ret = InnerParam.Lc * InnerParam.La * count * exp(InnerParam.Lb*count);
        }
    }
    return ret;
}
#endif

float CpsToBq(float cps)
{
    return (float)cps;
}

float CpsToBq_cm2(float cps)
{
    //return (float)cps/(InnerParam.BqXs);
    // 2017.9.28  bp/cm2 不用再转换单位了，探头自己转换
    return (float)cps;
}


// CPS 转换成当前单位
float CpsUnit(float cps, BYTE unit)
{
    switch (unit)
    {
        case UNIT_CPS:     return (float)cps;
        case UNIT_Bq:      return CpsToBq(cps);
        case UNIT_Bq_cm2:  return CpsToBq_cm2(cps);
    }
    return (float)cps;
}

float ToCps(float val, BYTE unit)
{
    switch (unit)
    {
        case UNIT_CPS:     return val;
        case UNIT_Bq:      return val;
        //case UNIT_Bq_cm2:  return val*(InnerParam.BqXs);
        case UNIT_Bq_cm2:  return val;
    }
    
    return val;
}


