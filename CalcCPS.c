#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <msp430.h>
#include "system.h"
#include "CommDataDefine.h"

#include "Queue.h"
#include "string.h"


#define CPS_AVG_BUG_SIZE (120)
#define CPS_COEFFICIENT (1)

unsigned long lCPS = 0;
unsigned long lCP100MS = 0;



// 平滑buffer
static squeue stAlphaCPSAvgQ;

static S_BENDI gs_AlphaBendi;

//static void Filter(squlink FilterQ, squlink CalcQ, unsigned long nNewCnt);
static DataType SmoothCps(float RealCPS, squlink avgQ, float OlduSvh, S_BENDI *bendi, float *NewuSvh);



/******************************************************
Low过滤
*/
void InitCalcLow(void)
{
    InitQueue(&stAlphaCPSAvgQ, (CPS_AVG_BUG_SIZE-1));
    memset((void*)&gs_AlphaBendi,0,sizeof(S_BENDI));
}

void EmtyCalcLowQ(void)
{
    InitQueue(&stAlphaCPSAvgQ, (CPS_AVG_BUG_SIZE-1));
}

void EmtyBendi(void)
{
    memset((void*)&gs_AlphaBendi,0,sizeof(S_BENDI));
}


// 每一秒被调用一次,输入参数，返回CPS，带出剂量率
DataType CalcLow(float RealCPS, float OlduSvh, float *NewuSvh)
{
    DataType CPS = 0;
    CPS = SmoothCps(RealCPS, &stAlphaCPSAvgQ, OlduSvh, &gs_AlphaBendi, NewuSvh);
    return CPS;
}




//static float AllTimeCPS,AllTimeSec;//长时间累加的计数和时间
/******************************************************
平滑算法
//squlink Q 对象
*/

#define TURNINGUSV      		(10)//转折剂量率，usv
#define LOW_UPFAST      		(5)//低剂量变化倍数
#define HIGH_UPFAST     		(3)//高剂量变化倍数
#define LOW_UPDOWN_TIMES  		(2)//低剂量响应时间
#define HIGHT_UPDOWN_TIMES  	(1)//连续突变次数


