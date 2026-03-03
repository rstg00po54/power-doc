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
typedef struct {
__IO float        Kp;             //比例系数
__IO float        Ki;             //积分系数
__IO float        Kd;             //微分系数
__IO float      	ek;             //当前输出和设定值误差
__IO float      	ek_1;           //当前输出和设定值误差
__IO float      	e1;             //上次输出和设定值误差
__IO float      	e2;             //上上次输出和设定值误差
__IO float        Error_Increase; //占空比增加量
}LOOP_TypeDef;

CCMRAM LOOP_TypeDef buck_pid;
CCMRAM LOOP_TypeDef boost_pid;
CCMRAM LOOP_TypeDef buck_boost_pid;

CCMRAM DP_FlagStatus buck_mode;
CCMRAM DP_FlagStatus boost_mode;
CCMRAM DP_FlagStatus buck_boost_mode;
CCMRAM DP_FlagStatus mode_update;

CCMRAM  __IO uint16_t Pulse_width=0;
CCMRAM  __IO float       Duty  =0.0f;          //占空比
extern  __IO uint16_t    Target_voltage;        //目标电压
extern  __IO float       Vin_f;

#define     Vosc         3.0f
CCMRAM void PID_INT(void)//PID初始化
{
/****************************BUCK电压环参数**********************************************/	   
	     buck_pid.Kp=0.005f;
	     buck_pid.Ki=0.01f;
	     buck_pid.Kd=0.003f;
	     buck_pid.ek=0.0f;
	     buck_pid.ek_1=0.0f;
	     buck_pid.e1=0.0f;
	     buck_pid.e2=0.0f;
	     buck_pid.Error_Increase=0.0f;
/****************************BOOST电压环参数**********************************************/	
		   boost_pid.Kp=0.005f;
	     boost_pid.Ki=0.001f;
	     boost_pid.Kd=0.0f;
	     boost_pid.ek=0.0f;
	     boost_pid.ek_1=0.0f;
	     boost_pid.e1=0.0f;
	     boost_pid.e2=0.0f;
	     boost_pid.Error_Increase=0.0f;
/****************************BUCK_BOOST电压环参数*****************************************/			 
			 buck_boost_pid.Kp=0.005f;
	     buck_boost_pid.Ki=0.001f;
	     buck_boost_pid.Kd=0.0f;
	     buck_boost_pid.ek=0.0f;
	     buck_boost_pid.ek_1=0.0f;
	     buck_boost_pid.e1=0.0f;
	     buck_boost_pid.e2=0.0f;
	     buck_boost_pid.Error_Increase=0.0f;
/******************************************************************************************/
       buck_mode=STOP;
		   boost_mode=START;
		   buck_boost_mode=STOP;
			 mode_update=STOP;
			 
			 Pulse_width=0.0f;
			 Duty=0.0f;
}

CCMRAM void PID_CAL(LOOP_TypeDef* mode,uint16_t vo)
{
	
  mode->ek=(Target_voltage-vo)*0.001f;        //当前误差
	mode->e1=mode->ek-mode->ek_1;		            //误差变化量
	
 //优化之后的PID算法(Kp*(ek-ek_1)+Ki*ek+Kd*(ek-2ek_1+ek_2))
	mode->Error_Increase+=mode->Kp*mode->e1+mode->Ki*mode->ek+mode->Kd*(mode->e1-mode->e2);

	mode->e2=mode->e1;                          //移位                                             
	mode->ek_1=mode->ek;                        //移位
	if(buck_boost_mode==START)
		{
			if(mode->Error_Increase>=Vosc)mode->Error_Increase=1.35f;                //最大占空比设定为1.35/3*100%=45%
			else if(mode->Error_Increase<=0.06f)mode->Error_Increase=0.06f;          //最小占空比设定为0.06/3*100%=2%
		}
	else
	  {
			if(mode->Error_Increase>=Vosc)mode->Error_Increase=2.85f;                //最大占空比设定为2.76/3*100%=92%
			else if(mode->Error_Increase<=0.06f)mode->Error_Increase=0.06f;          //最小占空比设定为0.06/3*100%=2%
	  }

}

CCMRAM void PID_Incremental(uint16_t Vout_actual)     //增量型PID
{
	//输入12-22V时工作在BOOST模式，22-26V时工作在BUCK-BOOST模式，26-36V时工作在BUCK模式,模式自动无缝切换
	if(buck_boost_mode==START)
		{
			if(Vin_f<22.0f)     {buck_mode=STOP;boost_mode=START;buck_boost_mode=STOP;mode_update=START;}
			else if(Vin_f>26.0f){buck_mode=START;boost_mode=STOP;buck_boost_mode=STOP;mode_update=START;}
		}
	else if(buck_mode==START)
		{
		  if(Vin_f<25.0f){buck_mode=STOP;boost_mode=STOP;buck_boost_mode=START;mode_update=START;}
		}
	else 
		{
		  if(Vin_f>23.0f){buck_mode=STOP;boost_mode=STOP;buck_boost_mode=START;mode_update=START;}
		}

	if(buck_mode==START)
	{
		PID_CAL(&buck_pid,Vout_actual);        //计算BUCK——PID环路	
	  Duty=buck_pid.Error_Increase*0.333333333f;
	  Pulse_width=Duty*PWM_PERIOD;
		if(mode_update==START){TB1_L_TB2_H();mode_update=STOP;}
		if(Pulse_width<100)Pulse_width=100;
		HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = Pulse_width;    //更新HRTIM1中（比较寄存器的值）
	  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = Pulse_width>>1; 
		HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = 100;              //Q3驱动脉宽设置为0，Q4驱动脉宽100%
	  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = 50; 
	}
	else if(boost_mode==START)
	{
	  PID_CAL(&boost_pid,Vout_actual);       //计算BOOST——PID环路
	  Duty=boost_pid.Error_Increase*0.333333333f;
	  Pulse_width=Duty*PWM_PERIOD;
		if(mode_update==START){TA1_H_TA2_L();mode_update=STOP;}
		HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = PWM_PERIOD+100;    //Q1驱动脉宽设置为100%，Q4驱动脉宽0
	  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = Pulse_width>>1; 
		HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = Pulse_width;       //更新HRTIM1中（比较寄存器的值）
	  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = Pulse_width>>1; 
	}
	else 
	{
	  PID_CAL(&buck_boost_pid,Vout_actual);  //计算BUCK_BOOST——PID环路
		Duty=buck_boost_pid.Error_Increase*0.333333333f;
	  Pulse_width=Duty*PWM_PERIOD;
		if(mode_update==START){GPIO_HRTIM_Config();mode_update=STOP;}
		HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = 18432;         //更新HRTIM1中（比较寄存器的值）
	  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = Pulse_width>>1; 
		HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = Pulse_width;    //更新HRTIM1中（比较寄存器的值）
	  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = Pulse_width>>1; 
	  
		//***Vo/Vin=f(Duty_buck)*f(Duty_boost)=Duty_buck/(1-Duty_boost),Duty_buck is fixed at 0.8, Duty_boost varying from 0.05 to 0.45
		//   Vo/Vin ratio varying from 0.85 to 1.45,thus covering the entire range of VOUT close to VIN(16V~28V)
	}   
}
