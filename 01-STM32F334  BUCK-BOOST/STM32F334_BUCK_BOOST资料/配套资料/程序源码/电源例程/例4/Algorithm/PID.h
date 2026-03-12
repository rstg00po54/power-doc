/****************************************************************************************
  * @file    PID.h
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/


#ifndef __PID_H__
#define __PID_H__

#include "init.h"

 void  PID_INT(void);
 void PID_Incremental(uint16_t Vout_actual);        //增量型PID





#endif
