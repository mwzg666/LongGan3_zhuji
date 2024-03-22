#include <stdio.h>
#include <string.h>
#include <msp430x552x.h>
#include "system.h"

#include "DataStore.h"
#include "Keyboard.h"
#include "oled.h"
#include "CommDataDefine.h"
#include "MainMenu.h"
#include "QuickMenu.h"
#include "flash.h"
#include "Measinter.h"
#include "OtherParam.h"
#include "Main.h"
#include "Input.h"
#include "DS1337.h"
#include "Eeprom.h"

extern INTERFACE currUI;
extern QPara Param;
extern char * const NucName[];
extern DS1337_QTIME cuTime;
extern char IsSuperUser;
extern char * const LogName[];
extern char * const UnitStr[];
extern char StrTemp[24];
extern QInnerParam InnerParam;
extern CMPTIME CmpTime;

#ifdef DATA_IN_INNER_FALSH
extern __no_init unsigned char Data_Cache[512] @0x3E00;
#endif

QStorageData RunData;


char ActiveMode = 0;   // 1 Yes 0 No
char AlarmStore = 0;   // 1  Yes 0 No
char MeteTime = 1;   // 1 秒
char DataType = DATA_RT;
char TrigerType = TRG_ALL;
char DataManIndex = 1;

void ShowDataManUI()
{
    Clear_DispBuf();
    GT_Show16X16Char(16,0,"数据日志管理",0);
    
    GT_Show12X12Char(0,20,"测量数据",  0);
    //char strData[16] = "";
    memset(StrTemp,0,24);
    sprintf(StrTemp,":%d/%d",InnerParam.StorageNum,DATA_COUNT);
    Show5X7String(53,23,StrTemp,0);

    GT_Show12X12Char(0,34,"系统日志",  0);
    memset(StrTemp,0,24);
    sprintf(StrTemp,":%d/%d",InnerParam.LogNum,LOG_COUNT);
    Show5X7String(53,37,StrTemp,0);

    ShowLine(50);
    
	GT_Show12X12Char(20,55,"存储参数设置",DataManIndex==1);
    GT_Show12X12Char(20,70,"查看测量数据",DataManIndex==2);
	GT_Show12X12Char(20,85,"查看系统日志",DataManIndex==3);
	GT_Show12X12Char(20,100,"删除所有数据",DataManIndex==4);
    

    ShowOptHint();
    DisplayRefresh();
}

void InterDataManUI()
{
    #ifdef INIT_OPT_LOC
    DataManIndex = 1;
    #endif
    currUI = DATAMAN;
    ShowDataManUI();
}

void DataManKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterMainMenu(); break;
        
        case UP:
        {
            if(--DataManIndex < 1)
            {
                DataManIndex = 4;
            }
            ShowDataManUI();
        }
        break;
        
        case DOWN:
        {
            if(++DataManIndex > 4)
            {
                DataManIndex = 1;
            }
            ShowDataManUI();           
        }
        break;
        
        case OKKEY:
        {
            switch (DataManIndex)
            {
                case 1: InterDataSetUI(TRUE);   break;
                
                case 2: // 查看测量数据记录
                {
                    if (InnerParam.StorageNum == 0)
                    {
                        ShowHint("无数据!", InterDataManUI);
                    }
                    else
                    {
                        InterViewDataUI();
                    }
                    break;
                }

                case 3:    // 查看操作日志
                {
                    if (InnerParam.LogNum == 0)
                    {
                        ShowHint("无操作日志!", InterDataManUI);
                    }
                    else
                    {
                        InterBrowseAllLog();
                    }
                    break;
                }
                
                case 4:  
                {
                    if (IsSuperUser)
                    {
                        InterDataDelUI(); 
                    }
                    else
                    {
                        DelMeasuData();
                    }
                    break;
                }
                                
            }
            //DataManIndex = 1;
        }
        break;
    }
}


// 删除数据菜单
char DataDelIndex = 1;
void InterDataDelUI()
{
    currUI = DATADEL;
    PopWindow(18,40,90,48,"删除数据");
    
    GT_Show12X12Char(24,54,"删除测量数据",DataDelIndex==1);
    GT_Show12X12Char(24,70,"删除系统日志",DataDelIndex==2);
    
    DisplayRefresh();
}

void DelData()
{
    Erase_Data_Seg(BANK_D);
    InnerParam.StorageNum = 0;
    Param.ucStartBlock = 0;
    InnerParam.LastData = 0; 

    ShowHint("测量数据删除成功",InterDataManUI);
    StorageLog(EVENT_DEL_DATA);
    
}


void DelMeasuData()
{
    if (InnerParam.StorageNum == 0)
    {
        ShowHint("无测量数据",InterDataManUI);
        return;
    }

    ShowComfirm("确定要删除所有数据吗?",DelData,InterDataManUI);

}


void DelLog()
{
    //Erase_Data_Seg(BANK_C);
    #ifdef LOG_IN_INNER_FALSH
    DWORD i;
    for (i=0;i<LOG_COUNT/64;i++)
    {
        Erase_Block((BYTE *)(FLASH_LOG_START_ADDR+i*512));
    }
    #endif

    
    InnerParam.LogNum = 0;
    InnerParam.LastLog = 0;

    
    ShowHint("系统日志删除成功",InterDataManUI);
    //StorageLog(EVENT_DEL_LOG);
    
}

void DelSysLog()
{
    if (IsSuperUser == 0)
    {
        ShowHint("无权限",InterDataDelUI);
        return;
    }
    
    if (InnerParam.LogNum == 0)
    {
        ShowHint("无系统日志",InterDataDelUI);
        return;
    }

    ShowComfirm("确定要删除所有日志吗?",DelLog,InterDataManUI);
}


void DataDelKey(PRESSKEY key)
{
    switch (key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterDataManUI();  break;
        
        case OKKEY:
        {
            switch (DataDelIndex)
            {
                case 1: DelMeasuData(); break;
                case 2: DelSysLog();     break;
            }
            
            break;
        }

        case UP:
        {
            if(--DataDelIndex < 1)
            {
                DataDelIndex = 2;
            }
            InterDataDelUI();
        }
        break;
        
        case DOWN:
        {
            if(++DataDelIndex > 2)
            {
                DataDelIndex = 1;
            }
            InterDataDelUI();
        }
        break;
    }
}


