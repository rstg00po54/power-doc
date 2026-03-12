#ifndef __ADC_INT_H__
#define __ADC_INT_H__

#include "init.h"
#define ADC1_DR_ADDRESS     0x50000040
#define ADC2_DR_ADDRESS     0x50000140
/*********************º¯ÊýÉùÃ÷Çø**************************************************************************************************/
 void ADC1_Config(void);
 void ADC2_Config(void);
 
 void ADC1_DMA_Config(void);
 void ADC2_DMA_Config(void);
 void ADC_Config(void);
#endif
