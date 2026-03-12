/****************************************************************************************
  * @file    type3.h
  * @author  尖叫的变压器
  * @version V1.0.0
  * @date    01-July-2019
  * @淘宝店铺链接：https://shop239216898.taobao.com/?spm=2013.1.1000126.d21.5e571852VY9erz
  * @LegalDeclaration ：本文档内容难免存在Bug，仅限于交流学习，禁止用于任何的商业用途
	* @Copyright   著作权归数字电源开源社区所有
*****************************************************************************************/

#ifndef __type3__H__
#define __type3__H__

#include "init.h"

#define pF  (1e-12)
#define nF  (1e-9)
#define uF  (1e-6)

#define om   1.0
#define Kom  1000.0
#define Mom  1000000.0


#define nS   (1e-9)
#define uS   (1e-6)
#define ms   (1e-3)

void type_3_int(void);
void type_3_cal(uint16_t vout_temp);//TYPE3算法
void type_3_tustin(void);

#endif
