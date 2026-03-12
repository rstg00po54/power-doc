#ifndef __LED_H__
#define __LED_H__

#include "init.h"

void LED_GPIO_InitConfig(void); //初始化IO口

void LED_TEST(void);


//	PB9   LED(绿色)   1.0A   当前电流1.0A
//	PB8   LED(绿色)   2.0A   当前电流2.0A
//	PB5   LED(绿色)   3.0A   当前电流3.0A
//	PB4   LED(绿色)   4.0A   当前电流4.0A
//	PB3   LED(绿色)   5.0A   当前电流5.0A
//	
//	PA15  LED(红色)   OCP    当前过流
//	PA12  LED(红色)   OVP    当前过压
//	PB15  LED(红色)   UVP    当前过压
//	PB14  LED(红色)   OTP    当前过温

#define LED_1A0_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_9)
#define LED_1A0_OFF GPIO_SetBits(GPIOB,GPIO_Pin_9)

#define LED_2A0_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_8)
#define LED_2A0_OFF GPIO_SetBits(GPIOB,GPIO_Pin_8)

#define LED_3A0_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_5)
#define LED_3A0_OFF GPIO_SetBits(GPIOB,GPIO_Pin_5)

#define LED_4A0_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_4)
#define LED_4A0_OFF GPIO_SetBits(GPIOB,GPIO_Pin_4)

#define LED_5A0_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_3)
#define LED_5A0_OFF GPIO_SetBits(GPIOB,GPIO_Pin_3)

#define LED_OCP_ON  GPIO_ResetBits(GPIOA,GPIO_Pin_15)
#define LED_OCP_OFF GPIO_SetBits(GPIOA,GPIO_Pin_15)

#define LED_OVP_ON  GPIO_ResetBits(GPIOA,GPIO_Pin_12)
#define LED_OVP_OFF GPIO_SetBits(GPIOA,GPIO_Pin_12)

#define LED_UVP_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_15)
#define LED_UVP_OFF GPIO_SetBits(GPIOB,GPIO_Pin_15)

#define LED_OTP_ON  GPIO_ResetBits(GPIOB,GPIO_Pin_14)
#define LED_OTP_OFF GPIO_SetBits(GPIOB,GPIO_Pin_14)

#endif