//数据存储设置界面
char DataSetIndex = 1;
void InterDataSetUI(BOOL First)
{
    currUI = DATASET;

    if (First)
    {
        DataSetIndex = 1;

        AlarmStore = Param.AlarmStore;
        ActiveMode = Param.ActiveMode;
        MeteTime = Param.MeteTime;
        DataType = Param.DataType;
        TrigerType = Param.TrigerType;
    }
    
    RefeshDataSetUI();
}

void SaveDataParam()
{
    Param.AlarmStore = AlarmStore;
    Param.ActiveMode = ActiveMode;
    Param.MeteTime   = MeteTime;
    Param.DataType   = DataType;
    Param.TrigerType = TrigerType;
    
    PopHint("正在保存参数");
    WriteParamToFlash();
}

void RefeshDataSetUI()
{
    Clear_DispBuf();
    
    GT_Show16X16Char(16,0,"存储参数设置",0);
    
    GT_Show12X12Char(20,26,"报警存储",0);
    Show5X7String(74,29,"[   ]",0);
    
    if(AlarmStore == 1)
    {
        GT_Show12X12Char(80,26,"开",DataSetIndex == 1);

        // 增加报警触发选择
        Show12X12String(20,42,"报警类型:",0);
        switch (TrigerType)
        {
            case 1: Show12X12String(78,42,"内部",    DataSetIndex == 2); break;
            case 2: Show12X12String(78,42,"外部",    DataSetIndex == 2); break;
            case 3: Show12X12String(78,42,"所有",    DataSetIndex == 2); break;
        }

        
        GT_Show12X12Char(100,58,"保存",DataSetIndex == 3);

        ShowOptHint();
        DisplayRefresh();
        return;
    }
    
    GT_Show12X12Char(80,26,"关",DataSetIndex == 1);   
    
    GT_Show12X12Char(20,42,"自动存储",0);
    Show5X7String(74,45,"[   ]",0);
    if(ActiveMode == 0)
    {
        GT_Show12X12Char(80,42,"关",DataSetIndex == 2);
        GT_Show12X12Char(100,58,"保存",DataSetIndex == 3);

        ShowOptHint();
        DisplayRefresh();
        return;
    }
    
    GT_Show12X12Char(80,42,"开",DataSetIndex == 2);
    
        
    GT_Show12X12Char(20,58,"间隔时间",0);
    Show5X7String(74,61,"[   ]",0);
    
    switch (MeteTime)
    {
        case 1: Show5X7String(80,61,"10s", DataSetIndex == 3); break;
        case 2: Show5X7String(80,61,"30s", DataSetIndex == 3); break;
        case 3: Show5X7String(80,61,"1m", DataSetIndex == 3); break;
        case 4: Show5X7String(80,61,"10m", DataSetIndex == 3); break;
        case 5: Show5X7String(80,61,"30m", DataSetIndex == 3); break;
        case 6: Show5X7String(80,61,"1h", DataSetIndex == 3); break;
    }

    Show12X12String(20,74,"数据类型:",0);
    switch (DataType)
    {
        case 1: Show12X12String(78,74,"平均值",DataSetIndex == 4); break;
        case 2: Show12X12String(78,74,"最大值",DataSetIndex == 4); break;
        case 3: Show12X12String(78,74,"实时值",DataSetIndex == 4); break;
    }

    GT_Show12X12Char(100,90,"保存",DataSetIndex == 5);
              
 
    ShowOptHint();    
    DisplayRefresh();
}

void DataSetKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterDataManUI();  break;
        
        case UP:
        {
            if(AlarmStore == 1)  
            {
                if (--DataSetIndex < 1)
                {
                    DataSetIndex = 3;
                }
            }
            else
            {
                if(ActiveMode == 1)
                {
                    if (--DataSetIndex < 1)
                    {
                        DataSetIndex = 5;
                    }
                }
                else
                {
                    if (--DataSetIndex < 1)
                    {
                        DataSetIndex = 3;
                    }
                }
            }
           
            RefeshDataSetUI();
        }
        break;
        
        case DOWN:
        {
            if(AlarmStore == 1)  
            {
                if (++DataSetIndex > 3)
                {
                    DataSetIndex = 1;
                }
            }
            else
            {
                if(ActiveMode == 1)
                {
                    if (++DataSetIndex > 5)
                    {
                        DataSetIndex = 1;
                    }
                }
                else
                {
                    if (++DataSetIndex > 3)
                    {
                        DataSetIndex = 1;
                    }
                }
            }

            RefeshDataSetUI();
        }
        break;

        case MODE:
        {
            switch (DataSetIndex)
            {
                case 1:
                {
                    AlarmStore = !AlarmStore;
                    RefeshDataSetUI();
                    break;
                }
                
                case 2: 
                {
                    if (!AlarmStore)
                    {
                        ActiveMode =!ActiveMode;
                        RefeshDataSetUI();
                    }
                    
                    break;
                }
            }

            break;
        }

        case OKKEY:
        {
            switch (DataSetIndex)
            {
                case 2: 
                {
                    if (AlarmStore)
                    {
                        SelectTrigerType();
                        //SaveDataParam();
                        //ShowHint("参数保存成功",InterDataManUI);
                    }
                    break;
                }
                
                case 3: 
                {
                    if (AlarmStore)
                    {                        
                        SaveDataParam();
                        ShowHint("参数保存成功",InterDataManUI);
                    }
                    else
                    {
                        if (ActiveMode)
                        {
                            SelectMeterTime();
                        }
                        else
                        {
                            SaveDataParam();
                            ShowHint("参数保存成功",InterDataManUI);
                        }
                    }
                    break;
                }

                case 4:
                {
                    if (ActiveMode)
                    {
                        SelectDataType();
                    }
                    
                    break;
                }
                
                case 5: 
                {
                    SaveDataParam();
                    ShowHint("参数保存成功",InterDataManUI);
                    break;
                }
            }
        }
        break;
    }
}



BYTE MeterTimeIndex = 1;
void PopTimeWindow()
{
    PopWindow(64,40,30,64,"");
    Show5X7String(68,44,"10s", MeterTimeIndex == 1);
    Show5X7String(68,54,"30s", MeterTimeIndex == 2);
    Show5X7String(68,64,"1m",  MeterTimeIndex == 3);
    Show5X7String(68,74,"10m", MeterTimeIndex == 4);
    Show5X7String(68,84,"30m", MeterTimeIndex == 5);
    Show5X7String(68,94,"1h",  MeterTimeIndex == 6);
    DisplayRefresh();
}

void SelectMeterTime()
{
    currUI = METERTIME;
    
    MeterTimeIndex = MeteTime;

    PopTimeWindow();    
}

void MeterTimeKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterDataSetUI(FALSE);  break;   
        
        case OKKEY:  
        {
            MeteTime = MeterTimeIndex;  
            InterDataSetUI(FALSE);
            break;
        }

        case UP:
        {
            if(--MeterTimeIndex < 1)
            {
                MeterTimeIndex = 6;
            }
            PopTimeWindow();
        }
        break;
        
        case DOWN:
        {
            if(++MeterTimeIndex > 6)
            {
                MeterTimeIndex = 1;
            }
            PopTimeWindow();
        }
        break;
    }
}



// 数据类型
char MeterTypeIndex = 0;
void PopTypeWindow()
{
    PopWindow(64,50,48,52,"");
    Show12X12String(68,54,"平均值",  MeterTypeIndex == 1);
    Show12X12String(68,70,"最大值",  MeterTypeIndex == 2);
    Show12X12String(68,86,"实时值",  MeterTypeIndex == 3);
    
    DisplayRefresh();
}

void SelectDataType()
{
    currUI = METERTYPE;
    
    MeterTypeIndex = DataType;

    PopTypeWindow(); 
}

void SelectTypeKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterDataSetUI(FALSE);  break;   
        
        case OKKEY:  
        {
            DataType = MeterTypeIndex;  
            InterDataSetUI(FALSE);
            break;
        }

        case UP:
        {
            if(--MeterTypeIndex < 1)
            {
                MeterTypeIndex = 3;
            }
            PopTypeWindow();
        }
        break;
        
        case DOWN:
        {
            if(++MeterTypeIndex > 3)
            {
                MeterTypeIndex = 1;
            }
            PopTypeWindow();
        }
        break;
    }
}


// 报警触发类型
char TrigerTypeIndex = 0;
void PopTrigerWindow()
{
    PopWindow(44,50,74,52,"");
    Show12X12String(48,54,"内部探测器",  TrigerTypeIndex == 1);
    Show12X12String(48,70,"外部探测器",  TrigerTypeIndex == 2);
    Show12X12String(48,86,"所有报警",    TrigerTypeIndex == 3);
    
    DisplayRefresh();
}

void SelectTrigerType()
{
    currUI = TRIGERTYPE;
    
    TrigerTypeIndex = TrigerType;

    PopTrigerWindow(); 
}

void SelectTrigerKey(PRESSKEY key)
{
    switch(key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterDataSetUI(FALSE);  break;   
        
        case OKKEY:  
        {
            TrigerType = TrigerTypeIndex;  
            InterDataSetUI(FALSE);
            break;
        }

        case UP:
        {
            if(--TrigerTypeIndex < 1)
            {
                TrigerTypeIndex = 3;
            }
            PopTrigerWindow();
        }
        break;
        
        case DOWN:
        {
            if(++TrigerTypeIndex > 3)
            {
                TrigerTypeIndex = 1;
            }
            PopTrigerWindow();
        }
        break;
    }
}


#if 0
void DeleteLastData()
{
    if(Param.ucStartInTailBlock > 0)
    {
        unsigned char ucUserFullBlock = Param.StorageNum/32;
        unsigned int uiAddrOffset = (Param.ucStartBlock + ucUserFullBlock)*32;
        uiAddrOffset %= 1056;
        
        QStorageData data;
        for(int i = 0; i <= Param.ucStartInTailBlock; ++i)
        {
            Flash_Read((unsigned char *)(FLASH_DATA_START_ADDR+sizeof(QStorageData)*(uiAddrOffset+i)), (unsigned char*)&data, sizeof(QStorageData));
            ArrData[i] = data;
        }
        
        Erase_Data_Per_Seg((char *)FLASH_DATA_START_ADDR+sizeof(QStorageData)*uiAddrOffset);
        Param.ucStartInTailBlock--;
        for(int i = 0; i < Param.ucStartInTailBlock; ++i)
        {
            Write_Data_Seg((unsigned char *)(FLASH_DATA_START_ADDR+sizeof(QStorageData)*(uiAddrOffset+i)),(unsigned char *)&(ArrData[i]),sizeof(QStorageData));    
        }
        
        memset(ArrData,0,sizeof(ArrData));
        Param.StorageNum--;
        
    }
    else if(Param.ucStartInTailBlock == 0)
    {
        if(Param.StorageNum > 0)
        {
            unsigned char ucUserFullBlock = Param.StorageNum/32 - 1;
            unsigned int uiAddrOffset = (Param.ucStartBlock + ucUserFullBlock)*32;
            uiAddrOffset %= 1056;
            
            QStorageData data;
            for(int i = 0; i <= 32; ++i)
            {
                Flash_Read((unsigned char *)(FLASH_DATA_START_ADDR+sizeof(QStorageData)*(uiAddrOffset+i)), (unsigned char*)&data, sizeof(QStorageData));
                ArrData[i] = data;
            }
            
            Erase_Data_Per_Seg((char *)FLASH_DATA_START_ADDR+sizeof(QStorageData)*uiAddrOffset);
            Param.ucStartInTailBlock = 31;
            for(int i = 0; i < 31; ++i)
            {
                Write_Data_Seg((unsigned char *)(FLASH_DATA_START_ADDR+sizeof(QStorageData)*(uiAddrOffset+i)),(unsigned char *)&(ArrData[i]),sizeof(QStorageData));    
            }
            
            memset(ArrData,0,sizeof(ArrData));
            Param.StorageNum--;
        }
    }
}
#endif





//查看操作日志--->
#define LOGLINES 5
int LogPageIndex = 0;
int LogPageNum = 0;
SYSLOG Log;

void InterBrowseAllLog()
{
    currUI = VIEWALLLOG;
   
    LogPageNum = (InnerParam.LogNum%LOGLINES == 0)? InnerParam.LogNum/LOGLINES: InnerParam.LogNum/LOGLINES+1;
    
    LogPageIndex = 0;
    
    RefreshLogView();
}

void ViewAllLogKey(PRESSKEY key)
{
    switch (key)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterDataManUI(); break;

        case UP:  // 翻页
        {
            if (--LogPageIndex < 0)
            {
                LogPageIndex = LogPageNum - 1;
            }
            RefreshLogView();
            break;
        }

        case DOWN:  // 翻页
        {
            
            if (++LogPageIndex >= LogPageNum)
            {
                LogPageIndex = 0;
            }
            
            RefreshLogView();
            break;
        }
    }
}




