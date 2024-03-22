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



// ƽ��buffer
static squeue stAlphaCPSAvgQ;

static S_BENDI gs_AlphaBendi;

//static void Filter(squlink FilterQ, squlink CalcQ, unsigned long nNewCnt);
static DataType SmoothCps(float RealCPS, squlink avgQ, float OlduSvh, S_BENDI *bendi, float *NewuSvh);



/******************************************************
Low����
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


// ÿһ�뱻����һ��,�������������CPS������������
DataType CalcLow(float RealCPS, float OlduSvh, float *NewuSvh)
{
    DataType CPS = 0;
    CPS = SmoothCps(RealCPS, &stAlphaCPSAvgQ, OlduSvh, &gs_AlphaBendi, NewuSvh);
    return CPS;
}




//static float AllTimeCPS,AllTimeSec;//��ʱ���ۼӵļ�����ʱ��
/******************************************************
ƽ���㷨
//squlink Q ����
*/

#define TURNINGUSV      		(10)//ת�ۼ����ʣ�usv
#define LOW_UPFAST      		(5)//�ͼ����仯����
#define HIGH_UPFAST     		(3)//�߼����仯����
#define LOW_UPDOWN_TIMES  		(2)//�ͼ�����Ӧʱ��
#define HIGHT_UPDOWN_TIMES  	(1)//����ͻ�����


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
    CPS = RealCPS;//��ȡ����һ������

    EnterQueue(avgQ, CPS);//�ȴ���ƽ��ֵ���ݶ�����

    Cnt = Lenqueue(avgQ);

    ave = CPS;//SumRear(avgQ, 2)/2;

    uSvh = CpsToUsv_h(ave);
	if(uSvh > 100000)//10m������ʵʱֵ
	{	
	  	*NewuSvh = uSvh;
		return ave; 
	}
	
	//�ж��Ƿ�������
    tempuSvh = uSvh;
    if(tempuSvh < 0.5)
    {
        tempuSvh = 0.5;
    }
    if((tempuSvh < TURNINGUSV)&&(bendi->OlduSvh4 < TURNINGUSV))//10u����5���仯
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
            ave = SumRear(avgQ, Cnt)/Cnt;//ǰCnt��ƽ��ֵ
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
        ave = SumRear(avgQ, 3)/3;//ǰ3��ƽ��ֵ
        uSvh = CpsToUsv_h(ave);

        //3���������1m�����Ҳ��������ϴ�ƽ����10%����ǰ3�������
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
        if(((100.0*(realTimeCPSuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.01)) > 50)/*&&(realTimeCPSuSvh >= 1)*/)//����
        {
            bendi->startFlashFlag = 1;
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }

        }
        else if(((100.0*(realTimeCPSuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.1)) > 28)&&(realTimeCPSuSvh >= 3))//����
        {
            bendi->startFlashFlag = 1;
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }
        }
        else if(((100.0*(realTimeCPSuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.01)) > 20)&&(realTimeCPSuSvh  >= 8))//����
        {
            bendi->startFlashFlag = 1;
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }
        }
        else if(((100.0*(bendi->OlduSvh5 - realTimeCPSuSvh)/(realTimeCPSuSvh+0.01)) > 50)/*&&(bendi->OlduSvh5 >= 1)*/)//�½�
        {
            bendi->down30++;
            bendi->startFlashFlag = 1;
        }
        else if(((100.0*(bendi->OlduSvh5 - realTimeCPSuSvh)/(realTimeCPSuSvh+0.01)) > 28)&&(bendi->OlduSvh5 >= 3))//�½�
        {
            bendi->startFlashFlag = 1;
            if(bendi->down30 > 0)
            {
                bendi->down30--;
            }
        }
        else if(((100.0*(bendi->OlduSvh5 - realTimeCPSuSvh)/(realTimeCPSuSvh+0.01)) > 20)&&(bendi->OlduSvh5 >= 8))//�½�
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
            ave = SumRear(avgQ, 5)/5;//ǰ5��ƽ��ֵ
            uSvh = CpsToUsv_h(ave);
			tempuSvh = uSvh;
			if(tempuSvh < 0.5)
			{
			  	tempuSvh = 0.5;
			}
			if((((100*(tempuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.1) > 35))&&(bendi->OlduSvh5 > 10))//10u
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.1) > 35))&&(bendi->OlduSvh5 > 10)))//���������ϴ�ƽ����30%����ǰ5�������
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
				ave = SumRear(avgQ, 10)/10;//ǰ10��ƽ��ֵ
            	uSvh = CpsToUsv_h(ave);
				tempuSvh = uSvh;
				if(tempuSvh < 0.5)
				{
					tempuSvh = 0.5;
				}
				if((((100*(tempuSvh-bendi->OlduSvh5)/(bendi->OlduSvh5+0.01) > 80)))//
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.01) > 80))))//���������ϴ�ƽ����30%����ǰ5�������
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
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.01) > 50))&&(bendi->OlduSvh5 > 1.5)))//���������ϴ�ƽ����30%����ǰ5�������
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
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.01) > 35))&&(bendi->OlduSvh5 > 3)))//���������ϴ�ƽ����30%����ǰ5�������
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
                ||(((100*(bendi->OlduSvh5 - tempuSvh)/(bendi->OlduSvh5+0.1) > 15))&&(bendi->OlduSvh5 > 100)))//���������ϴ�ƽ����30%����ǰ5�������
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



    //ǰ�涼����������ʵ���������ݵ�ƽ��ֵ
	Cnt = Lenqueue(avgQ);
	//����3��ļ���������0.5��
	if((bendi->OlduSvh2 > 30)&&(bendi->OlduSvh3 > 30)&&(bendi->OlduSvh4 > 30))
	{
	  	if(Cnt > 60)
		{
		  	SetQLength(avgQ, 60);
			Cnt = 60;
		}
	  	ave = SumRear(avgQ, Cnt)/Cnt;//ǰCnt��ƽ��ֵ
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
		ave = SumRear(avgQ, Cnt)/Cnt;//ǰCnt��ƽ��ֵ
		bendi->time += 1;
		bendi->cps += CPS;
	}
	else
	{
	  	ave = SumRear(avgQ, Cnt)/Cnt;//ǰCnt��ƽ��ֵ
		bendi->time += 1;
		bendi->cps += CPS;
	}
    /*else
	{
		//��ʱ���ۼ�ֵ
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
�����㷨
*/
/*static void Filter(squlink FilterQ, squlink CalcQ, unsigned long nNewCnt)
{

	EnterQueue(CalcQ, nNewCnt);
	
#if 0
	
	static DataType nCnt1st = 0;
    static DataType nCnt2nd = 0;
    static DataType nCnt3rd = 0;
    static DataType nCnt4th = 0;
    
    // ����������ʱ
    if (Lenqueue(FilterQ) == (FILTER_BUF_SIZE -1))
    {
        // 0ʱ������1����
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
        // ���ĸ�������������У����Ұѵ�һ����Ч�ļ�������CPS������
        EnterQueue(FilterQ, nCnt4th);
        
        // ���˶���
        // ֻ��һ������������������
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
        // ����������200ms�ڣ�ǰ��һ��100ms�ͺ���һ��100ms֮�ͣ������ֲ�����ǰ���С���ȵȣ�
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

        // FilterQ�е�һ����Ч�ļ�������CalcQ����
        DeQueue(FilterQ, (DataType*)&nCnt1st);
        if (IsFullQueue(CalcQ))
        {
            // �����Ѿ����ˣ��ȳ���
            DeQueue(CalcQ, (DataType*)&nCnt4th);
        }
        EnterQueue(CalcQ, nCnt1st);
    }
    // ǰFILTER_BUF_SIZE -1������ֱ�Ӽ������
    else
    {
        EnterQueue(FilterQ, nNewCnt);
        EnterQueue(CalcQ, nNewCnt);
    }
#endif
	
}*/
