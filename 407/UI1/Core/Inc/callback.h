/**
  ******************************************************************************
  *	@file  	 	callback.h
  *	@version 	V1.0
  * @date    	2021-06-01
  *	@author  	֪�пƼ�
  *	@brief   	���ڡ��ⲿ�ж�ȫ�ֱ����Լ�RAM��LCD��ʼ������
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
#define RXBUFFERSIZE   	1 		//�����С
#define USART_REC_LEN   200  	//�����������ֽ��� 200



#define SDRAM_Size 0x02000000  //32M�ֽ�
#define SDRAM_BANK_ADDR     ((uint32_t)0xD0000000) 				// FMC SDRAM ���ݻ���ַ
#define FMC_COMMAND_TARGET_BANK   FMC_SDRAM_CMD_TARGET_BANK2	//	SDRAM ��bankѡ��
#define SDRAM_TIMEOUT     ((uint32_t)0x1000) 						// ��ʱ�ж�ʱ��

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
extern uint16_t USART1_RX_STA;         			//����״̬���	
extern uint8_t  USART1_RxBuffer[RXBUFFERSIZE];	//HAL��USART����Buffer
extern uint8_t  USART1_RX_BUF[USART_REC_LEN];	//���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з�  

extern uint8_t usart2_txbuf[2700];
extern uint8_t usart2_rxbuf[512];
extern uint8_t usart2_rxone[1];
extern uint8_t usart2_rxcounter;

extern uint8_t key;
extern uint32_t lcd_colour;
extern __IO uint32_t sys_tick;	//ϵͳʱ����ÿ1ms����1
extern	volatile u8 ads1292_Cache[9];		//ads1292���ݻ���
extern  volatile u8 ads1292_recive_flag;	//���ݶ�ȡ��ɱ�־

extern float Input_data2; 					// ���뻺����
extern float Output_data2;         // ���������
extern uint8_t heart_rate;

extern uint32_t ecg_temp_buf_store_count;	//TCP���ݻ���ʱʹ�õ������±�
extern int32_t ecg_temp_buf[600];	//���ڷ����������������Ļ�����

//extern osc_window ecg_win;
extern uint32_t ecg_x_value;	//ecgʾ�����������
//extern FMC_SDRAM_CommandTypeDef command;	// ����ָ��

extern uint32_t ADCConvertedValue;	//ADC DMA  buffer

extern float temperature;
extern unsigned int RunningSteps;    			//����

/* Exported functions prototypes ---------------------------------------------*/

//void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);
extern float ADC_GetTemp(void);
u16 Get_Adc(void);
extern u16 Get_Adc_Average(u32 ch,u8 times);	// �õ�ĳ��ͨ����������������ƽ��ֵ

void SPI3_SetSpeed(u8 SPI_BaudRatePrescaler);
u8 SPI3_ReadWrite_Byte(u8 dat);
void SPI3_Send_Byte(u8 dat);
u8 SPI3_Read_Byte(void);

#endif