void ShowOneLog(BYTE x, BYTE y)
{
    //char showDta[24] = "";
    memset(StrTemp,0,24);
    sprintf(StrTemp,"%02d-%02d-%02d %02d:%02d:%02d",Log.year, Log.month,Log.day,Log.hour,Log.minute,Log.second);
    Show5X7String(x,y,StrTemp,0);
    if (Log.LogId < 100)
    {
        Show12X12String(x,y+8,LogName[Log.LogId],0);
    }
    else
    {
        memset(StrTemp,0,24);
        sprintf(StrTemp,"ErrCode:%d",Log.LogId);
        Show12X12String(x,y+8,StrTemp,0);
    }
}

void RefreshLogView()
{
    BYTE line = 0;
    //char str[20] = {0};
    memset(StrTemp,0,24);

    Clear_DispBuf();
    GT_Show12X12Char(40,0,"系统日志",0);
    
    for(WORD i = LOGLINES*LogPageIndex;(i<LOGLINES*LogPageIndex+LOGLINES)&&(i<InnerParam.LogNum); ++i)
    {
        memset(&Log,0,sizeof(SYSLOG));

        #ifdef LOG_IN_INNER_FALSH
        DWORD Offset = FLASH_LOG_START_ADDR + i*sizeof(SYSLOG);
        Flash_Read((unsigned char *)Offset, (unsigned char*)&Log, sizeof(SYSLOG));
        #else
        DWORD Offset = LOG_OFFSET + i*sizeof(SYSLOG);
        _DINT();
        BYTE *Data = (unsigned char *)&Log; 
        #if 0
        for (int i=0;i<sizeof(SYSLOG);i++)
        {
            Eeprom_ReadByte(Offset+i,(Data+i));
        }
        #else
        Eeprom_ReadBuff(Offset,Data,sizeof(SYSLOG));
        #endif
        _EINT();
        
        #endif
        if(Log.ucCheck != GetVerfity((unsigned char*)&Log,sizeof(SYSLOG)))
        {
            continue;
        }
        
        ShowOneLog(0,14+line*21);
        line++;
    }


    sprintf(StrTemp,"PAGE:%d/%d", LogPageIndex+1,LogPageNum);
    Show5X7String(0,120,StrTemp,0);

    //MoveLogCursor();
    DisplayRefresh();
}



//查看所有记录的数据---->
const BYTE LINES = 13;  // 每页显示的行数
int ViewStoreIndex = 0;
int PageIndex = 0;  // 

void InterBrowseAllData()
{
    currUI = VIEWSTORE;
    Clear_DispBuf();
    GT_Show12X12Char(0,0,"时间",0);
    GT_Show12X12Char(70,0,"值",0);
    GT_Show12X12Char(96,0,"单位",0);

    ReadFlashAllData();
    MoveCursor();
    DisplayRefresh();
}

void GetOneData(WORD loc, QStorageData *data)
{
    memset(data,0,sizeof(QStorageData));     
    
    #ifdef DATA_IN_INNER_FALSH
    DWORD Offset;
    if (InnerParam.StorageNum < DATA_COUNT)
    {
        Offset = FLASH_DATA_START_ADDR + loc*sizeof(QStorageData);
        Flash_Read((unsigned char *)Offset, (unsigned char*)data, sizeof(QStorageData));
    }
    else
    {
        if ((loc < InnerParam.LastData /32 *32) || (loc >= InnerParam.LastData))
        {
            Offset = FLASH_DATA_START_ADDR + loc*sizeof(QStorageData);
            Flash_Read((unsigned char *)Offset, (unsigned char*)data, sizeof(QStorageData));
        }
        else
        {
            // 在内存里面
            Offset = (loc%32)*sizeof(QStorageData);   
            memcpy(data,&Data_Cache[Offset],sizeof(QStorageData));
        }
    }
    #else
    WORD Offset;
    Offset = DATA_OFFSET + loc*sizeof(QStorageData);
    Eeprom_ReadBuff(Offset, (unsigned char*)data, sizeof(QStorageData));
    #endif
}

void ShowDataList(BYTE Line, QStorageData *data)
{
    float temp;
    //CMPTIME ct;

    if(data->ucCheck != GetVerfity((unsigned char*)data,sizeof(QStorageData)))
    {
        return;
    }
    
    memset(&CmpTime,0,sizeof(CMPTIME));
    memset(StrTemp,0,24);
    CmpTime.Time = data->Time;
    temp = GetVal1(data);
    
    if ((data->Channel == SENSER_IN) ||
        (data->Channel == SENSER_Y)  ||
        (data->Channel == SENSER_LP) ||
        (data->Channel == SENSER_GM) ||
        (data->Channel == SENSER_PD)
        )
    {
        if (temp >= 1000)
        {
            sprintf(StrTemp,"%02d-%02d %02d:%02d %0.0f",CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t,temp);
        }
        else if (temp >= 100)
        {
            sprintf(StrTemp,"%02d-%02d %02d:%02d %0.1f",CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t,temp);
        }
        else
        {
            sprintf(StrTemp,"%02d-%02d %02d:%02d %0.2f",CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t,temp);
        }
    }
    else
    {
        if (GetOutUnit1(data) == UNIT_CPS)
        {
            sprintf(StrTemp,"%02d-%02d %02d:%02d %0.0f",CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t,temp);
        }
        else
        {
            if (temp >= 1000)
            {
                sprintf(StrTemp,"%02d-%02d %02d:%02d %0.0f",CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t,temp);
            }
            else if (temp >= 100)
            {
                sprintf(StrTemp,"%02d-%02d %02d:%02d %0.1f",CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t,temp);
            }
            else
            {
                sprintf(StrTemp,"%02d-%02d %02d:%02d %0.2f",CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t,temp);
            }
        }
    }
    //Show5X7String(0,13+(i-LINES*PageIndex)*8,StrTemp,0);
    Show5X7String(0,13+Line*8,StrTemp,0);
    
    if ((data->Channel == SENSER_IN) ||
        (data->Channel == SENSER_Y) ||
        (data->Channel == SENSER_LP) ||
        (data->Channel == SENSER_GM) ||
        (data->Channel == SENSER_PD) )
    {
        //Show5X7String(101,13+(i-LINES*PageIndex)*8,UnitStr[GetInUnit1(data)],0);
        Show5X7String(101,13+Line*8,UnitStr[GetInUnit1(data)],0);
    }
    else
    {
        //Show5X7String(101,13+(i-LINES*PageIndex)*8,UnitStr[GetOutUnit1(data)],0);
        Show5X7String(101,13+Line*8,UnitStr[GetOutUnit1(data)],0);
    }
}

