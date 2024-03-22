/*************************************************************


文件名:  crc.h
作  者:  潘国义
描  述:  CRC校验模块
修订记录:   

**************************************************************/


#ifndef __CRC_H__
#define __CRC_H__

#define GEN    0x5b9
#define P      0x31b
#define K21    32767
#define K2     32768
#define FLAG   1024
#define NK2    512
#define NK     0x7ff
#define TRAP   0x01f
#define RED2   9


WORD CRC16(BYTE *puchMsg, DWORD usDataLen);

#endif
