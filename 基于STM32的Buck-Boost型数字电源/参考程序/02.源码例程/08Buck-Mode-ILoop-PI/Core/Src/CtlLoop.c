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
int32_t		IErr0=0;//电流误差
int32_t		i0=0;//电流环输出量


/*
** ===================================================================
**     Funtion Name :  void BUCKILoopCtlPI(void)
**     Description :  Buck模式下恒流输出环
**     Parameters  :无
**     Returns     :无
** ===================================================================
*/
#define ILOOP_KP     8//电流环PI环路P值    Q12
#define ILOOP_KI     2//电流环PI环路I值    Q12
#define BUCK_MIN_INTE      327680//最小占空Q24 2%最小占空比
CCMRAM void BUCKILoopCtlPI(void)
{
	//环路积分量Q24
	static  int32_t I_Integral=0;
	static  int32_t	IoutTemp=0;

	//获取矫正后的输出电流
	IoutTemp = (((int32_t )ADC1_RESULT[3] - SADC.IoutOffset)*CAL_IOUT_K>>12)+CAL_IOUT_B;
	
	//计算电流误差量，当输出电流小于参考电流，输出量增加，慢速环，
	IErr0 = CtrValue.Ioref - IoutTemp;
	//电流环路输出=积分量+KP*误差量
	i0=I_Integral + IErr0*ILOOP_KP;
	//积分量=积分量+KI*误差量
	I_Integral = I_Integral+ IErr0*ILOOP_KI ;
	
	//积分量限制，积分量最小限制，最小占空比
	if(I_Integral < BUCK_MIN_INTE )
		I_Integral = BUCK_MIN_INTE ;
	//积分量限制，积分量最大值限制，最大占空比
	if(I_Integral> (CtrValue.BUCKMaxDuty<<12))
		I_Integral = (CtrValue.BUCKMaxDuty<<12);
	
	//环路输出赋值，u0右移8位为IKP为人为放大Q12倍的数字量
	CtrValue.BuckDuty= i0>>12;
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
