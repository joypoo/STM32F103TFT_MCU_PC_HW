#ifndef __USART_1_H
#define __USART_1_H

#include "sys.h"
#include <stdio.h>
#include <string.h>

#define USART1_RX_BUFF_MAX 256

typedef struct
{
	unsigned char RxBuff[USART1_RX_BUFF_MAX];
	unsigned char RecFlag;
	unsigned short RecLen;	
	int nAdrPrgflash; //写入SPI Flash的地址
	int nPackageNo;   //当前包号
}USART1_TypeDef;

extern USART1_TypeDef usart1;


void Usart1_Init(unsigned int bound);
void Usart1_SendString(unsigned char *str);
void Usart1_SendPackage(unsigned char *data, unsigned short len);



#endif
