#ifndef __WIFI_H__
#define __WIFI_H__

#ifdef USE_WIFI

#define WIFI_STATION   1
#define WIFI_AP         2


typedef struct __WIFIPARAM
{
    BOOL Enable;
    BYTE Mode;         // 1: Client  2: Ap
    BYTE SSID[32];
    BYTE PassWord[32];
    BYTE SrvIp[4];
    WORD Port;
    
}WIFIPARAM;


BOOL InitWifi();

void Wifi_IoInit();
void Wifi_Reset();
void Wifi_ShutDown();
void Wifi_UartInit();


BOOL Wifi_AtCmd_Test();

void WifiParamDef();

void EnterWifiMan();
void WifiManKey(PRESSKEY key);
void WifiStatusKey(PRESSKEY key);
void WifiModeKey(PRESSKEY key);
void WifiApKey(PRESSKEY key);
void WifiSrvKey(PRESSKEY key);
void WifiDetailKey(PRESSKEY key);

#endif

#endif
