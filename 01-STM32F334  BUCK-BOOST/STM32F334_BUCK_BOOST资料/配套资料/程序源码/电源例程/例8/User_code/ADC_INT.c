/****************************************************************************************
  * @file    ADC_INT.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/


#include "ADC_INT.h"
#include "init.h"


__IO uint16_t  ADC1_RESULT[2]={0};
//ADC1_RESULT[0]保存输出电压
//ADC1_RESULT[1]保存输出电流
__IO uint16_t  ADC2_RESULT[3]={0};
//ADC2_RESULT[0]保存输入电压
//ADC2_RESULT[1]保存输入电流
//ADC2_RESULT[2]保存温度信息
__IO uint16_t calibration_value = 0;
//保存ADC矫正值，可以不用

void ADC_Config(void)
{
	ADC1_DMA_Config();
	ADC1_Config();         //初始化ADC1，启用DMA：
  ADC2_DMA_Config();
	ADC2_Config();         //初始化ADC2，启用DMA：
}
/*****************************************************ADC初始化********************************************************************/
//  PIN_10 PA0   ADC1_IN1   Vout_detc
//	PIN_11 PA1   ADC1_IN2   CS_out_detc
//	PIN_19 PB1   ADC1_IN12  Vin_detc 

//	PIN_20 PB2   ADC2_IN12  CS_in_detc 
//	PIN_25 PB12  ADC2_IN13  Temp_detc
//	PIN_27 PB14  ADC2_IN14  2V5_Ref
void ADC1_Config(void)                                    
{
	GPIO_InitTypeDef   GPIO_InitStructure;
  ADC_InitTypeDef    ADC_InitStructure;

  ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_DeInit(ADC1);                                      					 						//把ADC复位到初始状态

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);							 						//使能GPIOA时钟  
  RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div1);													 						//PCLK=HCLK=72MHz,ADCCLK=72MHz
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);							 						//使能ADC12时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;                        //IO口配置
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;                               //模拟输入
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;                              //不起作用
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
	
  ADC_StructInit(&ADC_InitStructure);                                         
  
  ADC_VoltageRegulatorCmd(ADC1, ENABLE);
  Delay_ms(10);
  
  ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);
  ADC_StartCalibration(ADC1);
  
  while(ADC_GetCalibrationStatus(ADC1) != RESET );
  calibration_value = ADC_GetCalibrationValue(ADC1);														//使用ADC之前先矫正 
  
  /* Configure the ADC2 in continuous mode */  
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                                                                    
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;                  
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;             
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_OneShot;                  
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;          
  ADC_CommonInit(ADC1, &ADC_CommonInitStructure);
  
  ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;			//连续转换
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 												//ADC精度12位
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_7;    //ADC外部触发启动转换：关闭    
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_RisingEdge; //ADC外部触发启动转换：关闭 
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;												//采集的数据右对齐
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;   
  ADC_InitStructure.ADC_NbrOfRegChannel = 2;																		//扫描通道数量1个
  ADC_Init(ADC1, &ADC_InitStructure);																						//初始化
  
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1,ADC_SampleTime_19Cycles5 );	  //开启通道1，采样率2.25M/S
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2,ADC_SampleTime_19Cycles5 );	  //开启通道1，采样率2.25M/S

  ADC_Cmd(ADC1, ENABLE);																												//ADC  Enable
  while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY)); 																//等待ADC 就绪
	ADC_DMACmd(ADC1, ENABLE);																										  //使能DMA
	ADC_DMAConfig(ADC1, ADC_DMAMode_Circular);
  ADC_StartConversion(ADC1);																										//采集开始
}
void ADC1_DMA_Config(void)
{
 	DMA_InitTypeDef    DMA_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);                            //使能DMA1时钟
//	NVIC_InitTypeDef NVIC_InitStructure; 
	DMA_DeInit(DMA1_Channel1);                                                     //DMA1复位到默认值
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); 

