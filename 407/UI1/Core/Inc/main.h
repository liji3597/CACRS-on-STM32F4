/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ESP8266_CH_PD_PIN_Pin GPIO_PIN_2
#define ESP8266_CH_PD_PIN_GPIO_Port GPIOE
#define DHT11_CLK_Pin GPIO_PIN_3
#define DHT11_CLK_GPIO_Port GPIOE
#define KEY_0_Pin GPIO_PIN_13
#define KEY_0_GPIO_Port GPIOC
#define KEY_0_EXTI_IRQn EXTI15_10_IRQn
#define LED_R_Pin GPIO_PIN_6
#define LED_R_GPIO_Port GPIOF
#define LED_G_Pin GPIO_PIN_7
#define LED_G_GPIO_Port GPIOF
#define LED_B_Pin GPIO_PIN_8
#define LED_B_GPIO_Port GPIOF
#define AD1292_PW_Pin GPIO_PIN_9
#define AD1292_PW_GPIO_Port GPIOE
#define AD1292_ST_Pin GPIO_PIN_10
#define AD1292_ST_GPIO_Port GPIOE
#define AD1292_CS_Pin GPIO_PIN_12
#define AD1292_CS_GPIO_Port GPIOE
#define AD1292_DRDY_Pin GPIO_PIN_8
#define AD1292_DRDY_GPIO_Port GPIOD
#define AD1292_DRDY_EXTI_IRQn EXTI9_5_IRQn
#define LTDC_Black_Pin GPIO_PIN_13
#define LTDC_Black_GPIO_Port GPIOD
#define ESP8266_RST_PIN_Pin GPIO_PIN_15
#define ESP8266_RST_PIN_GPIO_Port GPIOG
#define IIC_CL_Pin GPIO_PIN_4
#define IIC_CL_GPIO_Port GPIOB
#define IIC_DA_Pin GPIO_PIN_6
#define IIC_DA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
