/****************************************************************************************
  * @file    Config.h
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#ifndef __CONFIG_H__ 
#define __CONFIG_H__
/********************************************************************
例1 ：同步BUCK整流器 ，     输入:DC16V-36V  输出:12V/5A，控制算法：Type3       模式：恒压        晶体管：Q1,Q2交替导通，Q4导通，Q3关断         
例2 ：同步BUCK整流器 ，     输入:DC16V-36V  输出:12V/5A，控制算法：Type2       模式：恒压        晶体管：Q1,Q2交替导通，Q4导通，Q3关断        
例3 ：同步BUCK整流器 ，     输入:DC16V-36V  输出:12V/5A，控制算法：零极点匹配  模式：恒压        晶体管：Q1,Q2交替导通，Q4导通，Q3关断        
例4 ：同步BUCK整流器 ，     输入:DC16V-36V  输出:12V/5A，控制算法：PID         模式：恒压        晶体管：Q1,Q2交替导通，Q4导通，Q3关断       
例5 ：同步BUCK整流器 ，     输入:DC16V-36V  输出:12V/5A，控制算法：PID         模式：恒压恒流    晶体管：Q1,Q2交替导通，Q4导通，Q3关断        
例6 ：同步BUCK整流器 ，     输入:DC16V-36V  输出:12V/5A，控制算法：Type3       模式：PC程控调压  晶体管：Q1,Q2交替导通，Q4导通，Q3关断         
例7 ：同步BOOST整流器，     输入:DC12V-24V  输出:24V/5A，控制算法：Type3       模式：恒压        晶体管：Q3,Q4交替导通，Q1导通，Q2关断         
例8 ：同步BOOST整流器，     输入:DC12V-24V  输出:24V/5A，控制算法：PID         模式：恒压        晶体管：Q3,Q4交替导通，Q1导通，Q2关断       
例9 ：同步BUCK-BOOST整流器，输入:DC12V-36V  输出:24V/5A，控制算法：Type3       模式：恒压        晶体管：Q1Q3，Q2Q4交替导通                  
例10：同步BUCK-BOOST整流器，输入:DC12V-36V  输出:24V/5A，控制算法：Type3       模式：恒压        晶体管：BUCK，BOOST,BUCK-BOOST自动无缝切换  
补充说明：（例10）输入12-22V时工作在BOOST模式，22-26V时工作在BUCK-BOOST模式，26-36V时工作在BUCK模式,模式自动无缝切换
********************************************************************/     
/**********************宏定义*************************************/
typedef enum {STOP = 0, START = !STOP} DP_FlagStatus;
/*************************PWM参数配置********************************/
//#define KHz               (1000)
//#define MHz               (1000*KHz)
//#define F_HRTIM           4608*MHz

//#define PWM_FREQUENCY     200*KHz
//#define PWM_PERIOD       (F_HRTIM/PWM_FREQUENCY)            // 200KHZ,23040

#define PWM_PERIOD          23040.0f                          // 200KHZ,23040

/******************************IO口配置***************************************
LED状态指示IO配置
	PB9   LED(绿色)   1.0A   当前电流1.0A
	PB8   LED(绿色)   2.0A   当前电流2.0A
	PB5   LED(绿色)   3.0A   当前电流3.0A
	PB4   LED(绿色)   4.0A   当前电流4.0A
	PB3   LED(绿色)   5.0A   当前电流5.0A
	
	PA15  LED(红色)   OCP    当前过流
	PA12  LED(红色)   OVP    当前过压
	PB15  LED(红色)   UVP    当前过压
	PB14  LED(红色)   OTP    当前过温

串口通信IO配置
	PB6   USART1_TX  串口接发送
	PB7   USART1_RX  串口发接收
	
按键检测IO配置
	PA2  按键 UP
	PA3  按键 Down
	
PWM输出IO配置
	PA8   HRTIM_CHA1  驱动MOS管Q1
	PA9   HRTIM_CHA2  驱动MOS管Q2
	PA10  HRTIM_CHB1  驱动MOS管Q4
	PA11  HRTIM_CHB2  驱动MOS管Q3
	
ADC检测IO配置
  PA0   ADC1_IN1   VOUT_DETC
	PA1   ADC1_IN2   CS_OUT_DETC

	PA5   ADC2_IN2  VIN_DETC
	PA6   ADC2_IN3  TEMP_DETC
	PA7   ADC2_IN4  CS_IN_DETC

DAC IO配置
	PA4   DAC1_OUT1 

I2C通讯接口，读取EEPROM
	PB13      SCL
	PB12      SDA

**********************************************/

#endif
