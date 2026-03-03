/****************************************************************************************
  * @file    PID.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "PID.h"
/****************************电压环参数**********************************************/
CCMRAM __IO float        Kp_V=0.0f;             //比例系数
CCMRAM __IO float        Ki_V=0.0f;             //积分系数
CCMRAM __IO float        Kd_V=0.0f;             //微分系数
CCMRAM __IO float      	 error1_V=0.0f;         //当前输出和设定值误差
CCMRAM __IO float      	 error2_V=0.0f;         //当前输出和设定值误差
CCMRAM __IO float      	 e1_V=0.0f;             //上次输出和设定值误差
CCMRAM __IO float      	 e2_V=0.0f;             //上上次输出和设定值误差
/****************************电流环参数**********************************************/
CCMRAM __IO float        Kp_I=0.0f;             //比例系数
CCMRAM __IO float        Ki_I=0.0f;             //积分系数
CCMRAM __IO float        Kd_I=0.0f;             //微分系数
CCMRAM __IO float      	 error1_I=0.0f;         //当前输出和设定值误差
CCMRAM __IO float      	 error2_I=0.0f;         //当前输出和设定值误差
CCMRAM __IO float      	 e1_I=0.0f;             //上次输出和设定值误差
CCMRAM __IO float      	 e2_I=0.0f;             //上上次输出和设定值误差

CCMRAM __IO float        ERROR_Increase_V=0.0f; //占空比增加量
CCMRAM __IO float        ERROR_Increase_I=0.0f; //占空比增加量
CCMRAM __IO float        Duty_V=0.0f;           //占空比
CCMRAM __IO float        Duty_I=0.0f;           //占空比
CCMRAM __IO float        Duty  =0.0f;           //占空比

extern  __IO uint16_t    Target_voltage;        //目标电压
extern  __IO uint16_t    Target_current;        //目标电流

#define     Vosc         3.0f

CCMRAM void PID_INT(void)//PID初始化
{
       Kp_V=0.005f;             //比例系数
       Ki_V=0.001f;             //积分系数
       Kd_V=0.0f;             //微分系数
     	 error1_V=0.0f;         //当前输出和设定值误差
     	 error2_V=0.0f;         //当前输出和设定值误差
       e1_V=0.0f;             //上次输出和设定值误差
     	 e2_V=0.0f;             //上上次输出和设定值误差
	
       Kp_I=0.005f;             //比例系数
       Ki_I=0.001f;             //积分系数
       Kd_I=0.0f;             //微分系数
       error1_I=0.0f;         //当前输出和设定值误差
       error2_I=0.0f;         //当前输出和设定值误差
       e1_I=0.0f;             //上次输出和设定值误差
     	 e2_I=0.0f;             //上上次输出和设定值误差

       ERROR_Increase_V=0.0f; //占空比增加量
       ERROR_Increase_I=0.0f; //占空比增加量
       Duty_V=0.0f;           //占空比
       Duty_I=0.0f;           //占空比
       Duty  =0.0f;           //占空比
}

CCMRAM  __IO uint16_t Pulse_width=0;
CCMRAM void PID_Incremental(uint16_t Vout_actual,uint16_t Iout_actual)     //增量型PID
{
	//计算电压PID环路
	error1_V=(Target_voltage-Vout_actual)*0.001f;                    //当前误差
	e1_V=error1_V-error2_V;                                          //两次误差变化量
	ERROR_Increase_V+=Kp_V*e1_V+Ki_V*error1_V+Kd_V*(e1_V-e2_V);      //优化之后的PID算法(Kp*(ek-ek_1)+Ki*ek+Kd*(ek-2ek_1+ek_2))
	error2_V=error1_V;e2_V=e1_V;                                     //移位
	
	//限幅
	if(ERROR_Increase_V>=Vosc)ERROR_Increase_V=2.76f;                //最大占空比设定为2.76/3*100%=92%
	else if(ERROR_Increase_V<=0.06f)ERROR_Increase_V=0.06f;          //最小占空比设定为0.06/3*100%=2%
	
	//计算电流PID环路
	error1_I=(Target_current-Iout_actual)*0.001f;                    //当前误差
	e1_I=error1_I-error2_I;	                                         //两次误差变化量
	ERROR_Increase_I+=Kp_I*e1_I+Ki_I*error1_I+Kd_I*(e1_I-e2_I);      //优化之后的PID算法(Kp*(ek-ek_1)+Ki*ek+Kd*(ek-2ek_1+ek_2))	
	error2_I=error1_I;e2_I=e1_I;                                     //移位

	//限幅
	if(ERROR_Increase_I>=Vosc)ERROR_Increase_I=2.76f;                //最大占空比设定为2.76/3*100%=92%
	else if(ERROR_Increase_I<=0.06f)ERROR_Increase_I=0.06f;          //最小占空比设定为0.06/3*100%=2%
	
	
	//计算占空比
	//Duty_z=ERROR_Increase/Vosc;                                    //计算占空比
	
	Duty_V=ERROR_Increase_V*0.333333333f;                            //计算占空比
	Duty_I=ERROR_Increase_I*0.333333333f;                            //计算占空比
	
	Duty=Duty_V<Duty_I? Duty_V:Duty_I;                               //电压环和电流环取小的占空比
	
	/*************************** ************************************************/
	//解释：电压环和电流环取小的占空比，
  //条件：设定输出12V,OCP点3A。
	//恒压段：当电阻负载从无穷大减小到4Ω过程中，输出电流一直小于3A， 电流环认为有误差，占空比越来越大，逐渐接近100%，此时电压环占空比一定小于100%
	//恒流段：当电阻负载从4Ω继续减小过程中，    输出电压一直小于12V，电压环认为有误差，占空比越来越大，逐渐接近100%，此时电流环占空比一定小于100%
	/***************************************************************************/
	
	Pulse_width=Duty*PWM_PERIOD;
  /* Set the Capture Compare Register value */
  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = Pulse_width;  //更新HRTIM1中（比较寄存器的值）
	HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = Pulse_width>>1;     
}
