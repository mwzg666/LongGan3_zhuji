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

//���ݹ������
void InterDataManUI();
void DataManKey(PRESSKEY  dir);

//���ݴ洢���ý���
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
//���ݴ洢����
//void InterDataStoreUI();  
//void PromptInfo();
//void DataStoreKey(PRESSKEY  dir);

void ViewAllLogKey(PRESSKEY key);
void InterBrowseAllLog();
void RefreshLogView();

void InterDataDelUI();
void DataDelKey(PRESSKEY key);
    
//�鿴���ݲ˵�
void InterViewDataUI();
void ViewDataKey(PRESSKEY  dir);

//��ʱ��β�ѯ����
void InterByTimeViewUI();
void ByTimeViewKey(PRESSKEY  dir);

//��ʱ��β�ѯ����
void InterShowByTimeUI();
void ReadFlashAllDataByTime();
void MoveCursorByTime();
void ShowByTimeKey(PRESSKEY  dir);

//�鿴�������ݽ���
void InterBrowseAllData();
void ReadFlashAllData();
void MoveCursor();
void BrowseAllDataKey(PRESSKEY  dir);

//�鿴��ϸ���ݽ���
void InterLookDetail();
void LookDetailKey(PRESSKEY  dir);

//��ʱ��β鿴��ϸ���ݽ���
void InterLookDetailByTime();
void LookDetailByTimeKey(PRESSKEY  dir);

void SelectMeterTime();
void MeterTimeKey(PRESSKEY key);
void GetOneData(WORD loc, QStorageData *data);
#ifdef __cplusplus
}
#endif
#endif  