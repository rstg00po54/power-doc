/****************************************************************************************
  * @file    init.c
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/
#include "init.h"

void Initial_prepheral_(void)
{
	Init_TIM16();          //初始化定时器16，做延时函数使用
	ADC_Config();          //初始化ADC
	COMP2_Config();        //初始化比较器2
	DAC_Config();          //初始化DAC1_CH1
	HRTIM_Config();        //初始化HRTIM，fHRCK: 4.608 GHz - Resolution: 217 ps
	GPIO_HRTIM_Config();   //初始化HRTIM输出IO口
	USART1_Config();       //初始化串口模块2,和PC通讯，发送电压电流
	LED_GPIO_InitConfig(); //LED控制IO口初始化
	KEY_GPIO_InitConfig();
}
























//按键检测IO配置，详细见Config.h
//	PA2  按键 UP
//	PA3  按键 Down
void KEY_GPIO_InitConfig(void) //初始化KEY_IO口
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
  
  GPIO_InitStructure.GPIO_PuPd  =  GPIO_PuPd_NOPULL;  //关闭上下拉功能
  GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_50MHz;  //IO速度设定为50MHz
  GPIO_InitStructure.GPIO_OType =  GPIO_OType_PP;     //推完模式,
	GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_IN;      //选定为输入
	GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_2|GPIO_Pin_3;//选定IO口
	GPIO_Init(GPIOA, &GPIO_InitStructure);	//初始化
	
}






__IO uint8_t    PC_command[6] 	 ={0};
__IO uint8_t    Send_to_PC[14]   ={0};

void USART1_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef  USART_InitStruct;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB , ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 ,ENABLE);
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
		
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6,GPIO_AF_7);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7,GPIO_AF_7);
	
  USART_DeInit(USART1);
	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	
	USART_Init(USART1,&USART_InitStruct);  

  USART_ClearFlag(USART1, USART_FLAG_TC); /* 清发送外城标志，Transmission Complete flag */
	
  /* DMA1 Channel6   RX  */
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->RDR);    // 外设地址"顺丰快递”  
  DMA_InitStructure.DMA_MemoryBaseAddr =(uint32_t)&PC_command;						// 内存地址"  丰巢"
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;         							// 顺丰快递->蜂巢(收件) ，PC到MCU
  DMA_InitStructure.DMA_BufferSize = 6 ;                     							// 缓存容量，蜂巢柜子大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 				// 外设地址不递增
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;    							// 内存递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设字节宽度 
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					// 内存字节宽度
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;            							// 循环存储
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;      						    // 优先级很高，对应快递就是加急
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;               							// 内存与外设通信，而非内存到内存 
  DMA_Init(DMA1_Channel5, &DMA_InitStructure);               							// 把参数初始化

	DMA_Cmd(DMA1_Channel5, ENABLE);                            							// 启动DMA，
  USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);					                // 使能DMA串口发送和接受请求
	
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(USART1->TDR);    // 外设地址"顺丰快递”
  DMA_InitStructure.DMA_MemoryBaseAddr =(uint32_t)&Send_to_PC[0];					// 内存地址"  丰巢"
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;         							// "丰巢"->"顺丰快递"（寄件）,MCU到PC
	DMA_InitStructure.DMA_BufferSize = 14 ;                     						// 缓存容量，蜂巢柜子大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 				// 外设地址不递增
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;    							// 内存递增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设字节宽度 
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					// 内存字节宽度
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;            							  // 循环存储
  DMA_InitStructure.DMA_Priority = DMA_Priority_Low;      						  	// 优先级很高，对应快递就是加急
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;               							// 内存与外设通信，而非内存到内存 
  DMA_Init(DMA1_Channel4, &DMA_InitStructure);               							// 把参数初始化

	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);					                // 使能DMA串口发送和接受请求
	
	USART_Cmd(USART1, ENABLE);
}

void Send_data_to_PC(uint8_t no_pc)
{
  DMA_SetCurrDataCounter(DMA1_Channel4, no_pc);                           //设置发送多少个数据到PC 
  DMA_Cmd(DMA1_Channel4, ENABLE);                          						    // 启动DMA，开始发送
}
