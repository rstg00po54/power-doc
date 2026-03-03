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

CCMRAM __IO float        Kp=0.0f;             //比例系数
CCMRAM __IO float        Ki=0.0f;             //积分系数
CCMRAM __IO float        Kd=0.0f;             //微分系数
CCMRAM __IO float      	 ek=0.0f;             //当前输出和设定值误差
CCMRAM __IO float      	 ek_1=0.0f;           //上次输出和设定值误差
CCMRAM __IO float      	 ek_2=0.0f;           //上上次输出和设定值误差
CCMRAM __IO float        Duty_Increase=0.0f;  //占空比增加量
CCMRAM __IO float        Duty=0.0f;           //占空比
CCMRAM __IO float        Vosc;                //三角波（模拟补偿器输出和三角波比较生成PWM）

extern  __IO uint16_t    Target_voltage;
CCMRAM  __IO uint16_t    Pulse_width=0;

/*********************调试方法************************/
//第一步:设置KP=0,KI=0,KD=0
//第二步：KP从零缓慢增大，直到输出有轻微过冲，保留KP不变
//第三步：KI从零缓慢增大，直到系统稳定
//第四步：KD从零缓慢增大，直到开机过冲消失
//KPP可以理解为增益旋钮
//关于PID网上有很多调试方法，这里不再赘述
/*******************************************************/
#define Kpp  0.0015f 
#define Ti   0.0000025f     //PID调节的积分常数
#define Td   0.010f     //PID调节的微分时间常数
#define T    0.000005f     //采样周期


CCMRAM void PID_INT(void)//PID初始化
{
  Kp=Kpp * ( 1 + (T / Ti) + (Td / T) );
	Ki=(-Kpp) * ( 1 + (2 * Td / T ) );   
	Kd=Kpp * Td / T;
	ek=0;
	ek_1=0;
	ek_2=0;
	Duty_Increase=0;
	Duty=0;
}

CCMRAM void PID_Incremental(uint16_t Vout_actual)         //增量型PID
{
  ek=(float)(Target_voltage-Vout_actual)*0.001f;          //输出电压减去设定值
	Duty_Increase+=Kp*ek+Ki*ek_1+Kd*ek_2;     						  //优化之后的PID算法
	ek_2=ek_1;                                              //移位
	ek_1=ek;                                                //移位

	if(Duty_Increase>=Vosc)Duty_Increase=Vosc-0.24f;        //最大占空比92% 
	else if(Duty_Increase<=0.06f)Duty_Increase=0.06f;       //最小占空比2.0%
	
	//Duty=Duty_Increase/Vosc;                              //计算占空比
	Duty=Duty_Increase*0.333333333f;                        //计算占空比
	
	Pulse_width=Duty*PWM_PERIOD;
  /* Set the Capture Compare Register value */
  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = Pulse_width;  //更新HRTIM1中（比较寄存器的值）
	HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = Pulse_width>>1;     
}