void ReadFlashAllData()
{
    //static QStorageData data;
    WORD i,j,si,ei;
    BYTE line;
    
    memset(&RunData,0,sizeof(QStorageData));
    
    Clear(0,13,128,115);

    line = 0;
    
    if (InnerParam.StorageNum < DATA_COUNT)
    {
        si = LINES*PageIndex;
        ei = InnerParam.StorageNum;  
    }
    else
    {
        si = LINES*PageIndex + InnerParam.LastData;
        ei = InnerParam.StorageNum + InnerParam.LastData;  
    }
    
    for (i=si; (line<LINES)&&(i<ei); ++i)
    {        
        j = i%DATA_COUNT;
        
        GetOneData(j,&RunData);
        ShowDataList(line++, &RunData);
    }
}

void MoveCursor()
{
    const unsigned char Sign[] = {0x00,0x04,0x1C,0x7C,0x1C,0x04,0x00,0x00};
    //char str[20] = {0};
    memset(StrTemp,0,24);
    Clear(120,13,8,115);
    if(InnerParam.StorageNum > 0)
    {
        Diplay(120,13+(8*ViewStoreIndex),8,8,Sign,0);
    }
    sprintf(StrTemp,"%d/%d(%d/%d)",
            PageIndex+1,
            (InnerParam.StorageNum%LINES ==0)? InnerParam.StorageNum/LINES:InnerParam.StorageNum/LINES+1,
            LINES*PageIndex+ViewStoreIndex+1,
            InnerParam.StorageNum);
    
    Clear(0,120,128,8);
    Show5X7String(0,120,StrTemp,0);
    DisplayRefresh();
}

void BrowseAllDataKey(PRESSKEY  dir)
{
    int Max;

    Max = 0;
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterDataManUI();  break;

        case MODE: // 模式键翻页
        {
            PageIndex++;
            if((ViewStoreIndex + 1 + PageIndex*LINES) >= InnerParam.StorageNum)
            {
                PageIndex = 0;
                ViewStoreIndex = 0;
                ReadFlashAllData();
            }
            else
            {
                ReadFlashAllData();
            }

            MoveCursor();
            break;
        }

        case DOWN:
        {
            //下
            //while(DOWNKEY == 0)
            {
                //delay_ms(100);
                if((ViewStoreIndex + 1 + PageIndex*LINES) >= InnerParam.StorageNum)
                {
                    // 如果当前是最后一条数据者从0开始
                    PageIndex = 0;
                    ViewStoreIndex = 0;
                    ReadFlashAllData();
                    MoveCursor();
                    break;
                }
              
                Max = (LINES < (InnerParam.StorageNum-LINES*PageIndex)) ? 
                       LINES : (InnerParam.StorageNum-LINES*PageIndex);
                if(++ViewStoreIndex >= Max)
                {
                    ViewStoreIndex = 0;
                    if(Max == LINES)
                    {
                        PageIndex++;
                        ReadFlashAllData();
                    }
                }
                MoveCursor();
            }
        }
        break;
        
        case UP:
        {
            //上
            //while(UPKEY == 0)
            {
                //delay_ms(100);
                if(--ViewStoreIndex < 0)
                {
                    ViewStoreIndex = 0;
                    if(--PageIndex < 0)
                    {   // 显示最后一页
                        PageIndex = (InnerParam.StorageNum%LINES==0)?InnerParam.StorageNum/LINES-1:InnerParam.StorageNum/LINES;
                        ViewStoreIndex = (InnerParam.StorageNum%LINES==0)?LINES-1:InnerParam.StorageNum%LINES-1;
                    }
                    else
                    {
                        ViewStoreIndex = LINES-1;
                    }
                    ReadFlashAllData();
                }
                MoveCursor();
            }
        }
        break;
        
        case OKKEY:
        {
            //OK键
            if(InnerParam.StorageNum == 0)
            {
                break;
            }
            InterLookDetail();
        }
        break;
    }
}

void ShowDataDetail(QStorageData *data)
{
    if ((data->Channel == SENSER_IN) ||
        (data->Channel == SENSER_Y)  ||
        (data->Channel == SENSER_LP) ||
        (data->Channel == SENSER_GM) ||
        (data->Channel == SENSER_PD)
        )
    {
        Show12X12String(0,0,"实时剂量:",0);
        ShowSenser(82,3,data->Channel);
        
        
        //阈值
        //char Threshold[8] = "";
        //sprintf((char*)StrTemp,"%d%s",data->AlarmValue,
        //         UnitStr[data->Unit]);    
        //Show5X7String(64,14,StrTemp,0);
        
        ShowCounter(0,24,"",GetVal1(data),GetInUnit1(data));

        if ((data->Channel != SENSER_GM) &&
             (data->Channel != SENSER_PD) )
        {
            // PD和GM 没有累计剂量
            Show12X12String(0,80,"累计剂量:",0);
            ShowCounterSm(8,94,"",GetVal2(data),GetInUnit2(data));
        }
    }
    else if (data->Channel == SENSER_LC)
    {
        Show12X12String(0,0,"外部探测器:",0);
        ShowSenser(82,3,data->Channel);
        //Show5X7String(0,14,NucName[data->NucName],0);  //核素名称

        #if 0
        //阈值
        memset(StrTemp,0,24);
        sprintf((char*)StrTemp,"%d%s",data->AlarmValue,
                 UnitStr[GetOutUnit0(data)]);    
        Show5X7String(64,14,StrTemp,0);
        #endif
        
        ShowCounterMid(0,16,"@A",GetVal1(data),0);
        ShowCounterMid(0,48,"@B@Y",GetVal2(data),GetOutUnit1(data));
    }
    else
    {
        Show12X12String(0,0,"外部探测器:",0);
        ShowSenser(82,3,data->Channel);
        Show5X7String(0,14,NucName[data->NucName],0);  //核素名称
        
        //阈值
        memset(StrTemp,0,24);
        sprintf((char*)StrTemp,"%d%s",data->AlarmValue,
                 UnitStr[GetOutUnit0(data)]);    
        Show5X7String(64,14,StrTemp,0);
        
        ShowCounter(0,24,"",GetVal1(data),GetOutUnit1(data));


        Show12X12String(0,80,"内部探测器:",0);
        ShowCounterSm(8,94,"",GetVal2(data),GetOutUnit2(data));
    }   

    //时间
    //CMPTIME ct;
    memset(&CmpTime,0,sizeof(CMPTIME));
    memset(StrTemp,0,24);
    CmpTime.Time = data->Time;
    sprintf(StrTemp,"20%02d-%02d-%02d %02d:%02d:%02d",
                      CmpTime.y, CmpTime.m, CmpTime.d, CmpTime.h, CmpTime.t,CmpTime.s);
    Show5X7String(0,110,StrTemp,0);
}