//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn; 
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
//	NVIC_Init(&NVIC_InitStructure);
  
                                                     
  DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t)&(ADC1->DR);               //DMA连接到AD1的数据寄存器
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC1_RESULT;                  //DMA搬运的数据存放在ADC_RESULT数组里边
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                             //从外设读数据
  DMA_InitStructure.DMA_BufferSize = 2;                                          //告诉DMA要去外设读取数据的个数
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;               //我们只取ADC外设数据，所以不需要外设地址自动增加功能
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                        //存放完ADC_RESULT[0]，再去存ADC_RESULT[1].......
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;    //一次读取16bit
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;            //一次存储16bit
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                //DMA连续不断搬数据到内存，该过程不需要CPU参与
  DMA_InitStructure.DMA_Priority =DMA_Priority_VeryHigh;                         //优先级设定为高
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                   //DMA内存互访关闭
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  DMA_Cmd(DMA1_Channel1, ENABLE);                                                //使能DMA 
	DMA_ITConfig(DMA1_Channel1,DMA_IT_TC,ENABLE); //传输结束中断
  ADC_DMACmd(ADC1, ENABLE);	
	Delay_ms(10);
}
/*****************************************************ADC初始化********************************************************************/
void ADC2_Config(void)                                    
{
	GPIO_InitTypeDef   GPIO_InitStructure;
  ADC_InitTypeDef    ADC_InitStructure;

  ADC_CommonInitTypeDef ADC_CommonInitStructure;

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);							 						//使能GPIOA时钟  
  RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div1);													 						//PCLK=HCLK=72MHz,ADCCLK=72MHz
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);							 						//使能ADC12时钟
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  ADC_StructInit(&ADC_InitStructure);
  
  ADC_VoltageRegulatorCmd(ADC2, ENABLE);
  Delay_ms(10);
  
  ADC_SelectCalibrationMode(ADC2, ADC_CalibrationMode_Single);
  ADC_StartCalibration(ADC2);
  
  while(ADC_GetCalibrationStatus(ADC2) != RESET );
  calibration_value = ADC_GetCalibrationValue(ADC2);														//使用ADC之前先矫正 
  
  /* Configure the ADC2 in continuous mode */  
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                                                                    
  ADC_CommonInitStructure.ADC_Clock = ADC_Clock_AsynClkMode;                  
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;             
  ADC_CommonInitStructure.ADC_DMAMode = ADC_DMAMode_Circular;                  
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = 0;          
  ADC_CommonInit(ADC2, &ADC_CommonInitStructure);
  
	ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Disable;			//连续转换
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 												//ADC精度12位
  ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_7;    //ADC外部触发启动转换：关闭    
  ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_RisingEdge; //ADC外部触发启动转换：关闭 
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;												//采集的数据右对齐
  ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
  ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;   
  ADC_InitStructure.ADC_NbrOfRegChannel = 3;																		//扫描通道数量1个
  ADC_Init(ADC2, &ADC_InitStructure);																						//初始化
  
  ADC_RegularChannelConfig(ADC2, ADC_Channel_2, 1, ADC_SampleTime_61Cycles5);	  //开启通道1，采样率1M/S
  ADC_RegularChannelConfig(ADC2, ADC_Channel_4, 2, ADC_SampleTime_61Cycles5 );	//开启通道2，采样率1M/S
	ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 3, ADC_SampleTime_61Cycles5 );	//开启通道3，采样率1M/S

  ADC_Cmd(ADC2, ENABLE);																												//ADC  Enable
  while(!ADC_GetFlagStatus(ADC2, ADC_FLAG_RDY)); 																//等待ADC 就绪
  ADC_DMACmd(ADC2, ENABLE);																											//使能DMA
  ADC_DMAConfig(ADC2, ADC_DMAMode_Circular);
  ADC_StartConversion(ADC2);																										//采集开始

}
void ADC2_DMA_Config(void)
{
	DMA_InitTypeDef    DMA_InitStructure;
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);                            //使能DMA1时钟
  
  //DMA_DeInit(DMA1_Channel2);                                                   //DMA1复位到默认值
  DMA_InitStructure.DMA_PeripheralBaseAddr =ADC2_DR_ADDRESS;                     //DMA连接到AD1的数据寄存器
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC2_RESULT;                  //DMA搬运的数据存放在ADC_RESULT数组里边
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                             //从外设读数据
  DMA_InitStructure.DMA_BufferSize = 3;                                          //告诉DMA要去外设读取数据的个数
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;               //我们只取ADC外设数据，所以不需要外设地址自动增加功能
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                        //存放完ADC_RESULT[0]，再去存ADC_RESULT[1].......
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;    //一次读取16bit
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;            //一次存储16bit
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                                //DMA连续不断搬数据到内存，该过程不需要CPU参与
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;                            //优先级设定为高
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                                   //DMA内存互访关闭
  DMA_Init(DMA1_Channel2, &DMA_InitStructure);
  DMA_Cmd(DMA1_Channel2, ENABLE);                                                //使能DMA 
	ADC_DMAConfig(ADC2, ADC_DMAMode_Circular);
	ADC_DMACmd(ADC2, ENABLE);		
	Delay_ms(10);
}
