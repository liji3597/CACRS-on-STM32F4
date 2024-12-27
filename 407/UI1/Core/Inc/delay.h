/**
  ******************************************************************************
  *	@file  	 	delay.h
  *	@version 	V1.0
  * @date    	2021-06-01
  *	@author  	知行科技
  *	@brief   	使用SysTick的普通计数模式对延迟进行管理
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/  
#ifndef _DELAY_H
#define _DELAY_H

/* Includes ------------------------------------------------------------------*/
#include <sys.h>

/* Exported functions prototypes ---------------------------------------------*/
void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);
#endif

