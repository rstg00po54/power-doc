#ifndef __INIT_H__
#define __INIT_H__

#include "stm32f30x.h"
#include "stm32f30x_conf.h"
#include "delay.h"
#include "stm32f30x_tim.h"
#include "ADC_INT.h"
#include "HRTIM_INT.h"
#include "state_machine.h"
#include "type2.h"
#include "Config.h"
#include "LED.h"




//CCMRAM:紧耦合内存，读取速度CCMRAM>SRAM>FLASH
#define CCMRAM  __attribute__((section("ccmram")))
//CCMRAM  uint8_t test(void)   把uint8_t test(void)函数放入CCMRAM中，加快代码运行速度

#define   IQ_math(A,B)   (((int32_t)(A*4096)*(int32_t)(B*4096))>>12)

void Initial_prepheral_(void);    //初始化外设，具体参考函数内部

void USART1_Config(void);         //初始化串口

void Send_data_to_PC(uint8_t no_pc);

void KEY_GPIO_InitConfig(void); //初始化KEY_IO口

#endif