//查看详细数据界面
void InterLookDetail()
{    
    if(PageIndex*LINES + ViewStoreIndex < 0)
    {
        PageIndex = (InnerParam.StorageNum%LINES==0)?InnerParam.StorageNum/LINES-1:InnerParam.StorageNum/LINES;
        ViewStoreIndex = (InnerParam.StorageNum%LINES == 0)?LINES-1:InnerParam.StorageNum%LINES-1;
        //return;
    }
    
    currUI = DETAIL;
    Clear_DispBuf();


    //static DWORD Offset;
    WORD loc;
    //static QStorageData data;

    memset(&RunData,0,sizeof(QStorageData));

    if (InnerParam.StorageNum == DATA_COUNT)
    {
        loc = (LINES*PageIndex+ViewStoreIndex+InnerParam.LastData)%DATA_COUNT;
    }
    else
    {
        loc = (LINES*PageIndex+ViewStoreIndex)%DATA_COUNT;
    }
    
    GetOneData(loc,&RunData);
    ShowDataDetail(&RunData);
    
    
    // 位置
    memset(StrTemp,0,24);
    sprintf(StrTemp,"(%d/%d)", PageIndex*LINES+ViewStoreIndex+1, InnerParam.StorageNum);
    Show5X7String(0,120,StrTemp,0);
        
    DisplayRefresh();
}

void LookDetailKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterBrowseAllData(); break;
            
        case UP:
        {
            if(--ViewStoreIndex < 0)
            {
                PageIndex--;
                ViewStoreIndex = LINES-1;
            }
            InterLookDetail();
        }
        break;
        
        case DOWN:
        {
            if((ViewStoreIndex + 1 + PageIndex*LINES) >= InnerParam.StorageNum)
            {
                PageIndex = 0;
                ViewStoreIndex = 0;
                InterLookDetail();
                break;
            }
            
            if(++ViewStoreIndex >= LINES)
            {
                PageIndex++;
                ViewStoreIndex = 0;
            }
            InterLookDetail();
        }
        break;
        
        
        case OKKEY:
        {
            
        }
        break;
    }
}



// 查询数据
char ViewDataIndex = 1;

void RefreshViewDataUI()
{
    PopWindow(18,40,90,48,"数据查询");
    
    GT_Show12X12Char(24,54,"查看所有记录",ViewDataIndex==1);
	GT_Show12X12Char(24,70,"按时间段查询",ViewDataIndex==2);
    
    DisplayRefresh();
}

void InterViewDataUI()
{
    currUI = VIEWDATA;
    ViewDataIndex = 1;

    
    RefreshViewDataUI();
}

void ViewDataKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:  InterDataManUI();  break;
        
        case UP:
        {
            if(--ViewDataIndex < 1)
            {
                ViewDataIndex = 2;
            }
            RefreshViewDataUI();
        }
        break;
        
        case DOWN:
        {
            if(++ViewDataIndex > 2)
            {
                ViewDataIndex = 1;
            }
            RefreshViewDataUI();
        }
        break;
        
        case OKKEY:
        {
            //ViewDataIndex = 1;
            if(ViewDataIndex == 1)
            {
                //查看所有数据
                ViewStoreIndex = 0;
                PageIndex = 0;
                InterBrowseAllData();
            }
            else if(ViewDataIndex == 2)
            {
                //按时间段查看
                InterByTimeViewUI();
            }
        }
        break;
        
    }
}




//按时间段查询数据--->
int begYear = 2015;
char begMonth = 6;
char begDay = 8;
int begHour = 10;
int begMint = 41;

int endYear = 2015;
char endMonth = 6;
char endDay = 8;
int endHour = 10;
int endMint = 42;

int nStartPos = -1;   // 按时间查询结果的开始位置
int nEndpos = -1;     // 结束位置
int DataCountByTime = 0;  // 查询出来的数据数量

int ViewStoreByTimeIndex = 0;
int PageByTimeIndex = 0;
char ByTimeViewIndex = 1;


// 计算前一天
void SetPreDay()
{
    const BYTE days[] = {31,28,31,30,31,30,31,31,30,31,30,31};  
    if (cuTime.day > 1)
    {
        begYear = cuTime.year;
        begMonth = cuTime.month;
        begDay =  cuTime.day-1;
    }
    else  // 跨月份
    {   
        if (cuTime.month > 1)
        {
            begYear = cuTime.year;
            begMonth = cuTime.month-1;
            begDay =  days[begMonth-1];
            if ((begYear % 4) && (begMonth == 2))// 润年二月多一天
            {
                begDay++;
            }
        }
        else  // 跨年
        {
            begYear = cuTime.year-1;
            begMonth = 12;
            begDay = 31;
        }
    }
}

void InitQueryTime()
{
    // 默认查询前一天的数据
    GetTimer(&cuTime);
    SetPreDay();
    begHour = cuTime.hour;
    begMint = cuTime.minute;

    endYear = cuTime.year;
    endMonth = cuTime.month;
    endDay = cuTime.day;
    endHour = cuTime.hour;
    endMint = cuTime.minute;
}

void RefreshTimeViewUI()
{
    Clear_DispBuf();

    memset(StrTemp,0,24);

    GT_Show16X16Char(0,0,"请输入查询时间",0);
    
    const unsigned char sig[] = {0x00,0x10,0x10,0x38,0x38,0x54,0x10,0x00};
    
    GT_Show12X12Char(0,26,"开始时间",0);
 
    sprintf(StrTemp,"20%02d-%02d-%02d %02d:%02d",begYear%100,begMonth,begDay,begHour,begMint);
    Show5X7String(5,42,StrTemp,0);

    GT_Show12X12Char(0,62,"结束时间",0);
    sprintf(StrTemp,"20%02d-%02d-%02d %02d:%02d",endYear%100,endMonth,endDay,endHour,endMint);
    Show5X7String(5,78,StrTemp,0);
    GT_Show12X12Char(100,98,"确定",ByTimeViewIndex == 11);
    if(ByTimeViewIndex <= 5)
    {
        Diplay(8+(ByTimeViewIndex*16),50,8,8,sig,0);
    }
    else if(ByTimeViewIndex <= 10)
    {
        Diplay(8+((ByTimeViewIndex-5)*16),86,8,8,sig,0);
    }
    
    DisplayRefresh();
}

void InterByTimeViewUI()
{
    currUI = BYTIMEVIEW;

    InitQueryTime();
    
    RefreshTimeViewUI();
}

