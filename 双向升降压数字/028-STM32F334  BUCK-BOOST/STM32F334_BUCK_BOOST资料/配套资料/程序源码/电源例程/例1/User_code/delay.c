/****************************************************************************************
  * @file    delay.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "delay.h"

void Delay_ms(__IO uint32_t nCount)
{
  while (nCount != 0)
  {
		nCount--;
	  Delay_us(1000);
  }
}
void Delay_us(__IO uint16_t time)
{
		 TIM16->CNT=0;			
	   while((TIM16->CNT)<time);   //TIME16,时钟频率为1MHz，计一个数为1us  
}
/*********************************************************************************************************************/
void Init_TIM16(void)                                                         //初始化定时器14
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;                              //初始化定时器用的结构体
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);                       //开启TIM14定时器时钟
	
	TIM_TimeBaseStructure.TIM_Prescaler=72000000/1000000-1;                     //1MHz
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;                   //向上计数
	TIM_TimeBaseStructure.TIM_Period=0xFFFF;                                    //满65536回到0重新计数
	TIM_TimeBaseStructure.TIM_ClockDivision=0;                                  //不分频
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;                              //重复溢出多少次才发生中断，0代表1,N+1
	TIM_TimeBaseInit(TIM16, &TIM_TimeBaseStructure);                            //初始化定时器
	TIM_SetAutoreload(TIM16,0xFFFF);                                            //自动加载数值到影子寄存器              
	TIM_ARRPreloadConfig(TIM16,ENABLE);                                         //自动加载值，因为我们没有开中断，所以使能或者失能都是可以的
 	TIM_Cmd(TIM16, ENABLE);                                                     //开启定时器
}
