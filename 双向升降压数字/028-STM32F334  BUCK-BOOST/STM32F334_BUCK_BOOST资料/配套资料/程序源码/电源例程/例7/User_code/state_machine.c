/****************************************************************************************
  * @file    state_machine.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "state_machine.h"

/**********************定义变量***********************************/

CCMRAM __IO uint16_t   Vin[4]          ={0};
CCMRAM __IO uint16_t   Vout[4]         ={0};
CCMRAM __IO uint16_t   Iin[4]          ={0};
CCMRAM __IO uint16_t   Iout[4]         ={0};
CCMRAM __IO uint16_t   Temperature[4]  ={0};
CCMRAM __IO uint16_t   V_temp          =0;
CCMRAM __IO uint16_t   Target_voltage  =24000;

/**********************浮点变量**********************************/

CCMRAM __IO float      Vin_f           =0;
CCMRAM __IO float      Vout_f          =0;
CCMRAM __IO float      Iin_f           =0;     
CCMRAM __IO float      Iout_f          =0;
CCMRAM __IO float      temp_7900       =0;
CCMRAM __IO float      SOA_IN          =0;

/**********************定义标志位*********************************/

CCMRAM DP_FlagStatus    flag_Soft_start =START;
CCMRAM DP_FlagStatus    flag_start_cnt  =START;
CCMRAM DP_FlagStatus    flag_Vin_ovp    =STOP;
CCMRAM DP_FlagStatus    flag_Vin_uvp    =STOP;
CCMRAM DP_FlagStatus    flag_OTP        =STOP;
CCMRAM __IO uint8_t     i               =0;
CCMRAM __IO uint8_t     j               =0;
CCMRAM __IO uint8_t     k               =0;

/**********************通讯**************************************/

extern  __IO uint8_t    PC_command[6];
extern  __IO uint8_t    Send_to_PC[14];

/**********************外部变量*************************************/

extern __IO uint16_t  ADC1_RESULT[2];     //DMA无法访问CCRAM，故不能把变量定义在CCMRAM
//ADC1_RESULT[0]保存输出电压
//ADC1_RESULT[1]保存输出电流
extern __IO uint16_t  ADC2_RESULT[3];     //DMA无法访问CCRAM，故不能把变量定义在CCMRAM
//ADC2_RESULT[0]保存输入电压
//ADC2_RESULT[1]保存输入电流
//ADC2_RESULT[2]保存温度信息

/***********************保护参数配置**********************************/

#define  VIN_OVP_POINT    			        24500                //输入过压保护点      单位（mV）
#define  VIN_OVP_RECOVERY_POINT    			22000                //输入过压保护恢复点  单位（mV）

#define  VIN_UVP_POINT    			        12000                //输入欠压保护点      单位（mV）
#define  VIN_UVP_RECOVERY_POINT    			14000                //输入欠压保护恢复点  单位（mV）

#define  VOUT_OVP_POINT    			       (Target_voltage+3000) //输出欠压保护点      单位（mV）
#define  VOUT_UVP_POINT    			       (Target_voltage-3000) //输出欠压保护点      单位（mV）
 
#define  IOUT_OCP_POINT                 5000                 //输出过流保护点      单位（mA）
#define  IIN_OCP_POINT                  8000                 //输入过流保护点      单位（mA）

#define  INPUT_POWER                    200                  //最大输出功率        单位（W）
#define  OTP_POINT                      60                   //触发过温保护        单位（℃）
#define  OTP_RECOVERY_POINT             50                   //过温保护恢复点      单位（℃）

