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
BYTE KeyTimer = 0;  // 大于200ms才响应新的按键
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

    //按键
    P1DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6);//输入

    // 不使用中断 -- pgy
    //P1IES |= BIT0 + BIT2 + BIT3 + BIT4 + BIT5;   //下降沿
    //P1IES &= ~(BIT2 + BIT3);  //上升沿
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
    // 按键音-- 有报警时不开
    if (InnerParam.Keytone) 
    {
        if (InnerParam.Sound == 0)
        {   
            // 不开报警音要响
            StartBeep(200,0);
        }
        else
        {
            if (!HaveValidAlarm())
            {  
                // 没有报警要响
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
    
    

    if ((P1IN & BIT0) == 0)   // 电源键
    {
        if (PowerKey())  // 长按power 关机
        {
            // 按一下要返回
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
        // 两个按键之间要大于200ms
        return NONE;
    }
    #endif
    
    Key = P1IN & 0x7C;
    if (Key == 0x7C)   // 没有键按下直接返回
    {
        tm = 0;
        Key_his = 0x7C;
        return NONE;
    }

    if (Key == Key_his)
    {
        if (currUI == MEASU)
        {
            // 主界面不支持长按键处理
            return NONE;
        }
        
        // 长按键处理
        //#ifdef USE_IDLE
        //if (tm < 20) //200000)
        //#else
        if (tm < 200000)
        //#endif
        {
            tm ++;
            return NONE;   // 没有按键状态变化
        }

        delay_ms(50); // 长按每50ms +  10ms返回键码
    }

    delay_ms(10);
    if (Key == (P1IN & 0x7C))
    {
        Key_his = Key;

        for (i=2;i<7;i++)
        {
            if ((Key & (1<<i)) == 0)
            {
                if (tm == 0)  // 长按就不用响了
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

