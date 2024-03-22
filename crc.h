/*************************************************************


�ļ���:  crc.h
��  ��:  �˹���
��  ��:  CRCУ��ģ��
�޶���¼:   

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
