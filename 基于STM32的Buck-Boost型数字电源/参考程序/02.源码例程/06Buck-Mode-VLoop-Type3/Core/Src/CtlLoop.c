/* USER CODE BEGIN Header */
/****************************************************************************************
	* 
	* @author  文七电源
	* @淘宝店铺链接：https://shop598739109.taobao.com
	* @file           : CtlLoop.c
  * @brief          : 环路功能函数
  * @version V1.0
  * @date    01-03-2021
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权文七电源所有
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
	
/* USER CODE END Header */
#include "CtlLoop.h"

/****************环路变量定义**********************/
int32_t   VErr0=0,VErr1=0,VErr2=0,VErr3=0;//电压误差Q12
int32_t		u0=0,u1=0,u2=0,u3=0;//电压环输出量
/*
** ===================================================================
**     Funtion Name :  void BUCKVLoopCtlTYPE3(void)
**     Description :   电压环路环路计算-type3电压环路计算-（参照BUCK-TYPE3环路计算文档）
**     Parameters  :无
**     Returns     :无
** ===================================================================
*/

//环路的参数，具体参照mathcad计算文件《恒压输出-恒压-Tpye3型补偿器》
#define BUCKTYPETb0	4995		//Q8
#define BUCKTYPETb1	-4838		//Q8
#define BUCKTYPETb2	-4992		//Q8
#define BUCKTYPETb3	4838		//Q8
#define BUCKTYPETa1	5079		//Q12
#define BUCKTYPETa2	-1043		//Q12
#define BUCKTYPETa3	60			//Q12
CCMRAM void BUCKVLoopCtlTYPE3(void)
{
	int32_t VoutTemp=0;//输出电压矫正后
	
	//输出电压矫正
	VoutTemp = ((uint32_t )ADC1_RESULT[2]*CAL_VOUT_K>>12)+CAL_VOUT_B;
	//计算电压误差量，当参考电压大于输出电压，占空比增加，输出量增加
	VErr0= CtrValue.Voref  - VoutTemp;
	//调用PID环路计算公式（参照BUCK-TYPE3环路计算文档）
	u0 = (( u1*BUCKTYPETa1 + u2*BUCKTYPETa2 + u3*BUCKTYPETa3)>>12) + ((VErr0*BUCKTYPETb0 + VErr1*BUCKTYPETb1 + VErr2*BUCKTYPETb2 + VErr3*BUCKTYPETb3)>>8);	
	//历史数据幅值
	VErr3 = VErr2;
	VErr2 = VErr1;
	VErr1 = VErr0;
	u3 = u2;
	u2 = u1;
	u1 = u0;
	//环路输出赋值
	CtrValue.BuckDuty= u0;
	CtrValue.BoostDuty=MIN_BOOST_DUTY1;//BOOST上管固定占空比93%，下管7%			
	//环路输出最大最小占空比限制
	if(CtrValue.BuckDuty > CtrValue.BUCKMaxDuty)
		CtrValue.BuckDuty = CtrValue.BUCKMaxDuty;	
	if(CtrValue.BuckDuty < MIN_BUKC_DUTY)
		CtrValue.BuckDuty = MIN_BUKC_DUTY;
	//PWMENFlag是PWM开启标志位，当该位为0时,buck的占空比为0，无输出;
	if(DF.PWMENFlag==0)
		CtrValue.BuckDuty = MIN_BUKC_DUTY;
	//更新对应寄存器
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = CtrValue.BuckDuty * PERIOD>>12; //buck占空比
  HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR>>1; //ADC触发采样点
	HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = PERIOD - (CtrValue.BoostDuty * PERIOD>>12);//Boost占空比
}
