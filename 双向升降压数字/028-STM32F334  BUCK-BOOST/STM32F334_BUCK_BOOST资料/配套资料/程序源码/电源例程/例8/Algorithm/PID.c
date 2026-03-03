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

CCMRAM __IO float        Kp=0.0f;              //比例系数
CCMRAM __IO float        Ki=0.0f;              //积分系数
CCMRAM __IO float        Kd=0.0f;              //微分系数
CCMRAM __IO float      	 ek=0.0f;              //当前输出和设定值误差
CCMRAM __IO float      	 ek_1=0.0f;            //上次输出和设定值误差
CCMRAM __IO float      	 e1=0.0f;              //上上次输出和设定值误差
CCMRAM __IO float      	 e2=0.0f;              //上上次输出和设定值误差
CCMRAM __IO float        ERROR_Increase=0.0f;  //占空比增加量
CCMRAM __IO float        Duty=0.0f;            //占空比

extern  __IO uint16_t    Target_voltage;

#define     Vosc         3.0f

CCMRAM void PID_INT(void)//PID初始化
{
	Kp  =0.005f;   
  Ki  =0.001f;   
	Kd  =0.000f;      
	ek  =0.0f;   
	ek_1=0.0f;   
	e1  =0.0f;   
	e2  =0.0f;   
	ERROR_Increase=0;
	Duty=0;
}

CCMRAM  __IO uint16_t Pulse_width=0;
CCMRAM void PID_Incremental(uint16_t Vout_actual)           //增量型PID
{
  ek=(Target_voltage-Vout_actual)*0.001f;                   //输出电压减去设定值
	e1=ek-ek_1;		
	
	ERROR_Increase+=Kp*e1+Ki*ek+Kd*(e1-e2);     						  //优化之后的PID算法(Kp*(ek-ek_1)+Ki*ek+Kd*(ek-2ek_1+ek_2))

	e2=e1;
	ek_1=ek;
	
	if(ERROR_Increase>=Vosc)ERROR_Increase=2.76f;              //最大占空比设定为2.76/3*100%=92%
	else if(ERROR_Increase<=0.06f)ERROR_Increase=0.06f;        //最小占空比设定为0.06/3*100%=2%
	
	//Duty_z=ERROR_Increase/Vosc;                              //计算占空比
	Duty=ERROR_Increase*0.333333333f;                          //计算占空比
	
	Pulse_width=Duty*PWM_PERIOD;
  /* Set the Capture Compare Register value */
  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = Pulse_width;  //更新HRTIM1中（比较寄存器的值）
	HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = Pulse_width>>1;     
}