DataType SmoothCps(float RealCPS, squlink avgQ, float OlduSvh, S_BENDI *bendi, float *NewuSvh)
{
    static DataType CPS = 0,ave=0;
    unsigned long Cnt;// = Lenqueue(Q);
    float uSvh,tempuSvh,realTimeCPSuSvh;
    float multiple;
	unsigned char updowntimes;


    if(OlduSvh < 0.5)
    {
        OlduSvh = 0.5;
    }
    bendi->OlduSvh6 = bendi->OlduSvh5;
    bendi->OlduSvh5 = bendi->OlduSvh4;
    bendi->OlduSvh4 = bendi->OlduSvh3;
    bendi->OlduSvh3 = bendi->OlduSvh2;
    bendi->OlduSvh2 = OlduSvh;
    //CPS = SumRear(Q, Cnt);
    CPS = RealCPS;//读取最新一个数据

    EnterQueue(avgQ, CPS);//先存入平均值数据队列中

    Cnt = Lenqueue(avgQ);

    ave = CPS;//SumRear(avgQ, 2)/2;

    uSvh = CpsToUsv_h(ave);
	if(uSvh > 100000)//10m以上用实时值
	{	
	  	*NewuSvh = uSvh;
		return ave; 
	}
	
	//判断是否陡升陡降
    tempuSvh = uSvh;
    if(tempuSvh < 0.5)
    {
        tempuSvh = 0.5;
    }
    if((tempuSvh < TURNINGUSV)&&(bendi->OlduSvh4 < TURNINGUSV))//10u以下5倍变化
    {
        multiple = LOW_UPFAST;
		updowntimes = LOW_UPDOWN_TIMES;
    }
    else
    {
        multiple = HIGH_UPFAST;
		updowntimes = HIGHT_UPDOWN_TIMES;
    }
    if((tempuSvh > multiple*bendi->OlduSvh4)||(tempuSvh * multiple < bendi->OlduSvh4))
    {
        bendi->updownTimes++;
        if(bendi->updownTimes >= 2)
        {
            bendi->updownTimes = 0;
            if(Cnt >=updowntimes)
            {
                SetQLength(avgQ, updowntimes);
                Cnt = updowntimes;
            }
            ave = SumRear(avgQ, Cnt)/Cnt;//前Cnt秒平均值
            uSvh = CpsToUsv_h(ave);
            *NewuSvh = uSvh;
			
            bendi->down30 = 0;
            bendi->startFlashFlag = 0;
            bendi->FlashTimes = 0;
            bendi->OlduSvh6 = uSvh;
            bendi->OlduSvh5 = uSvh;
            bendi->OlduSvh4 = uSvh;
            bendi->OlduSvh3 = uSvh;
            bendi->OlduSvh2 = uSvh;
			bendi->time = Cnt;
			bendi->cps = ave*Cnt;
            return ave;     
        }
    }
    else
    {
        bendi->updownTimes = 0;
    }

    if(Cnt >= 3)
    {
        ave = SumRear(avgQ, 3)/3;//前3秒平均值
        uSvh = CpsToUsv_h(ave);

        //3秒计数大于1m，并且波动大于上次平均的10%，用前3秒的数据
        if((uSvh > 1000.0)&&(abs((int)(100.0*(uSvh-bendi->OlduSvh3)/(bendi->OlduSvh3+1.0))) > 6))
        {
            SetQLength(avgQ, 3);
            *NewuSvh = uSvh;
			
			bendi->time = 3;
			bendi->cps = ave*3;

            bendi->OlduSvh6 = uSvh;
            bendi->OlduSvh5 = uSvh;
            bendi->OlduSvh4 = uSvh;
			bendi->OlduSvh3 = uSvh;
            bendi->OlduSvh2 = uSvh;
            return ave;
        }
    }

   
    if(Cnt >= 5)
    {
        realTimeCPSuSvh = CpsToUsv_h(CPS);
        if(realTimeCPSuSvh < 0.5)
        {
            realTimeCPSuSvh = 0.5;
        }
        if(((100.0*(realTimeCPSuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.01)) > 50)/*&&(realTimeCPSuSvh >= 1)*/)//上升
        {
            bendi->startFlashFlag = 1;
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }

        }
        else if(((100.0*(realTimeCPSuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.1)) > 28)&&(realTimeCPSuSvh >= 3))//上升
        {
            bendi->startFlashFlag = 1;
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }
        }
        else if(((100.0*(realTimeCPSuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.01)) > 20)&&(realTimeCPSuSvh  >= 8))//上升
        {
            bendi->startFlashFlag = 1;
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }
        }
        else if(((100.0*(bendi->OlduSvh5 - realTimeCPSuSvh)/(realTimeCPSuSvh+0.01)) > 50)/*&&(bendi->OlduSvh5 >= 1)*/)//下降
        {
            bendi->down30++;
            bendi->startFlashFlag = 1;
        }
        else if(((100.0*(bendi->OlduSvh5 - realTimeCPSuSvh)/(realTimeCPSuSvh+0.01)) > 28)&&(bendi->OlduSvh5 >= 3))//下降
        {
            bendi->startFlashFlag = 1;
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }
        }
        else if(((100.0*(bendi->OlduSvh5 - realTimeCPSuSvh)/(realTimeCPSuSvh+0.01)) > 20)&&(bendi->OlduSvh5 >= 8))//下降
        {
            bendi->startFlashFlag = 1;
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }
        }
        else
        {
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }
            
        }
        if(bendi->down30 >= 9)
        {
            bendi->down30 = 0;
            if(bendi->OlduSvh5 < 1.5)
			{
			  	bendi->startFlashFlag = 0;
				bendi->FlashTimes = 0;
				
				SetQLength(avgQ, 5);
				ave = SumRear(avgQ, 5)/5;
				uSvh = CpsToUsv_h(ave);
				*NewuSvh = uSvh;
				bendi->OlduSvh6 = uSvh;
				bendi->OlduSvh5 = uSvh;
				bendi->OlduSvh4 = uSvh;
				bendi->OlduSvh3 = uSvh;
				bendi->OlduSvh2 = uSvh;
				bendi->time = 5;
				bendi->cps = ave*5;
				return ave;
			}
        }
        
        if(bendi->startFlashFlag == 1)
        {
            bendi->FlashTimes++;
        }
        else
        {
            bendi->FlashTimes = 0;
        }
        if(bendi->FlashTimes >= 5)
        {
            ave = SumRear(avgQ, 5)/5;//前5秒平均值
            uSvh = CpsToUsv_h(ave);
			tempuSvh = uSvh;
			if(tempuSvh < 0.5)
			{
			  	tempuSvh = 0.5;
			}
			if((((100*(tempuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.1) > 35))&&(bendi->OlduSvh5 > 10))//10u
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.1) > 35))&&(bendi->OlduSvh5 > 10)))//波动大于上次平均的30%，用前5秒的数据
            {
                SetQLength(avgQ, 5);
                *NewuSvh = uSvh;
                bendi->down30 = 0;
                bendi->OlduSvh6 = uSvh;
                bendi->OlduSvh5 = uSvh;
                bendi->OlduSvh4 = uSvh;
                bendi->OlduSvh3 = uSvh;
                bendi->OlduSvh2 = uSvh;
				
				bendi->startFlashFlag = 0;
            	bendi->FlashTimes = 0;
				bendi->time = 5;
				bendi->cps = ave*5;
                return ave;
            }
			Cnt = Lenqueue(avgQ);
			if(Cnt >= 10)
			{
			  	bendi->startFlashFlag = 0;
            	bendi->FlashTimes = 0;
				ave = SumRear(avgQ, 10)/10;//前10秒平均值
            	uSvh = CpsToUsv_h(ave);
				tempuSvh = uSvh;
				if(tempuSvh < 0.5)
				{
					tempuSvh = 0.5;
				}
				if((((100*(tempuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.01) > 80)))//
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.01) > 80))))//波动大于上次平均的30%，用前5秒的数据
				{
					SetQLength(avgQ, 5);
					*NewuSvh = uSvh;
					bendi->down30 = 0;
					bendi->OlduSvh6 = uSvh;
					bendi->OlduSvh5 = uSvh;
					bendi->OlduSvh4 = uSvh;
					bendi->OlduSvh3 = uSvh;
					bendi->OlduSvh2 = uSvh;
					bendi->time = 5;
					bendi->cps = ave*5;
					return ave;
				}
				else if((((100*(tempuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.01) > 50))&&(bendi->OlduSvh5 > 1.5))//10u
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.01) > 50))&&(bendi->OlduSvh5 > 1.5)))//波动大于上次平均的30%，用前5秒的数据
				{
					SetQLength(avgQ, 5);
					*NewuSvh = uSvh;
					bendi->down30 = 0;
					bendi->OlduSvh6 = uSvh;
					bendi->OlduSvh5 = uSvh;
					bendi->OlduSvh4 = uSvh;
					bendi->OlduSvh3 = uSvh;
					bendi->OlduSvh2 = uSvh;
					bendi->time = 5;
					bendi->cps = ave*5;
					return ave;
				}
				else if((((100*(tempuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.01) > 35))&&(bendi->OlduSvh5 > 3))//10u
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.01) > 35))&&(bendi->OlduSvh5 > 3)))//波动大于上次平均的30%，用前5秒的数据
				{
					SetQLength(avgQ, 5);
					*NewuSvh = uSvh;
					bendi->down30 = 0;
					bendi->OlduSvh6 = uSvh;
					bendi->OlduSvh5 = uSvh;
					bendi->OlduSvh4 = uSvh;
					bendi->OlduSvh3 = uSvh;
					bendi->OlduSvh2 = uSvh;
					bendi->time = 5;
					bendi->cps = ave*5;
					return ave;
				}
				else if((((100*(tempuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.1) > 15))&&(bendi->OlduSvh5 > 100))//100u
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.1) > 15))&&(bendi->OlduSvh5 > 100)))//波动大于上次平均的30%，用前5秒的数据
				{
					SetQLength(avgQ, 5);
					*NewuSvh = uSvh;
					bendi->down30 = 0;
					bendi->OlduSvh6 = uSvh;
					bendi->OlduSvh5 = uSvh;
					bendi->OlduSvh4 = uSvh;
					bendi->OlduSvh3 = uSvh;
					bendi->OlduSvh2 = uSvh;
					bendi->time = 5;
					bendi->cps = ave*5;
					return ave;
				}
			}
        }
    }



    //前面都不满足则用实际所有数据的平均值
	Cnt = Lenqueue(avgQ);
	//连续3秒的计数都大于0.5，
	if((bendi->OlduSvh2 > 30)&&(bendi->OlduSvh3 > 30)&&(bendi->OlduSvh4 > 30))
	{
	  	if(Cnt > 60)
		{
		  	SetQLength(avgQ, 60);
			Cnt = 60;
		}
	  	ave = SumRear(avgQ, Cnt)/Cnt;//前Cnt秒平均值
		bendi->time += 1;
		bendi->cps += CPS;
	}
	else if((bendi->OlduSvh2 > 20)&&(bendi->OlduSvh3 > 20)&&(bendi->OlduSvh4 > 20))
	{
	  	if(Cnt > 90)
		{
		  	SetQLength(avgQ, 90);
			Cnt = 90;
		}
		ave = SumRear(avgQ, Cnt)/Cnt;//前Cnt秒平均值
		bendi->time += 1;
		bendi->cps += CPS;
	}
	else
	{
	  	ave = SumRear(avgQ, Cnt)/Cnt;//前Cnt秒平均值
		bendi->time += 1;
		bendi->cps += CPS;
	}
    /*else
	{
		//长时间累加值
		bendi->time += 1;
		bendi->cps += CPS;
		ave = bendi->cps/bendi->time;
	}*/
    uSvh = CpsToUsv_h(ave);
    *NewuSvh = uSvh;
	Cnt = Lenqueue(avgQ);
	if(Cnt > 15)
	{
	  	tempuSvh = SumRear(avgQ, 15)/15;
		uSvh = CpsToUsv_h(tempuSvh);
	  	if((uSvh + 0.5 < *NewuSvh)&&(*NewuSvh < 1.5))
		{
		  	*NewuSvh = uSvh;
			ave = tempuSvh;
			bendi->cps = tempuSvh*15;
			bendi->time = 15;
			SetQLength(avgQ, 15);
		}
	}
    return ave;
}