/**********************复位变量**************************************/
CCMRAM void Reset_VAR(void)
{
		flag_Soft_start =START;
		flag_start_cnt  =START;
	
		HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = PWM_PERIOD+100;                          //更新HRTIMA（比较寄存器的值）
    HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = 50;
  
    HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = 100;                                     //更新HRTIMB（比较寄存器的值）
	  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = 50;
	 for(i=0;i<4;i++)
		{
		 Vin[i] =0x00;
		 Vout[i]=0x00;
		 Iin[i] =0x00;
		 Iout[i]=0x00;
		 Temperature[i]=0x00;
		}
		
		V_temp=0x00;
		Target_voltage=24000;

	  Vin_f           =0;
	  Vout_f          =0;
	  Iin_f           =0;
	  Iout_f          =0;
	  temp_7900       =0;
	  SOA_IN          =0;
		
		for(i=0;i<6;i++)PC_command[i]=0x00;
	  for(i=0;i<14;i++)PC_command[i]=0x00;
	
		i               =0;
		j               =0;
		k               =0;
}
typedef enum
{
 Task_0_Initial_state,   //初始状态
 Task_1_Get_ADC_VALUE,   //获取ADC值
 Task_2_Vin_detc,        //检测输入电压
 Task_3_Vout_detc,       //检测输出电压
 Task_4_Iin_detc,        //检测输入电流
 Task_5_Iout_detc,       //检测输出电流
 Task_6_SOA_detc,        //检测输入功率
 Task_7_Temperature_detc,//检测温度
 Task_8_PC_command,      //和上位机通讯
 Task_9_Soft_start,      //开始缓启动
}System_Task;

CCMRAM System_Task Current_State,Next_State; 

