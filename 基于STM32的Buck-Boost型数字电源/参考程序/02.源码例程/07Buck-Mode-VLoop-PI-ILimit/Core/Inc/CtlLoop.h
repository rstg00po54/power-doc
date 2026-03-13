#ifndef __CTLLOOP_H
#define __CTLLOOP_H	 

#include "stm32f3xx_hal.h"
#include "function.h"

void BUCKVLoopCtlPI(void);
void ILimitLoopCtl(void);

extern int32_t  VErr0;
extern int32_t	u0;
extern int32_t	IErr0;//电流误差
extern int32_t	i0;//电流环输出量

//一个开关周期数字量 
#define PERIOD 10240	 
#endif

