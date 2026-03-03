/****************************************************************************************
  * @file    HRTIM_INT.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#include "HRTIM_INT.h"


/**
  * @brief  Configure GPIO outputs for the HRTIM
  * @param  None
  * @retval None
  */
//  PWM输出IO配置
//	PIN_29 PA8    HRTIM_TA1  PWMH_L输出
//	PIN_30 PA9    HRTIM_TA2  PWML_L输出
//	PIN_31 PA10   HRTIM_TB1  PWMH_R输出
//	PIN_32 PA11   HRTIM_TB2  PWML_R输出
void GPIO_HRTIM_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  
  /* Configure HRTIM output: TA1 (PA8) */
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  /* Alternate function configuration : HRTIM TA1 (PA8) */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_13);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_13);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_13);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource11, GPIO_AF_13);
  
}
/**
  * @brief  HRTIM configuration
  * @param  None
  * @retval None
  */
void HRTIM_Config(void)
{
  HRTIM_OutputCfgTypeDef HRTIM_TIM_OutputStructure;       								  		//输出通道置位复位配置
  HRTIM_BaseInitTypeDef HRTIM_BaseInitStructure;         									  		//定时器参数配置，周期等
  HRTIM_TimerInitTypeDef HRTIM_TimerInitStructure;       									  		//定时器模式初始化
  HRTIM_TimerCfgTypeDef HRTIM_TimerWaveStructure;         								 			//输出波形配置 
  HRTIM_CompareCfgTypeDef HRTIM_CompareStructure;        									  		//比较器配置
	HRTIM_ADCTriggerCfgTypeDef	HRTIM_ADCTrigStructure;     											//ADC触发设置
	HRTIM_DeadTimeCfgTypeDef HRTIM_TIM_DeadTimeStructure;  									  		//死区时间配置
	NVIC_InitTypeDef 						NVIC_InitStructure;      												  //中断服务函数设置
  
  RCC_HRTIM1CLKConfig(RCC_HRTIM1CLK_PLLCLK);             												//选择时钟来源72MHZ*2=144MHZ
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_HRTIM1, ENABLE); 												//使能时钟
  
  HRTIM_DLLCalibrationStart(HRTIM1, HRTIM_CALIBRATIONRATE_14); 									//时钟矫正
  HRTIM1_COMMON->DLLCR |= HRTIM_DLLCR_CALEN;                   									//使能
  
  while(HRTIM_GetCommonFlagStatus(HRTIM1, HRTIM_ISR_DLLRDY) == RESET); 					//等待矫正完成
  
  HRTIM_TimerInitStructure.HalfModeEnable = HRTIM_HALFMODE_DISABLED;  					//HALF用于调频，Duty=50%，不用
  HRTIM_TimerInitStructure.StartOnSync = HRTIM_SYNCSTART_DISABLED;          		//外部同步信号关闭
  HRTIM_TimerInitStructure.ResetOnSync = HRTIM_SYNCRESET_DISABLED;          		//外部同步信号关闭
  HRTIM_TimerInitStructure.DACSynchro = HRTIM_DACSYNC_NONE;                 		//不用DAC
  HRTIM_TimerInitStructure.PreloadEnable = HRTIM_PRELOAD_ENABLED;           		//预加载使能
  HRTIM_TimerInitStructure.UpdateGating = HRTIM_UPDATEGATING_INDEPENDENT;   		//DMA猝发时独立执行
  HRTIM_TimerInitStructure.BurstMode = HRTIM_TIMERBURSTMODE_MAINTAINCLOCK;  		//不使用Burstmode
  HRTIM_TimerInitStructure.RepetitionUpdate = HRTIM_UPDATEONREPETITION_ENABLED;
  
  HRTIM_BaseInitStructure.Period = PWM_PERIOD; 																	//定义PWM周期，详细见Config.h文件
  HRTIM_BaseInitStructure.RepetitionCounter = 1;                                //多少个周期触发一次中断
  HRTIM_BaseInitStructure.PrescalerRatio = HRTIM_PRESCALERRATIO_MUL32;          //锁相环144MHZ*32=4.6GHZ
  HRTIM_BaseInitStructure.Mode = HRTIM_MODE_CONTINOUS;                          //循环计数
  HRTIM_Waveform_Init(HRTIM1, HRTIM_TIMERINDEX_MASTER, &HRTIM_BaseInitStructure, &HRTIM_TimerInitStructure);	
  HRTIM_Waveform_Init(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, &HRTIM_BaseInitStructure, &HRTIM_TimerInitStructure);
	HRTIM_Waveform_Init(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, &HRTIM_BaseInitStructure, &HRTIM_TimerInitStructure);
  
  /* ------------------------------------------------ */
  /* TIMERA output and registers update configuration */
  /* ------------------------------------------------ */
  HRTIM_TimerWaveStructure.DeadTimeInsertion = HRTIM_TIMDEADTIMEINSERTION_ENABLED;     //使能死区 
  HRTIM_TimerWaveStructure.DelayedProtectionMode = HRTIM_TIMDELAYEDPROTECTION_DISABLED;//关闭延时保护模式
  HRTIM_TimerWaveStructure.FaultEnable = HRTIM_TIMFAULTENABLE_NONE;                    //错误保护模式关闭
  HRTIM_TimerWaveStructure.FaultLock = HRTIM_TIMFAULTLOCK_READWRITE;                   //错误锁定功能关闭
  HRTIM_TimerWaveStructure.PushPull = HRTIM_TIMPUSHPULLMODE_DISABLED;                  //推挽模式关闭
  HRTIM_TimerWaveStructure.ResetTrigger = HRTIM_TIMRESETTRIGGER_MASTER_PER;            //主定时器复位时从定时器也复位
  HRTIM_TimerWaveStructure.ResetUpdate = HRTIM_TIMUPDATEONRESET_ENABLED;               //使能复位更新事件
  HRTIM_TimerWaveStructure.UpdateTrigger = HRTIM_TIMUPDATETRIGGER_MASTER;              //更新触发源，选择注定是其
  HRTIM_WaveformTimerConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, &HRTIM_TimerWaveStructure);
	HRTIM_WaveformTimerConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, &HRTIM_TimerWaveStructure);
  
  HRTIM_TIM_OutputStructure.Polarity = HRTIM_OUTPUTPOLARITY_HIGH;                      //高电平设置为正极性
  HRTIM_TIM_OutputStructure.SetSource = HRTIM_OUTPUTSET_TIMPER;                        //计数器回0是输出拉高
  HRTIM_TIM_OutputStructure.ResetSource = HRTIM_OUTPUTRESET_TIMCMP1;                   //计数到CMP1寄存器值，输出拉低
  HRTIM_TIM_OutputStructure.IdleMode = HRTIM_OUTPUTIDLEMODE_NONE;                      //如果不开启BURST-MODE，这一项建议关闭
  HRTIM_TIM_OutputStructure.IdleState = HRTIM_OUTPUTIDLESTATE_INACTIVE;                //立即执行，关闭输出
  HRTIM_TIM_OutputStructure.FaultState = HRTIM_OUTPUTFAULTSTATE_INACTIVE;              //错位u发生时关闭输出，失能
  HRTIM_TIM_OutputStructure.ChopperModeEnable = HRTIM_OUTPUTCHOPPERMODE_DISABLED;      //斩波模式关闭
  HRTIM_TIM_OutputStructure.BurstModeEntryDelayed = HRTIM_OUTPUTBURSTMODEENTRY_REGULAR;//选择进入BURST-MODE延时
  HRTIM_WaveformOutputConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_OUTPUT_TA1, &HRTIM_TIM_OutputStructure);
	HRTIM_WaveformOutputConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_OUTPUT_TB1, &HRTIM_TIM_OutputStructure);
	
  /* Set compare registers for duty cycle on TA1 */
  HRTIM_CompareStructure.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;            
  HRTIM_CompareStructure.AutoDelayedTimeout = 0;
  HRTIM_CompareStructure.CompareValue =100;     
  HRTIM_WaveformCompareConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, &HRTIM_CompareStructure);
	HRTIM_WaveformCompareConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, &HRTIM_CompareStructure);
	
	//死区时间计算，(1/144Mhz)/8=6.94444ns/8=0.868ns,在分频为0的时候
	HRTIM_TIM_DeadTimeStructure.FallingLock = HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE;                   //HRTIM死区时间寄存器锁定，只有再下次系统复位时才能修改，运行期间只读
  HRTIM_TIM_DeadTimeStructure.FallingSign = HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE;                //死区时间设定为正值
  HRTIM_TIM_DeadTimeStructure.FallingSignLock = HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE;           //锁定之后，只有系统复位才能修改
  HRTIM_TIM_DeadTimeStructure.FallingValue = ((uint16_t)120);                                      //下降沿死区时间
  HRTIM_TIM_DeadTimeStructure.Prescaler = 0x00;                                                    //不分频，CLK=144MHz*8;
  HRTIM_TIM_DeadTimeStructure.RisingLock = HRTIM_TIMDEADTIME_RISINGLOCK_WRITE;                     //HRTIM死区时间寄存器锁定，只有再下次系统复位时才能修改，运行期间只读
  HRTIM_TIM_DeadTimeStructure.RisingSign = HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE;                  //死区时间设定为正值
  HRTIM_TIM_DeadTimeStructure.RisingSignLock = HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE;             //锁定之后，只有系统复位才能修改
  HRTIM_TIM_DeadTimeStructure.RisingValue = ((uint16_t)120);                                       //上升沿死区时间
  HRTIM_DeadTimeConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, &HRTIM_TIM_DeadTimeStructure);            //开始配置定时器A
	HRTIM_DeadTimeConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, &HRTIM_TIM_DeadTimeStructure);            //开始配置定时器B
	
  HRTIM_CompareStructure.AutoDelayedMode = HRTIM_AUTODELAYEDMODE_REGULAR;                          //设置ADC触发模式，立即执行
  HRTIM_CompareStructure.AutoDelayedTimeout = 0;                                                   //延时为零
  HRTIM_CompareStructure.CompareValue = 50;                                                        //ADC采样时刻点
  HRTIM_WaveformCompareConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_3, &HRTIM_CompareStructure);//设定为比较器3触发ADC
	
	HRTIM_ADCTrigStructure.Trigger      = HRTIM_ADCTRIGGEREVENT13_TIMERB_CMP3;                       //选择触发源
  HRTIM_ADCTrigStructure.UpdateSource = HRTIM_ADCTRIGGERUPDATE_TIMER_B;                            //HRTIMA计数回零时可以更新ADC触发来源
  HRTIM_ADCTriggerConfig(HRTIM1, HRTIM_ADCTRIGGER_1, &HRTIM_ADCTrigStructure);                     //开始配置
	
	//非逐周期环路计算时可以采用以下中断，比如设置为2个PWM周期中断一次
	/* Interrupt initialization */       
  /* -------------------------*/
  NVIC_InitStructure.NVIC_IRQChannel 										= HRTIM1_TIMA_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority 				= 1;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd 								= ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  /* TIMER A issues an interrupt on each repetition event */
  HRTIM_ITConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_TIM_IT_REP, ENABLE);
  /* Start HRTIM's TIMER A */
	
	HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = PWM_PERIOD+100;                          //更新HRTIMA（比较寄存器的值）
  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_A].CMP3xR = 50;
 
  HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP1xR = 100;                                     //更新HRTIMB（比较寄存器的值）
	HRTIM1->HRTIM_TIMERx[HRTIM_TIMERINDEX_TIMER_B].CMP3xR = 50;
	
	
	HRTIM_WaveformCounterStart(HRTIM1, HRTIM_TIMERID_MASTER);                                        //启动定时器
  HRTIM_WaveformCounterStart(HRTIM1, HRTIM_TIMERID_TIMER_A);                                       //启动定时器
	HRTIM_WaveformCounterStart(HRTIM1, HRTIM_TIMERID_TIMER_B);                                       //启动定时器
	
	Delay_us(100);
	
  HRTIM_WaveformOutputStart(HRTIM1,HRTIM_OUTPUT_TA1);
	HRTIM_WaveformOutputStart(HRTIM1,HRTIM_OUTPUT_TA2);
	HRTIM_WaveformOutputStart(HRTIM1,HRTIM_OUTPUT_TB1);
	HRTIM_WaveformOutputStart(HRTIM1,HRTIM_OUTPUT_TB2);
	
//非逐周期环路计算时可以采用以下中断，比如设置为2个PWM周期中断一次
	/* Interrupt initialization */       
  /* -------------------------*/
//  NVIC_InitStructure.NVIC_IRQChannel 										= HRTIM1_TIMA_IRQn;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority 				= 0;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 0;
//  NVIC_InitStructure.NVIC_IRQChannelCmd 								= ENABLE;
//  NVIC_Init(&NVIC_InitStructure);
//  /* TIMER A issues an interrupt on each repetition event */
//  HRTIM_ITConfig(HRTIM1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_TIM_IT_REP, ENABLE);
  /* Start HRTIM's TIMER A */
}


