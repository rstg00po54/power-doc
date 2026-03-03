/****************************************************************************************
  * @file    type3.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "init.h"
#include "type3.h"

//CCMRAM:紧耦合RAM，读取速度CCMRAM>SRAM>FLASH
CCMRAM __IO float      Vosc;                 //三角波（模拟补偿器输出和三角波比较生成PWM）
CCMRAM __IO float      Xn,Xn_1,Xn_2,Xn_3;   //差分方程输入项：输出电压-参考电压
CCMRAM __IO float      Yn,Yn_1,Yn_2,Yn_3;   //差分方程输出项：误差放大器输出Verror
CCMRAM __IO float      Duty;								    //占空比

/**********************仿真1us************************/
CCMRAM __IO float k1=-0.415017720023f;            //差分方程系数：对应差分方程各项  OK的
CCMRAM __IO float k2=0.37445855483f;
CCMRAM __IO float k3=0.414919275447f;
CCMRAM __IO float k4=-0.37455699940f;
CCMRAM __IO float k5=1.113014372908f;
CCMRAM __IO float k6=-0.039574719432f;
CCMRAM __IO float k7=-0.073439653475f;

extern __IO uint16_t Target_voltage;
CCMRAM void type_3_int(void)
{

 Xn=0;         //误差X(n)
 Xn_1=0;       //误差X(n-1)
 Xn_2=0;       //误差X(n-1)
 Xn_3=0;       //误差X(n-2)
	
 Yn=0;         //补偿器输出Y(n)
 Yn_1=0;       //补偿器输出Y(n-1)
 Yn_2=0;       //补偿器输出Y(n-2)
 Yn_3=0;       //补偿器输出Y(n-3)

 Duty=0.0f;
 
 Vosc  =3.0f;
	
//Y(n)=k1*X(n)+k2*X(n-1)+k3*X(n-2)+k3*X(n-3)+k4*Y(n)+k5*Y(n-1)+k6*Y(n-2)+k7*Y(n-3);
}

CCMRAM __IO uint16_t Pulse_with_type3p3z=0;
CCMRAM void type_3_cal(uint16_t vout_temp)                   //TYPE3算法
{
	 Xn=(float)(vout_temp-Target_voltage)*0.001f;              //输出电压-参考电压
	 Yn=k1*Xn+k2*Xn_1+k3*Xn_2+k4*Xn_3+k5*Yn_1+k6*Yn_2+k7*Yn_3; //这个方程代表模拟电路type3补偿器

	 if(Yn>=Vosc){Yn=Vosc-0.24f;}					                     //最大占空比92%                          
	 else if(Yn<=0.06f){Yn=0.06f;}                             //最小占空比2.0%
	 Duty=Yn*0.333333333f;	    								               //Y(n)/Vosc
	 
	 Xn_3=Xn_2;Xn_2=Xn_1;Xn_1=Xn;							                 //移位				 														 
	 Yn_3=Yn_2;Yn_2=Yn_1;Yn_1=Yn;	                             //移位	
	 
	 Pulse_with_type3p3z=Duty*PWM_PERIOD;                      //计算脉宽
	 
	 if(Pulse_with_type3p3z<=100)Pulse_with_type3p3z=100;
	 /* Set the Capture Compare Register value */
	 HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = Pulse_with_type3p3z;  //更新HRTIM1中（比较寄存器的值）
	 HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = Pulse_with_type3p3z>>1;  	 
}

void type_3_tustin(void)
{
	double R1,R2,R3,C1,C2,C3,Ts;
	double a1,a2,a3,a4,a5,k0,b1,b2,b3,b4,b5;
	double Gain_adj;
	R1=2.6*Kom;C1=2.2*nF;
	R2=3.3*Kom;C2=20.0*nF;
	R3=47.0*om;C3=100.0*nF;

  Ts=10.0*uS ;//%sampling period
	
  Gain_adj=50.0f;
	
	a1=R1*C1/Ts;
	a2=R1*C2/Ts;
	a3=R1*C3/Ts;
	a4=R2*C2/Ts;
  a5=R3*C3/Ts;
	
	b1=(a3*a4+a4*a5)*4;
	b2=(a4+a3+a5)*2;
	b3=(a1*a4+a1*a5+a2*a5)*4;
	b4=(a1*a4*a5)*8;
	b5=(a1+a2)*2;
	
	k0=(b5+b3+b4);
	k1=(-1-b2-b1)/k0;
	k2=(-3-b2+b1)/k0;
	k3=(-3+b2+b1)/k0;
	k4=(-1+b2-b1)/k0;
	k5=-(b5-b3-3*b4)/k0;
	k6=-(-b5-b3+3*b4)/k0;
	k7=-(-b5+b3-b4)/k0;
	
	k1=k1/Gain_adj;
  k2=k2/Gain_adj;
  k3=k3/Gain_adj;
  k4=k4/Gain_adj;
}
