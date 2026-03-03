#include "DAC_INT.h"


__IO uint16_t calibration_value_DAC = 0;
 void DAC_Config(void)
{
  DAC_InitTypeDef    DAC_InitStructure;
  GPIO_InitTypeDef   GPIO_InitStructure;

  /* Enable GPIOA clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  
  //PA4  DAC1_HC1输出
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	DAC_DeInit(DAC1); 
  /* Enable DAC clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
  
  /* DAC1 channel1 Configuration */
  DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
  DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
  DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bits2_0;//这个参数只有波形发生器位使能的时候有作用。
  DAC_InitStructure.DAC_Buffer_Switch = DAC_BufferSwitch_Enable;
  DAC_Init(DAC1, DAC_Channel_1, &DAC_InitStructure);
  
  /* Enable DAC1 Channel1 */
  DAC_Cmd(DAC1, DAC_Channel_1, ENABLE);
	
	Delay_us(100);
	DAC_SetChannel1Data(DAC1, DAC_Align_12b_R, 4000);//设定DAC输出电压3.2V，对应7.75A,逐周期峰值过流保护
}

//    /* Output converted value on DAC1_OUT1 */
//    DAC_SetChannel1Data(DAC1, DAC_Align_12b_R, ADCVal);