void QueryByTime()
{
    static char TimeStrt[14] = "";
    static char TimeEnd[14] = "";
    static char TimeRead[14] = "";
    //CMPTIME ct;
    int i;

    memset(TimeStrt,0,14);
    memset(TimeEnd,0,14);
    memset(TimeRead,0,14);
    memset(&CmpTime,0,sizeof(CMPTIME));
    
    sprintf(TimeStrt,"%02d%02d%02d%02d%02d",begYear%100,begMonth,begDay,begHour,begMint);    
    sprintf(TimeEnd,"%02d%02d%02d%02d%02d",endYear%100,endMonth,endDay,endHour,endMint);
    
    
    //Clear_DispBuf();
    if(strncmp(TimeStrt,TimeEnd,10) >= 0)
    {

        ShowHint("时间设置错误",InterByTimeViewUI);
        return;
    }
    else
    {
        //GT_Show12X12Char(20,20,"查询中请等待",0);
        //DisplayRefresh();
        PopHint("查询中请等待");
    }
    
    nStartPos = -1;
    nEndpos = -1;
    
    
    //static DWORD Offset;
    int bi,ei;

    if (InnerParam.StorageNum < DATA_COUNT)
    {
        bi = 0;
        ei = InnerParam.StorageNum;
    }
    else
    {
        bi = InnerParam.LastData;
        ei = InnerParam.StorageNum+InnerParam.LastData;
    }
    
    
    for(i=bi; i<ei; ++i)
    {
        SystemTask();  // 数据多了时间较长，系统任务不能停

        
        GetOneData(i%DATA_COUNT,&RunData);
        if(RunData.ucCheck != GetVerfity((unsigned char*)&RunData,sizeof(RunData)))
        {
            continue;
        }
        
        memset(TimeRead,0,14);
        CmpTime.Time = RunData.Time;
        sprintf(TimeRead,"%02d%02d%02d%02d%02d",CmpTime.y,CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t);
        if(strncmp(TimeRead,TimeStrt,10) >= 0)
        {
            nStartPos = i;
            break;
        }
    }

    if (nStartPos == -1)
    {
        ShowHint("无数据",InterByTimeViewUI);
        return;
    }

    
    for(i = nStartPos+1; i < ei; ++i)
    {
        SystemTask();  // 数据多了时间较长，系统任务不能停

        GetOneData(i%DATA_COUNT, &RunData);
        if(RunData.ucCheck != GetVerfity((unsigned char*)&RunData,sizeof(RunData)))
        {
            continue;
        }
        
        CmpTime.Time = RunData.Time;
        memset(TimeRead,0,14);
        sprintf(TimeRead,"%02d%02d%02d%02d%02d",CmpTime.y,CmpTime.m,CmpTime.d,CmpTime.h,CmpTime.t);
        
        if(strncmp(TimeRead,TimeEnd,10) <= 0)
        {
            nEndpos = i;
        }
        else
        {
            break;
        }
    }

    

    if ((nStartPos == -1) || (nEndpos == -1) || (nEndpos < nStartPos))
    {
        ShowHint("无数据",InterByTimeViewUI);
        return;
    }

    DataCountByTime = nEndpos - nStartPos + 1;

    ViewStoreByTimeIndex = 0;
    PageByTimeIndex = 0;
    InterShowByTimeUI();
}

void ByTimeViewKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        case RETURN:
        {
            ByTimeViewIndex = 1;
            InterDataManUI();
        }
        break;
        
        case UP:
        {
            
            {
                //delay_ms(100);
                if(ByTimeViewIndex == 1)
                {
                    if(++begYear > 2099)
                    {
                        begYear = 2015;
                    }
                }
                else if(ByTimeViewIndex == 2)
                {
                    if(++begMonth > 12)
                    {
                        begMonth = 1;
                    }
                }
                else if(ByTimeViewIndex == 3)
                {
                    if(++begDay > 31)
                    {
                        begDay = 1;
                    }
                }
                else if(ByTimeViewIndex == 4)
                {
                    if(++begHour > 23)
                    {
                        begHour = 0;
                    }
                }
                else if(ByTimeViewIndex == 5)
                {
                    if(++begMint > 59)
                    {
                        begMint = 0;
                    }
                }
                else if(ByTimeViewIndex == 6)
                {
                    if(++endYear > 2099)
                    {
                        endYear = 2015;
                    }
                }
                else if(ByTimeViewIndex == 7)
                {
                    if(++endMonth > 12)
                    {
                        endMonth = 1;
                    }
                }
                else if(ByTimeViewIndex == 8)
                {
                    if(++endDay > 31)
                    {
                        endDay = 1;
                    }
                }
                else if(ByTimeViewIndex == 9)
                {
                    if(++endHour > 23)
                    {
                        endHour = 0;
                    }
                }
                else if(ByTimeViewIndex == 10)
                {
                    if(++endMint > 59)
                    {
                        endMint = 0;
                    }
                }
                RefreshTimeViewUI();
            }
            
        }
        break;
        
        case DOWN:
        {
            
            {
                //delay_ms(100);
                if(ByTimeViewIndex == 1)
                {
                    if(--begYear < 2015)
                    {
                        begYear = 2099;
                    }
                }
                else if(ByTimeViewIndex == 2)
                {
                    if(--begMonth < 1)
                    {
                        begMonth = 12;
                    }
                }
                else if(ByTimeViewIndex == 3)
                {
                    if(--begDay < 1)
                    {
                        begDay = 31;
                    }
                }
                else if(ByTimeViewIndex == 4)
                {
                    if(--begHour < 0)
                    {
                        begHour = 23;
                    }
                }
                else if(ByTimeViewIndex == 5)
                {
                    if(--begMint < 0)
                    {
                        begMint = 59;
                    }
                }
                else if(ByTimeViewIndex == 6)
                {
                    if(--endYear < 2015)
                    {
                        endYear = 2099;
                    }
                }
                else if(ByTimeViewIndex == 7)
                {
                    if(--endMonth < 1)
                    {
                        endMonth = 12;
                    }
                }
                else if(ByTimeViewIndex == 8)
                {
                    if(--endDay < 1)
                    {
                        endDay = 31;
                    }
                }
                else if(ByTimeViewIndex == 9)
                {
                    if(--endHour < 0)
                    {
                        endHour = 23;
                    }
                }
                else if(ByTimeViewIndex == 10)
                {
                    if(--endMint < 0)
                    {
                        endMint = 59;
                    }
                }
                RefreshTimeViewUI();
            }
        }
        break;
        
        case OKKEY:
        {
            if(ByTimeViewIndex == 11)  // 确定
            {
                ByTimeViewIndex = 1;
                
                QueryByTime();
                return;
            }
            
            if(++ByTimeViewIndex > 11)
            {
                ByTimeViewIndex = 1;
            }
            RefreshTimeViewUI();
        }
        break;
        
        
    }
}