CCMRAM void bcfsm(void)  //CCMRAM把函数放进CCRAM内存，加速执行速度
{
	LED_TEST();								          		//开机效果：流水灯
	type_3_tustin();
	Next_State=Current_State=Task_0_Initial_state;
	while(1)
	{	 
		 Current_State=Next_State;
		 switch(Current_State)
			{
				case Task_0_Initial_state:   //初始状态
													{	
														 Reset_VAR();													                                  //复位系统变量到初始状态
                             CS_DISPALY();																											    //电流指示LED复位
														 type_3_int();													                                //初始化Type3补偿器
														 Delay_ms(100);                                                         //打嗝时间
														 LED_OVP_OFF;LED_UVP_OFF;LED_OCP_OFF;LED_OTP_OFF;                       //关闭警示LED变化												  														  
														 Next_State=Task_1_Get_ADC_VALUE;				                                //下一状态：读取ADC值										
													}  
													break;
				case Task_1_Get_ADC_VALUE :  //获取ADC值
										      {
											if(DMA_GetFlagStatus(DMA1_FLAG_TC2) != RESET)
													{			
														DMA_ClearITPendingBit(DMA1_FLAG_TC2); 																  //清除中断标志位	
														DMA_ClearFlag(DMA1_FLAG_TC2);                                           //清楚传送完成标志位
														/***************************************计算Vin***************************************************************/
														Vin[3]  =(Vin[2]+Vin[1]+Vin[0]+ADC2_RESULT[0])>>2;                      //取输入电压平均值
			                      Vin[2]  =Vin[1];Vin[1]=Vin[0];Vin[0]=ADC2_RESULT[0];                    //移位
														
														Iin[3]  =(Iin[2]+Iin[1]+Iin[0]+ADC2_RESULT[1])>>2;                      //取输入电流平均值
			                      Iin[2]  =Iin[1];Iin[1]=Iin[0];Iin[0]=ADC2_RESULT[1];                    //移位
														
														Temperature[3]=(Temperature[2]+Temperature[1]+Temperature[0]+ADC2_RESULT[2])>>2;
			                      Temperature[2] =Temperature[1];Temperature[1]=Temperature[0];Temperature[0]=ADC2_RESULT[2];														
														if(k<100){k++;Next_State=Task_1_Get_ADC_VALUE; }                        //等待数据稳定，
                            else	Next_State=Task_2_Vin_detc;													              																															
													}
											else  Next_State=Task_1_Get_ADC_VALUE;                                        //等待数据更新
										}
										      break;
				case Task_2_Vin_detc:        //检测输入电压
													{
														Vin[3]=((Vin[3]*3300)>>13)*35+83;                                      		//2K和33K电阻分压，((Vin[3]*3300)>>12)*17.5;
														Vin_f=Vin[3]*0.001f;
														//Vin[3]=16000;
													/******************************判断是否过压*********************************************/	
													  if(Vin[3]>VIN_OVP_POINT)															 									//输入大于过压保护点
															{
																flag_Vin_ovp=START;LED_OVP_ON;															      		//标志位置1，亮灯警示
																Next_State=Task_0_Initial_state;                                		//下一状态：复位
																break;
															}
													 if(flag_Vin_ovp==START)                                          		      //过压保护中...
															{
															  if(Vin[3]<VIN_OVP_RECOVERY_POINT){flag_Vin_ovp=STOP;LED_OVP_OFF;	} 		//输入电压回落到过压恢复点，解除OVP
																else {Next_State=Task_0_Initial_state; break;}                  		//电压未回到正常值，下一状态：复位															
															}
													/******************************判断是否欠压*********************************************/		
													  if(Vin[3]<VIN_UVP_POINT)                                            		//输入电压跌落到欠压保护点，触发欠压保护
															{
																flag_Vin_uvp=START; LED_UVP_ON;	                           						//标志位置1，亮灯警示									
																Next_State=Task_0_Initial_state;
                                break;																
															}
													  if(flag_Vin_uvp==START)
															{
																if(Vin[3]>VIN_UVP_RECOVERY_POINT){flag_Vin_uvp=STOP;LED_UVP_OFF;}		  //输入电压回升到欠压恢复点，解除欠压保护
																else  {Next_State=Task_0_Initial_state;break;}                		  //电压未回到正常值，下一状态：复位																																									 	
												      }
												 /******************************电压正常*********************************************/	
														 Next_State=Task_3_Vout_detc;					 	                           			//开始检测输出	
											}															
												  break;
				case Task_3_Vout_detc :      //检测输出电压 
													{														
                            Vout_f=Vout[3]*0.001f;                           												
														if(Vout[3]>VOUT_OVP_POINT){Next_State=Task_0_Initial_state;break;}      //输出电压大于过压保护点，触发输出过压保护
														if(flag_Soft_start!=STOP){Next_State=Task_4_Iin_detc;break;}            //缓启动阶段不检测输出欠压
													  if(Vout[3]<VOUT_UVP_POINT){Next_State=Task_0_Initial_state;break;}      //输出电压小于UVP，触发输出欠压保护													
													  Next_State=Task_4_Iin_detc;						                                  //输出正常，开始检测电流																																																																		 												
												  }			
													break;
				case Task_4_Iin_detc :       //检测输入电流
													{													 
														 //Iin[3]=(float)((Iin[3]*3300)>>12)/20/10-8.25f;                       //8.25A代表1.65V的偏置
														 Iin_f=((Iin[3]*3300)>>12)*0.005f-8.21f;                 								//8.25A代表1.65V的偏置,(1650mV/20)/5mΩ=8.25A,
														 Iin[3]=Iin_f*1000;                          														//mA
														 if(flag_Soft_start==STOP)                                              //缓启动结束，开始检测输入电流
														 {                                                                      //缓启动阶段不检测输入电流保护，脉冲电流较大	
														 	 if(Iin[3]>IIN_OCP_POINT){Next_State=Task_0_Initial_state;break;}     //输入过流，复位功率回路
														 }                                 												 
														 Next_State=Task_5_Iout_detc;                                          
													}
													break;
				case Task_5_Iout_detc :      //检测输出电流
													{
														Iout[3]  =(Iout[2]+Iout[1]+Iout[0]+ADC1_RESULT[1])>>2;                  //取输入电流平均值
			                      Iout[2]  =Iout[1];Iout[1]=Iout[0];Iout[0]=ADC1_RESULT[1];               //移位
														Iout_f=((Iout[3]*3300)>>12)*0.005f-8.21f;                                //计算输出电流，5mΩ采样电阻，运放倍数20，偏置1.65V
													  Iout[3]=Iout_f*1000;                                                    //mA                                                   //mA
													  k++;
														if(k>200){CS_DISPALY();k=0;}                                            //200个周期刷新一次LED显示电流													
														if(Iout[3]>IOUT_OCP_POINT)                                              //
														{
															  LED_OCP_ON;	
														    Next_State=Task_0_Initial_state;                                    //过流，复位功率回路
															  break;
														}													
														else	Next_State=Task_6_SOA_detc;                                       //输出电流正常，开始检测SOA                       		
													}
													break;
				case Task_6_SOA_detc:        //检测输入功率
													{
													  SOA_IN=Vin_f*Iin_f;                                                     //单位：瓦
													  if(SOA_IN>INPUT_POWER)Next_State=Task_0_Initial_state;                  //输入功率大于设定值，复位
														else Next_State=Task_7_Temperature_detc;                                //输入功率正常，开始检测温度
													}
													break;
				case Task_7_Temperature_detc://检测温度
													{  
														Temperature[3]=(((Temperature[3]*3300)>>12)-500)/10;                    //Vout=TC1*TA+V0℃，MCP9700温度传感器       
														temp_7900=Temperature[3];                                               //TC1=19mV/℃，V0是温度0摄氏度是输出：400mV
														if(Temperature[3]>OTP_POINT)
															{
																 LED_OTP_ON;
																 flag_OTP=START;
																 Next_State=Task_0_Initial_state;                                     //过温，复位
																 break;
															}
                            if(flag_OTP==START)
															{
															  if(Temperature[3]<OTP_RECOVERY_POINT)                                 //温度下降到恢复点
																{
																  flag_OTP=STOP;LED_OTP_OFF;
																}
																else {Next_State=Task_0_Initial_state; break;}                        //温度低于过温度点，大于恢复点，复位   
															} 															
														Next_State=Task_8_PC_command;                                             
													}
													break;
				case Task_8_PC_command:      //和上位机通讯
													{	
														/****************************************指令描述********************************************************/
														/* AA 55 01 2E E0 08 */    //PC设置电压指令格式：帧头：AA 55，指令类别：01，电压：2EE0（12000mV）,帧尾：08													
														/* AA 55 02 00 00 08 */    //PC查询信息指令格式：帧头：AA 55，指令类别：02，无效位：XX XX,        帧尾：08													
														/* AA 55 08 AA   XX XX XX XX   XX XX XX XX   XX 08 */  //共14个字节
														                 ////MCU回复指令格式：输入电压，输入电流，输出电压，输出电流，温度
														/*******************************************END**********************************************************/														
														if(DMA_GetFlagStatus(DMA1_FLAG_TC4) != RESET){DMA_ClearFlag(DMA1_FLAG_TC4);} //发送完成标志为清零
														if(DMA_GetFlagStatus(DMA1_FLAG_TC5) != RESET)                                //接收完成标志为清零
															{
																DMA_ClearFlag(DMA1_FLAG_TC5);
																if(PC_command[0]==0xAA&&PC_command[1]==0x55)
																		{																
																			if(PC_command[2]==0x01){Target_voltage=((uint16_t)PC_command[3]<<8)+PC_command[4];}														
																			else if(PC_command[2]==0x02)
																						{
																							Send_to_PC[0]=0xAA;   
																							Send_to_PC[1]=0x55; 
																							Send_to_PC[2]=0x08;//ACK应答位
																							Send_to_PC[3]=0XAA;																
																							Send_to_PC[4]=Vin[3]>>8;  Send_to_PC[5]=Vin[3];
																							Send_to_PC[6]=Iin[3]>>8;  Send_to_PC[7]=Iin[3]; 														
																							Send_to_PC[8]=Vout[3]>>8; Send_to_PC[9]=Vout[3];    														
																							Send_to_PC[10]=Iout[3]>>8;Send_to_PC[11]=Iout[3];														
																							Send_to_PC[12]=Temperature[3];Send_to_PC[13]=0x08; 																
																							Send_data_to_PC(14);                               																
																						}														
																	 }
																else 
																	{
																	 PC_command[0]=0x00;
																	 PC_command[1]=0x00;
																	 DMA_SetCurrDataCounter(DMA1_Channel5, 6); 
																	}
															 PC_command[2]=0x00;                                                       //标志为置零
														}
													if(flag_start_cnt!=STOP)Next_State=Task_9_Soft_start;                          //进入缓起状态
                          else Next_State=Task_1_Get_ADC_VALUE;																
													}
													break;
				case Task_9_Soft_start :     //缓启动
													{
														  //TIM15->CNT=0x00;	                                                       //开始计时
															flag_start_cnt=STOP;																											 //计时结束									                 														
													  	Next_State=Task_1_Get_ADC_VALUE;													                 //回状态1
													}														
													break;															
				default:  break;
			}
	}
}
CCMRAM void HRTIM1_TIMA_IRQHandler(void)
{
	if( HRTIM_GetITStatus( HRTIM1,  HRTIM_TIMERINDEX_TIMER_A, HRTIM_TIM_FLAG_REP) != RESET)
	{
		HRTIM_ClearITPendingBit( HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_TIM_FLAG_REP);			//清楚中断标志位
		//HRTIM_ClearFlag(HRTIM1,HRTIM_TIMERINDEX_TIMER_A,HRTIM_TIM_FLAG_REP);
	  Vout[3]  =(Vout[2]+Vout[1]+Vout[0]+ADC1_RESULT[0])>>2;
	  Vout[2]  =Vout[1];Vout[1]=Vout[0];Vout[0]=ADC1_RESULT[0];
	  Vout[3]  =((Vout[3]*3300)>>13)*35+83;
	  if(flag_start_cnt==STOP)
		 {
			type_3_cal(Vout[3]);
			if(flag_Soft_start!=STOP)
			 {		
				 if(V_temp>Target_voltage){flag_Soft_start=STOP;}
				 else  V_temp+=5;		   	 
			 }
		 }
	}
}
//void DMA1_Channel1_IRQHandler(void)
//{
//if(DMA_GetFlagStatus(DMA1_FLAG_TC1) != RESET)       //判断传输完成标志为
//     {
//			DMA_ClearITPendingBit(DMA1_FLAG_TC1);
//			DMA_ClearFlag(DMA1_FLAG_TC1);                 //清满标志
//			 Vout[3]  =(Vout[2]+Vout[1]+Vout[0]+ADC1_RESULT[0])>>2;
//			 Vout[2]  =Vout[1];Vout[1]=Vout[0];Vout[0]=ADC1_RESULT[0];
//			 Vout[3]  =((Vout[3]*3300)>>13)*35;
//			 if(flag_start_cnt==STOP)
//				 {
//					type_3_cal(Vout[3]);
//					if(flag_Soft_start!=STOP)
//					 {		
//						 if(V_temp>Target_voltage){flag_Soft_start=STOP;}
//						 else  V_temp+=5;		   	 
//					 }
//				 }		 			
//	   }			
//}
CCMRAM void CS_DISPALY(void)
{
	if(Iout[3]<1000){LED_1A0_OFF;LED_2A0_OFF;LED_3A0_OFF;LED_4A0_OFF;LED_5A0_OFF;}       //输出电流小于1.0A
	else if(Iout[3]<2000){LED_1A0_ON;LED_2A0_OFF;LED_3A0_OFF;LED_4A0_OFF;LED_5A0_OFF;}   //输出电流小于2.0A
	else if(Iout[3]<3000){LED_1A0_ON;LED_2A0_ON;LED_3A0_OFF;LED_4A0_OFF;LED_5A0_OFF;}    //输出电流小于3.0A
	else if(Iout[3]<4000){LED_1A0_ON;LED_2A0_ON;LED_3A0_ON;LED_4A0_OFF;LED_5A0_OFF;}     //输出电流小于4.0A
	else if(Iout[3]<5000){LED_1A0_ON;LED_2A0_ON;LED_3A0_ON;LED_4A0_ON;LED_5A0_OFF;}      //输出电流小于5.0A
	else {LED_1A0_ON;LED_2A0_ON;LED_3A0_ON;LED_4A0_ON;LED_5A0_ON;}                       //输出电流5.0A以上
}
