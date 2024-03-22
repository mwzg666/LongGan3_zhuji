#ifndef __PKUR_H__
#define __PKUR_H__


#ifdef PKUR
void Pkur_IoInit(void);
void Pkur_HwInit(void);
void SGetCount(unsigned char *data);
void SMeasreStart(unsigned char data);
void SSetLimen(unsigned char *datain);
void SCLR(void);
void Pkur_Init();
#endif

#endif