//按时间段查询数据
char ShowByTime = 1;
void InterShowByTimeUI()
{
    currUI = SHOWBYTIME;
    Clear_DispBuf();
    
    GT_Show12X12Char(0,0,"时间",0);
    GT_Show12X12Char(70,0,"值",0);
    GT_Show12X12Char(98,0,"单位",0);
    
    ReadFlashAllDataByTime();
    MoveCursorByTime();
    DisplayRefresh();
    
}

void ShowByTimeKey(PRESSKEY  dir)
{
    int Max = 0;
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        
        case RETURN:
        {
            //返回查看数据记录菜单
            InterByTimeViewUI();
        }
        break;
        
        case DOWN:
        {
            
            {
                
                //下
                if((ViewStoreByTimeIndex + 1 + PageByTimeIndex*LINES) >= DataCountByTime)
                {
                    //break;
                    PageByTimeIndex = 0;
                    ViewStoreByTimeIndex = 0;
                    ReadFlashAllDataByTime();
                    MoveCursorByTime();
                    break;
                }
              
                Max = (LINES < DataCountByTime-LINES*PageByTimeIndex) ? LINES-1 : DataCountByTime - LINES*PageByTimeIndex - 1;
                if(++ViewStoreByTimeIndex > Max)
                {
                    ViewStoreByTimeIndex = 0;
                    if(Max == LINES-1)
                    {
                        PageByTimeIndex++;
                        ReadFlashAllDataByTime();
                        
                    }
                }

                MoveCursorByTime();
            }
        }
        break;
        
        case UP:
        {
            
            {
                
                //上
                if(--ViewStoreByTimeIndex < 0)
                {
                    ViewStoreByTimeIndex = 0;
                    if(--PageByTimeIndex < 0)
                    {
                        PageByTimeIndex = (DataCountByTime%LINES==0)?DataCountByTime/LINES-1 : DataCountByTime/LINES;
                        ViewStoreByTimeIndex = (DataCountByTime%LINES==0)?LINES-1 : DataCountByTime%LINES-1;
                        
                    }
                    else
                    {
                        ViewStoreByTimeIndex = LINES-1;
                        //ReadFlashAllDataByTime();
                        //MoveCursorByTime();
                    }
                    ReadFlashAllDataByTime();
                }
                
                MoveCursorByTime();
            }
            
        }
        break;
        
        case OKKEY:
        {
            //OK键
            if(InnerParam.StorageNum == 0)
            {
                break;
            }
            InterLookDetailByTime();
        }
        break;
    }
}


void ReadFlashAllDataByTime()
{   
    //static QStorageData data;
    //static CMPTIME ct;
    
    int i;
    
    //memset(&ct,0,sizeof(ct));
    Clear(0,13,128,115);
    
    
    int loc;
    
    for(i = LINES*PageByTimeIndex;(i<LINES*PageByTimeIndex+LINES)&&(i<DataCountByTime); ++i)
    {
        loc = (i+nStartPos)%DATA_COUNT;

        GetOneData(loc,&RunData);
        ShowDataList(i-LINES*PageByTimeIndex,&RunData);
    }
}

void MoveCursorByTime()
{
    const unsigned char Sign[] = {0x00,0x04,0x1C,0x7C,0x1C,0x04,0x00,0x00};
    
    
    Clear(120,13,8,115);
    if(DataCountByTime > 0)
    {
        Diplay(120,13+(8*ViewStoreByTimeIndex),8,8,Sign,0);
    }

    memset(StrTemp,0,24);
    sprintf(StrTemp,"%d/%d(%d/%d)",
            PageByTimeIndex+1,(DataCountByTime%LINES ==0)? DataCountByTime/LINES : DataCountByTime/LINES+1,
            LINES*PageByTimeIndex+ViewStoreByTimeIndex+1,DataCountByTime);

    Clear(0,120,128,8);
    Show5X7String(0,120,StrTemp,0);
    DisplayRefresh();
}



void InterLookDetailByTime()
{
    if(PageByTimeIndex*LINES + ViewStoreByTimeIndex < 0)
    {
        PageByTimeIndex = (DataCountByTime%LINES==0)?DataCountByTime/LINES-1 : DataCountByTime/LINES;
        ViewStoreByTimeIndex = (DataCountByTime%LINES==0)?LINES-1 : DataCountByTime%LINES-1;
        //return;
    }
    
    currUI = DETAILBYTIME;

    Clear_DispBuf();
    
    
    //static QStorageData data;
    int loc;
    memset(&RunData,0,sizeof(QStorageData));
    loc = (LINES*PageByTimeIndex+ViewStoreByTimeIndex+nStartPos)%DATA_COUNT;
    
    GetOneData(loc,&RunData);
    
    ShowDataDetail(&RunData);

    // 位置
    memset(StrTemp,0,24);
    sprintf(StrTemp,"(%d/%d)", PageByTimeIndex*LINES+ViewStoreByTimeIndex+1, DataCountByTime);
    Show5X7String(0,120,StrTemp,0);

    
    DisplayRefresh();
}

void LookDetailByTimeKey(PRESSKEY  dir)
{
    switch(dir)
    {
        case TIMEOUT: case POWER: EnterMeasuInter(); break;
        
        case UP:
        {
            if(--ViewStoreByTimeIndex < 0)
            {
                PageByTimeIndex--;
                ViewStoreByTimeIndex = LINES-1;
            }
            InterLookDetailByTime();
        }
        break;
        
        case DOWN:
        {
            if((ViewStoreByTimeIndex + 1 + PageByTimeIndex*LINES) >= DataCountByTime)
            {
                PageByTimeIndex = 0;
                ViewStoreByTimeIndex = 0;
                InterLookDetailByTime();
                break;
            }
            
            if(++ViewStoreByTimeIndex >= LINES)
            {
                PageByTimeIndex++;
                ViewStoreByTimeIndex = 0;
                //InterLookDetailByTime();
                //break;
            }

            InterLookDetailByTime();
        }
        break;
        
        case RETURN:
        {
            InterShowByTimeUI();
        }
        break;
        
        case OKKEY:
        {
            
        }
        break;
    }
}

