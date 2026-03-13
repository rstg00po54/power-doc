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
int32_t   VErr0=0;//电压误差Q12
int32_t		u0=0;//电压环输出量
int32_t		IErr0=0;//电流误差
int32_t		i0=0;//电流环输出量
/*
** ===================================================================
**     Funtion Name :  void BUCKVLoopCtlPI(void)
**     Description :   增量式PI，电压环路环路计算
**     Parameters  :无
**     Returns     :无
** ===================================================================
*/
#define BUCK_VKP    	50//电压环PI环路P值 Q12，调试数据
#define BUCK_VKI     	5//电压环PI环路I值  Q12，调试数据
#define BUCK_MIN_INTE      327680//最小占空Q24 2%最小占空比
CCMRAM void BUCKVLoopCtlPI(void)
{
	//环路积分量Q24
	static  int32_t   V_Integral=0;
	int32_t VoutTemp=0;//输出电压矫正后
	
	//输出电压矫正
	VoutTemp = ((uint32_t )ADC1_RESULT[2]*CAL_VOUT_K>>12)+CAL_VOUT_B;
	//输出电流限制环
	ILimitLoopCtl();
	//计算电压误差量，当参考电压大于输出电压，占空比增加，输出量增加
	//CtrValue.Ilimitout为限流环输出，当电流小于限流值时，该值为0不起作用，
	//当电流大于限流值时，该值为正，通过减小CtrValue.Voref  限制输出电流继续增加
	VErr0= CtrValue.Voref  -  VoutTemp - CtrValue.Ilimitout;
	//电压环路输出=积分量+KP*误差量Q12
	u0= V_Integral + VErr0*BUCK_VKP;
	//积分量=积分量+KI*误差量
	V_Integral = V_Integral + VErr0*BUCK_VKI;
	//积分量限制，积分量最小限制，最小占空比
	if(V_Integral < BUCK_MIN_INTE )
		V_Integral = BUCK_MIN_INTE ;
	//积分量限制，积分量最大值限制，最大占空比
	if(V_Integral> (CtrValue.BUCKMaxDuty<<12))
		V_Integral = (CtrValue.BUCKMaxDuty<<12);
	
	//环路输出赋值，u0右移8位为BUCKPIDb0-2为人为放大Q12倍的数字量
	CtrValue.BuckDuty= u0>>12;
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


/*
** ===================================================================
**     Funtion Name :  void ILimitLoopCtl(void)
**     Description :  限流环
**     当输出电流小于限流值时，限流环输出为零
**     当输出电流大于限流值时，限流环输出为正
**     其输出叠加在电压环的电压参考值上面，通过降低电压环参考值，从而降低输出电流
**     Parameters  :无
**     Returns     :无
** ===================================================================
*/
#define ILIMIT_KP     5//电流环PI环路P值    Q12
#define ILIMIT_KI     1//电流环PI环路I值    Q12
CCMRAM void ILimitLoopCtl(void)
{
	//环路积分量Q24
	static  int32_t 	I_Integral=0;

	//计算电流误差量，当输出电流大于参考电流，输出量增加，慢速环，用平均值计算
	IErr0 = SADC.IoutAvg - CtrValue.ILimit;
	//电流环路输出=积分量+KP*误差量
	i0=I_Integral + IErr0*ILIMIT_KP;
	//积分量=积分量+KI*误差量
	I_Integral = I_Integral+ IErr0*ILIMIT_KI ;

	//积分量限制，积分量最小限制,-0.5V的反向偏置  （0.5/68）*Q24
	if( I_Integral < -123400)
		I_Integral = -123400;
	//积分量限制，积分量最大值限制，48V数字量对应Q48 （48/68）*Q24
	if(I_Integral >11840000)
		I_Integral=11840000 ;
	//输出最小限制
	if(i0 < 0)
		i0 = 0;
	
	CtrValue.Ilimitout = i0>>12;
}
