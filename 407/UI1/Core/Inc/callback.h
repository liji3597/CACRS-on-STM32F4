/**
  ******************************************************************************
  *	@file  	 	callback.h
  *	@version 	V1.0
  * @date    	2021-06-01
  *	@author  	知行科技
  *	@brief   	串口、外部中断全局变量以及RAM、LCD初始化函数
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
#ifndef _CALLBACK_H
#define _CALLBACK_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "tim.h" 
#include "adc.h"
#include "spi.h"

#include "sys.h"
#include "delay.h"
#include "stdio.h"

//#include "lcd_rgb.h"
#include "ads1292.h"
//#include "UI.h"

/* Private defines -----------------------------------------------------------*/
#define RXBUFFERSIZE   	1 		//缓存大小
#define USART_REC_LEN   200  	//定义最大接收字节数 200



#define SDRAM_Size 0x02000000  //32M字节
#define SDRAM_BANK_ADDR     ((uint32_t)0xD0000000) 				// FMC SDRAM 数据基地址
#define FMC_COMMAND_TARGET_BANK   FMC_SDRAM_CMD_TARGET_BANK2	//	SDRAM 的bank选择
#define SDRAM_TIMEOUT     ((uint32_t)0x1000) 						// 超时判断时间

#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000) 
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200) 

/* Exported constants --------------------------------------------------------*/
extern uint16_t USART1_RX_STA;         			//接收状态标记	
extern uint8_t  USART1_RxBuffer[RXBUFFERSIZE];	//HAL库USART接收Buffer
extern uint8_t  USART1_RX_BUF[USART_REC_LEN];	//接收缓冲,最大USART_REC_LEN个字节.末字节为换行符  

extern uint8_t usart2_txbuf[2700];
extern uint8_t usart2_rxbuf[512];
extern uint8_t usart2_rxone[1];
extern uint8_t usart2_rxcounter;

extern uint8_t key;
extern uint32_t lcd_colour;
extern __IO uint32_t sys_tick;	//系统时基：每1ms自增1
extern	volatile u8 ads1292_Cache[9];		//ads1292数据缓存
extern  volatile u8 ads1292_recive_flag;	//数据读取完成标志

extern float Input_data2; 					// 输入缓冲区
extern float Output_data2;         // 输出缓冲区
extern uint8_t heart_rate;

extern uint32_t ecg_temp_buf_store_count;	//TCP数据缓存时使用的数组下标
extern int32_t ecg_temp_buf[600];	//用于发送数据至服务器的缓存区

//extern osc_window ecg_win;
extern uint32_t ecg_x_value;	//ecg示波器横轴变量
//extern FMC_SDRAM_CommandTypeDef command;	// 控制指令

extern uint32_t ADCConvertedValue;	//ADC DMA  buffer

extern float temperature;
extern unsigned int RunningSteps;    			//步数

/* Exported functions prototypes ---------------------------------------------*/

//void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);
extern float ADC_GetTemp(void);
u16 Get_Adc(void);
extern u16 Get_Adc_Average(u32 ch,u8 times);	// 得到某个通道给定次数采样的平均值

void SPI3_SetSpeed(u8 SPI_BaudRatePrescaler);
u8 SPI3_ReadWrite_Byte(u8 dat);
void SPI3_Send_Byte(u8 dat);
u8 SPI3_Read_Byte(void);

#endif
