#include <stdlib.h>
#include <string.h>
#include <msp430x552x.h>
#include "CommDataDefine.h"
#include "system.h"
#include "Main.h"
#include "Keyboard.h"
#include "oled.h"

#include "QuickMenu.h"
#include "Background.h"
#include "VerifPassword.h"
#include "MainMenu.h"
#include "OtherParam.h"
#include "DataStore.h"
#include "Measinter.h"
#include "RadioSet.h"
#include "flash.h"

extern QPara Param;
extern INTERFACE currUI;
extern QInnerParam InnerParam;

#ifdef DOUBLE_DELAY
BYTE KeyTimer = 0;  // ����200ms����Ӧ�µİ���
#endif

void KeyBoard_Init()
{
    //memset(Sm,0,sizeof(unsigned long)*nSmoothNum);  

    //P1.0  --  CPU_KEYINT    :  PUSH Button Int
    //P1.1  --  EXTP_DETECT  :  Battery check   
    
    //P1.2  --  CPU_BACK       :  Back key
    //P1.3  --  CPU_MODE      :  Mode key
    //P1.4  --  CPU_OK          :  Ok key
    //P1.5  --  CPU_DOWN     :  Down key
    //P1.6  --  CPU_UP           :  Up key

    //����
    P1DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6);//����

    // ��ʹ���ж� -- pgy
    //P1IES |= BIT0 + BIT2 + BIT3 + BIT4 + BIT5;   //�½���
    //P1IES &= ~(BIT2 + BIT3);  //������
    //P1IES |= BIT4 + BIT5 + BIT0 + BIT1;
    //P1IE |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5;

    
    
    //_EINT();
    

    #if 0
    for(int i = 0; i < 50; i++)
    {
        if((P1IN & BIT0) == 1)
        {
            P3DIR |= BIT6;
            P3OUT &= ~BIT6;//off
            exit(0);
        }   
        delay_ms(10);
    }
    #endif    

}


void KeySound()
{
    // ������-- �б���ʱ����
    if (InnerParam.Keytone) 
    {
        if (InnerParam.Sound == 0)
        {   
            // ����������Ҫ��
            StartBeep(200,0);
        }
        else
        {
            if (!HaveValidAlarm())
            {  
                // û�б���Ҫ��
                StartBeep(200,0);
            }
        }
    }
}

BOOL PowerKey()
{
    BYTE Cnt = 0;
    Cnt = 0;

    delay_ms(10);
    if ((P1IN & BIT0) != 0)
    {
        return FALSE;
    }
    
    while((P1IN & BIT0) == 0)
    {
        Sleep(100);
        if (Cnt++ > 10)
        {
            ShutDown(0);
        }
    }

    return TRUE;
}



PRESSKEY GetKey(void)
{
    BYTE Key,i;
    static BYTE Key_his = 0x7C;
    static DWORD tm = 0;
    
    

    if ((P1IN & BIT0) == 0)   // ��Դ��
    {
        if (PowerKey())  // ����power �ػ�
        {
            // ��һ��Ҫ����
            KeySound();

            #ifdef NEW_KEYBOARD
            return RETURN;
            #else
            return POWER;
            #endif
        }
    }

    #ifdef DOUBLE_DELAY
    if (KeyTimer < KEYDELAY)
    {
        // ��������֮��Ҫ����200ms
        return NONE;
    }
    #endif
    
    Key = P1IN & 0x7C;
    if (Key == 0x7C)   // û�м�����ֱ�ӷ���
    {
        tm = 0;
        Key_his = 0x7C;
        return NONE;
    }

    if (Key == Key_his)
    {
        if (currUI == MEASU)
        {
            // �����治֧�ֳ���������
            return NONE;
        }
        
        // ����������
        //#ifdef USE_IDLE
        //if (tm < 20) //200000)
        //#else
        if (tm < 200000)
        //#endif
        {
            tm ++;
            return NONE;   // û�а���״̬�仯
        }

        delay_ms(50); // ����ÿ50ms +  10ms���ؼ���
    }

    delay_ms(10);
    if (Key == (P1IN & 0x7C))
    {
        Key_his = Key;

        for (i=2;i<7;i++)
        {
            if ((Key & (1<<i)) == 0)
            {
                if (tm == 0)  // �����Ͳ�������
                {
                    KeySound();  
                    #ifdef DOUBLE_DELAY
                    KeyTimer = 0;
                    #endif
                }
                
                return (PRESSKEY)i;
            }
        }
    }

    return NONE;
}