/******************************************************
过滤算法
*/
/*static void Filter(squlink FilterQ, squlink CalcQ, unsigned long nNewCnt)
{

	EnterQueue(CalcQ, nNewCnt);
	
#if 0
	
	static DataType nCnt1st = 0;
    static DataType nCnt2nd = 0;
    static DataType nCnt3rd = 0;
    static DataType nCnt4th = 0;
    
    // 进入三个数时
    if (Lenqueue(FilterQ) == (FILTER_BUF_SIZE -1))
    {
        // 0时，当作1计算
        nCnt1st = GetElement(FilterQ,1);
#if 0 // debug
        uart_send("A", 1);
        uart_send_cps(nCnt1st);
#endif
        if (nCnt1st <= 0)
        {
            nCnt1st = 1;
        }
        nCnt2nd = GetElement(FilterQ,2);
#if 0 // debug
        uart_send("B", 1);
        uart_send_cps(nCnt2nd);
#endif
        if (nCnt2nd <= 0)
        {
            nCnt2nd = 1;
        }
        nCnt3rd = GetElement(FilterQ,3);
#if 0 // debug
        uart_send("C", 1);
        uart_send_cps(nCnt3rd);
#endif
        if (nCnt3rd <= 0)
        {
            nCnt3rd = 1;
        }

        nCnt4th = nNewCnt;
        // 第四个计数，加入队列，并且把第一个有效的计数加入CPS队列中
        EnterQueue(FilterQ, nCnt4th);
        
        // 过滤抖动
        // 只有一个数陡增，过滤两个
        if ((nCnt2nd >= (nCnt1st * COEFFICIENT)) && (nCnt2nd >= (nCnt3rd * COEFFICIENT)))
        {
#if 0 // debug 
            uart_send("F ", 2);
            uart_send_cps(nCnt2nd);
            uart_send("F ", 2);
            uart_send_cps(nCnt3rd);
#endif 
            nCnt2nd = ((nCnt1st + nCnt4th) / 2);
            nCnt3rd = nCnt2nd;
            SetElement(FilterQ, 2, nCnt2nd);
            SetElement(FilterQ, 3, nCnt3rd);
        }
        // 抖动可能在200ms内，前面一个100ms和后面一个100ms之和（计数分布可能前大后小，等等）
        else if ((nCnt2nd >= (nCnt1st * COEFFICIENT)) && (nCnt3rd >= (nCnt1st * COEFFICIENT)) && 
                 (nCnt3rd >= (nCnt4th * COEFFICIENT) || nCnt2nd >= (nCnt4th * COEFFICIENT)))
        {
#if 0 // debug 
            uart_send("F ", 2);
            uart_send_cps(nCnt2nd);
            uart_send("F ", 2);
            uart_send_cps(nCnt3rd);
#endif
            nCnt2nd = ((nCnt1st + nCnt4th) / 2);
            nCnt3rd = nCnt2nd;
            SetElement(FilterQ, 2, nCnt2nd);
            SetElement(FilterQ, 3, nCnt3rd);
        }

        // FilterQ中第一个有效的计数加入CalcQ队列
        DeQueue(FilterQ, (DataType*)&nCnt1st);
        if (IsFullQueue(CalcQ))
        {
            // 队列已经满了，先出列
            DeQueue(CalcQ, (DataType*)&nCnt4th);
        }
        EnterQueue(CalcQ, nCnt1st);
    }
    // 前FILTER_BUF_SIZE -1计数，直接加入队列
    else
    {
        EnterQueue(FilterQ, nNewCnt);
        EnterQueue(CalcQ, nNewCnt);
    }
#endif
	
}*/
