#ifndef __DELAY_H__
#define __DELAY_H__
#include "stm32f30x.h"
#include "stm32f30x_conf.h"
#include "init.h"
#include "delay.h"

void Delay_us(uint16_t time);
void Delay_ms(__IO uint32_t nCount);
void Init_TIM16(void);               //初始化定时器16做延时函数使用
#endif
