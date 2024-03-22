#ifndef DATASTORE_H
#define DATASTORE_H

#include "CommDataDefine.h"

#ifdef __cplusplus
extern  "C" {
#endif


typedef struct 
{
    float OutA_Cps_Av;
    float OutA_Cur_Av;
    float OutA_Cps_Max;
    float OutA_Cur_Max;

    float OutB_Cps_Av;
    float OutB_Cur_Av;
    float OutB_Cps_Max;
    float OutB_Cur_Max;

    float In_Av;
    float In_Max;
}DATAINFO;

//数据管理界面
void InterDataManUI();
void DataManKey(PRESSKEY  dir);

//数据存储设置界面
void InterDataSetUI(BOOL First);
void RefeshDataSetUI();
void DataSetKey(PRESSKEY  dir);
void DeleteLastData();
void SelectDataType();
void SelectTypeKey(PRESSKEY key);
void SelectTrigerType();
void SelectTrigerKey(PRESSKEY key);
void DelMeasuData();
void DelData();
void DelLog();
//数据存储界面
//void InterDataStoreUI();  
//void PromptInfo();
//void DataStoreKey(PRESSKEY  dir);

void ViewAllLogKey(PRESSKEY key);
void InterBrowseAllLog();
void RefreshLogView();

void InterDataDelUI();
void DataDelKey(PRESSKEY key);
    
//查看数据菜单
void InterViewDataUI();
void ViewDataKey(PRESSKEY  dir);

//按时间段查询数据
void InterByTimeViewUI();
void ByTimeViewKey(PRESSKEY  dir);

//按时间段查询数据
void InterShowByTimeUI();
void ReadFlashAllDataByTime();
void MoveCursorByTime();
void ShowByTimeKey(PRESSKEY  dir);

//查看所有数据界面
void InterBrowseAllData();
void ReadFlashAllData();
void MoveCursor();
void BrowseAllDataKey(PRESSKEY  dir);

//查看详细数据界面
void InterLookDetail();
void LookDetailKey(PRESSKEY  dir);

//按时间段查看详细数据界面
void InterLookDetailByTime();
void LookDetailByTimeKey(PRESSKEY  dir);

void SelectMeterTime();
void MeterTimeKey(PRESSKEY key);
void GetOneData(WORD loc, QStorageData *data);
#ifdef __cplusplus
}
#endif
#endif  