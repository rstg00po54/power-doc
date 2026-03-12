#include "LED.h"
#include "Config.h"
//Config.h文件中有详细IO参数配置
//	PB9   LED(绿色)   1.0A   当前电流1.0A
//	PB8   LED(绿色)   2.0A   当前电流2.0A
//	PB5   LED(绿色)   3.0A   当前电流3.0A
//	PB4   LED(绿色)   4.0A   当前电流4.0A
//	PB3   LED(绿色)   5.0A   当前电流5.0A
//	
//	PA15  LED(红色)   OCP    当前过流
//	PA12  LED(红色)   OVP    当前过压
//	PB15  LED(红色)   UVP    当前过压
//	PB14  LED(红色)   OTP    当前过温
void LED_GPIO_InitConfig(void) //初始化IO口
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC, ENABLE);
  
  GPIO_InitStructure.GPIO_PuPd  =  GPIO_PuPd_NOPULL;  //关闭上下拉功能
  GPIO_InitStructure.GPIO_Speed =  GPIO_Speed_50MHz;  //IO速度设定为50MHz
  GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_OUT;     //选定为输出
  GPIO_InitStructure.GPIO_OType =  GPIO_OType_PP;     //推挽模式
  GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_12|GPIO_Pin_15;  //选定IO口
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化
	
	GPIO_InitStructure.GPIO_Pin   =  GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_14|GPIO_Pin_15;//选定IO口
  GPIO_Init(GPIOB, &GPIO_InitStructure);	//初始化
	
	LED_1A0_OFF;                //关灯
	LED_2A0_OFF;                //关灯
	LED_3A0_OFF;                //关灯
	LED_4A0_OFF;                //关灯
	LED_5A0_OFF;                //关灯
	LED_OCP_OFF;                //关灯
	LED_OVP_OFF;                //关灯
	LED_UVP_OFF;                //关灯
	LED_OTP_OFF;                //关灯
}

void LED_TEST(void)
{
	uint16_t i=100;   
	LED_1A0_ON;                   //点亮1.0A的指示灯
	while(1)
	{
		Delay_ms(i);
		LED_2A0_ON;Delay_ms(i);     //点亮2.0A的指示灯
		LED_3A0_ON;Delay_ms(i);     //点亮3.0A的指示灯
		LED_4A0_ON;Delay_ms(i);     //点亮4.0A的指示灯
		LED_5A0_ON;Delay_ms(i);     //点亮5.0A的指示灯
		LED_OCP_ON;Delay_ms(i);     //点亮OCP 的指示灯
		LED_OVP_ON;Delay_ms(i);     //点亮OVP 的指示灯
		LED_UVP_ON;Delay_ms(i);     //点亮UVP 的指示灯
		LED_OTP_ON;Delay_ms(i);     //点亮OTP 的指示灯
		
		LED_2A0_OFF;                //关灯
		LED_3A0_OFF;                //关灯
		LED_4A0_OFF;                //关灯
		LED_5A0_OFF;                //关灯
		LED_OCP_OFF;                //关灯
		LED_OVP_OFF;                //关灯
		LED_UVP_OFF;                //关灯
		LED_OTP_OFF;                //关灯
		break;
	}
}
