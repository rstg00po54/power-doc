#include "COMP2.h"

 void COMP2_Config(void)
{
  /* Init Structure definition */
  COMP_InitTypeDef        COMP_InitStructure;
  GPIO_InitTypeDef        GPIO_InitStructure;
   
  /* COMP Peripheral clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	

	//PA7作为COMP2同向输入端，对应开发板输入电感电流
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* COMP2 config */
  COMP_InitStructure.COMP_InvertingInput = COMP_InvertingInput_DAC1OUT1;    //DAC_CH1接到比较器2的反相输入端
  COMP_InitStructure.COMP_NonInvertingInput =  COMP_NonInvertingInput_IO1;  //和PA7连接在一起
  COMP_InitStructure.COMP_Output = COMP_Output_HRTIM1_EE6_2;                //比较器的输出作为外部事件6
  COMP_InitStructure.COMP_OutputPol = COMP_OutputPol_NonInverted;           //正极性
  COMP_InitStructure.COMP_BlankingSrce = COMP_BlankingSrce_None;            //Blank时间
  COMP_InitStructure.COMP_Hysteresis = COMP_Hysteresis_High;
  COMP_InitStructure.COMP_Mode = COMP_Mode_HighSpeed;
  COMP_Init(COMP_Selection_COMP2, &COMP_InitStructure);
  
  /* Enable COMP2 */
  COMP_Cmd(COMP_Selection_COMP2, ENABLE);
}
