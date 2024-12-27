/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma2d.h"
#include "ltdc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "callback.h"
#include "lcd_rgb.h"
#include "esp8266_at.h"
#include "esp8266_mqtt.h"
#include "dht11.h"
#include "ads1292.h"
#include "UI.h"
#include "ecg_fir.h"
#include "mpu6050.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//�����ĵ�ͼ����
//osc_window ecg_win;
//uint8_t heart_rate;
extern osc_window ecg_win;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void pedometer_step(unsigned long* unStepCountTmp,unsigned int* bushu){
//	char g,a;
	float pitch,row,yaw;
//	short gx,gy,gz;
//	short ax,ay,az;
//	sprintf(&g,"Gyroscope:x=%hd y=%hd z=%hd",gx,gy,gz);
//	sprintf(&a,"Accelerometer:x=%hd y=%hd z=%hd",ax,ay,az);
	LCD_SetTextFont(&CH_Font20);					// ����2424��������,ASCII�����ӦΪ2412
	mpu_dmp_get_data(&pitch,&row,&yaw);
	LCD_DisplayDecimals(700,400,pitch,5,2); 			//��ʾ�������� 
	LCD_DisplayDecimals(700,425,row,5,2); 			//��ʾ�������� 
	LCD_DisplayDecimals(700,450,yaw,5,2);
//	MPU_Get_Gyroscope(&gx,&gy,&gz);
//	MPU_Get_Accelerometer(&ax,&ay,&az);
//	LCD_ShowString(30,180,200,16,16,&g);
//	LCD_ShowString(30,200,200,16,16,&a);
		LCD_SetTextFont(&CH_Font32);					// ����2424��������,ASCII�����ӦΪ2412
		if(!dmp_get_pedometer_step_count(unStepCountTmp))		//�õ�����    ����0��ʾ�������óɹ�
		{	
			if(*unStepCountTmp > 0) 				
			{
				*bushu = *unStepCountTmp;        	//��Ų���
				LCD_DisplayNumber(700,350,*bushu,3); 			//��ʾ��������                
			}
		}
}

//LMT70�������׷��̶�Ӧ�ĸ�����
const double a= -1.809628E-09;
const double b= -3.325395E-06;
const double c= -1.814103E-01;
const double d=  2.050894E+02;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
/*ʱ��*/
	uint32_t lmt70_tick = 0 ;	//LMT70��������
	uint32_t step_tick = 0;		//������ʾʱ��
	uint32_t Report_tick = 0;
//	int ecg_temp_buf_tx_count = 0;
//u16 EGC_i;	
/*����*/	
	uint8_t len;					//���ڽ������ݳ���
	
//	int resss[3];
/*ADC�ɼ�*/	
	u16 adcx;
	u16 Value;
	
	u16 Temper_buf[200];
	float T_sum;	//�¶���ƽ��ʱ����ʱ����
	float T_avr;		//��ƽ���Ľ��
	uint8_t T_sum_count;	//��ƽ��ʱ���ۼӼ���	
	
//	float temp;
	
//	uint32_t ecg_x_value = 0;	//ecgʾ�����������
	
	unsigned long unStepCountTmp=1;

	
	float pitch,row,yaw;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_FMC_Init();
  MX_LTDC_Init();
  MX_DMA2D_Init();
  MX_ADC2_Init();
  MX_SPI5_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

	delay_init(180);                //��ʼ����ʱ����

	HAL_UART_Receive_IT(&huart1, (u8 *)USART1_RxBuffer, RXBUFFERSIZE);//�ú����Ὺ�������ж�
	HAL_UART_Receive_IT(&huart2,usart2_rxone,1);			//��USART1�жϣ����ն�����Ϣ
	HAL_TIM_Base_Start_IT(&htim2);
//	HAL_TIM_Base_Start_IT(&htim3);							  	      //ʹ�ܶ�ʱ��3�Ͷ�ʱ��3�����жϣ�TIM_IT_UPDATE��������Ϊ500ms
	SDRAM_Initialization_Sequence(&hsdram2,&command);				  //����SDRAM
	__HAL_SPI_ENABLE(&hspi5);                                // ʹ��SPI5
//	HAL_ADC_Start_IT(&hadc2);
//	HAL_ADC_Start_DMA(&hadc2,&ADCConvertedValue,1);
//	
	LCD_Init();						//����RGB-LCD	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  	
//	LCD_Test();						//RGB��Ļ����
	LCD_SetBackColor(LCD_BLACK); 			//	���ñ���ɫ
	LCD_Clear(); 											// ����
	LCD_SetTextFont(&CH_Font32);					// ����3232��������,ASCII�����ӦΪ
	LCD_SetColor(LCD_WHITE);						// ���û���,��ɫ
	LCD_DrawRect(0,0,799,38);
	LCD_DrawRect(606,40,193,109);
	LCD_DrawRect(606,150,193,109);
	LCD_DrawRect(606,260,193,109);
	LCD_DrawRect(606,370,193,109);
	LCD_DisplayText(190, 2,"�����˶����ݲɼ��������ڵ�");	// ��ʾ����
		
	LCD_SetColor(LCD_GREEN);						// ���û��ʣ���ɫ			
	LCD_DisplayText(610,  55,"���ʣ�");
	LCD_SetColor(LIGHT_YELLOW);					// ���û��ʣ�����ɫ		
	LCD_DisplayText(610, 165,"�¶ȣ�");	
	LCD_SetColor(LIGHT_CYAN);						// ���û��ʣ�������ɫ
	LCD_DisplayText(610, 275,"������");
	LCD_SetColor(LIGHT_GREY);						// ���û��ʣ���ɫ
	LCD_DisplayText(610, 385,"��̬��");
	
	ECG_UI_init(&ecg_win,0,40,604,439);		//ʾ������ʼ��
	arm_fir_init();
	
	MPU_Init();          //mpu6050��ʼ��
/*��ʼ��6050*/
	while(mpu_dmp_init())  	
	{
		LCD_DisplayString(30,130,"MPU6050 Error");
		delay_ms(200);
//		LCD_Fill(30,130,239,130+16,WHITE);						
		delay_ms(200);
	}
//	LCD_DisplayString(30,130,200,16,16,(u8*)"mpu6050 ok"); 		
	dmp_set_pedometer_step_count(0);    
	/*��ʾ�����ǣ�����*/		
//	LCD_DisplayString(30,160,200,16,16,"pitch:   ");	 
//	LCD_DisplayString(30,180,200,16,16,"row  :   ");
//	LCD_DisplayString(30,200,200,16,16,"yaw  :   ");
//	LCD_DisplayString(30,220,200,16,16,"step :   ");			
	
	ES8266_MQTT_Init();
	StatusReport();
		
	ADS1292_PowerOnInit();						 // ADS1292�ϵ��ʼ��
	while(Set_ADS1292_Collect(0))//0 �����ɼ�  //1 1mV1Hz�ڲ������ź� //2 �ڲ��̽���������
	{	
			printf("1292�Ĵ�������ʧ��\r\n");
			delay_ms(500);
			LED1=!LED1;
	}

	Value = Get_Adc();
	
  while (1)
  {
	  
/*����*/
	if(USART1_RX_STA&0x8000)
	{					   
		len=USART1_RX_STA&0x3fff;		//�õ��˴ν��յ������ݳ���
		printf("\r\n�����͵���ϢΪ:\r\n");
		HAL_UART_Transmit(&huart1,(uint8_t*)USART1_RX_BUF,len,1000);	//���ͽ��յ�������
		while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC)!=SET);			//�ȴ����ͽ���
		printf("\r\n\r\n");//���뻻��
		USART1_RX_STA=0;
	}
/*����*/
	if(ads1292_recive_flag)
	{
//			arm_fir(&Input_data2,&Output_data2);

//			ECG_UI_refresh(&ecg_win,ecg_x_value++,Output_data2);	//ˢ��ʾ�������ڣ�����Ļ���ص�Ϊ��λ��

//			printf("B: %8d,%8d,%d\r\n",(u32)Input_data2,(u32)Output_data2,heart_rate);

			ads1292_recive_flag=0;
	}
	/*�ж�*/	  
	if(key==KEY0_PRES)//KEY0����
	{
		LED1=!LED1;
		key=0;
	}
	//������ջ���������
	else
	{
//		delay_ms(10);  
//		times++;
		
	/*���ڷ�����*/			
//		if(times%5000==0)
//		{
//			//printf("\r\n����������\r\n");
//		}
		//if(times%50==0)printf("������%d\r\n",times/50);  
		//if(times%30==0)LED0=!LED0;//��˸LED,��ʾϵͳ��������.
		
/*�¶Ȳ���*/		
		if(sys_tick-lmt70_tick> 50)
		{
			lmt70_tick = sys_tick;
			
			//�ж�ȥ������	
			adcx=Get_Adc();
			if((adcx-Value)>30||(Value-adcx)>30)
				 Value =  Value;
			else
				Value=adcx;
			
//			
//			temp=(float)Value*(3300.0/4095);      		// ��ȡ�����Ĵ�С����ʵ�ʵ�ѹֵ
//			temperature=(-0.193)*temp+212.009-16;								// �����¶�ֵ
			
			
//			temperature=a*temp*temp*temp+b*temp*temp+c*temp+d;
//			printf("%d,%f,%f\r\n",Value,temp,temperature);
	
			
			Temper_buf[T_sum_count++] = Value;
			if(T_sum_count>=200)
				T_sum_count = 0;
			
			T_sum -= Temper_buf[T_sum_count];
			T_sum += Value;
			T_avr = T_sum/200.0;
			
//			T_sum+=temperature;
//			T_sum_count++;
			if(T_sum_count % 20 == 0)	//2s����һ��
			{
				LCD_SetColor(LIGHT_CYAN);									// ���û���	������ɫ	
				LCD_ShowNumMode(Fill_Space);								// ����λ���ո�
				
//				T_avr = T_sum/T_sum_count ;		//����ƽ���¶�+�����¶�+ delta_tmp
				T_avr=(float)T_avr*(3300.0/4095);
				temperature=a*T_avr*T_avr*T_avr+b*T_avr*T_avr+c*T_avr+d;	
//				temperature=(-0.193)*T_avr+210.509;								// �����¶�ֵ				
				
				LCD_SetColor(LIGHT_YELLOW);					// ���û��ʣ�����ɫ	
				lcd_colour=LIGHT_YELLOW;
				LCD_DisplayDecimals( 686, 211, temperature,  5,2);		// ��ʾС��;
				LCD_DisplayText(766, 211,"��");
				
				LCD_SetColor(LCD_GREEN);												// ���û��ʣ���ɫ	
				lcd_colour=LCD_GREEN;
				LCD_DisplayNumber( 718, 101, heart_rate, 3);		//��ʾ����
//				T_sum_count = 0;
//				T_sum = 0;
			}
		}
		if(sys_tick-step_tick> 50)
		{
			step_tick = sys_tick;
			
//			pedometer_step(&unStepCountTmp,&bushu);						/*�Ʋ�����*/
			LCD_SetTextFont(&CH_Font20);					// ����2424��������,ASCII�����ӦΪ2412
			mpu_dmp_get_data(&pitch,&row,&yaw);
			LCD_SetColor(LIGHT_GREY);						// ���û��ʣ���ɫ
			lcd_colour=LIGHT_GREY;
			LCD_DisplayDecimals(718,400,pitch,5,2); 			//��ʾ�������� 
			LCD_DisplayDecimals(718,425,row,5,2); 				//��ʾ�������� 
			LCD_DisplayDecimals(718,450,yaw,5,2);
			
			LCD_SetTextFont(&CH_Font32);					// ����2424��������,ASCII�����ӦΪ2412
			if(!dmp_get_pedometer_step_count(&unStepCountTmp))		//�õ�����    ����0��ʾ�������óɹ�
			{	
//				if(unStepCountTmp > 0) 				
//				{
					RunningSteps = unStepCountTmp;        	//��Ų���
					LCD_SetColor(LIGHT_CYAN);						// ���û��ʣ�������ɫ
					lcd_colour=LIGHT_CYAN;
					LCD_DisplayNumber(718,321,RunningSteps,3); 			//��ʾ��������                
//				}
			}
			
		}
		
		if(sys_tick-Report_tick> 500)
		{
			Report_tick = sys_tick;
			if(ecg_temp_buf_store_count>=300)
			{
//				resss[0] = printf(
//				"{\"method\":\"thing.service.property.set\",\"id\":\"181454577\",\"params\":{\
//					\"temperature\":%.1f,\
//					\"HeartRate\":%d,\
//					\"RunningSteps\":%d,\
//					\"Switch_LEDB\":%d\
//				},\"sites\":[",
//				(float)temperature,
//				(int)heart_rate,
//				(int)RunningSteps,

//				HAL_GPIO_ReadPin(LED1_GPIO_Port,LED1_Pin) ? 0:1
//				);
//				resss[1] = 0;
////				printf("DATE:300\r\n");
//				for(ecg_temp_buf_tx_count=0;ecg_temp_buf_tx_count<300-1;ecg_temp_buf_tx_count++)
//				{
//					resss[1] += printf("%d,",ecg_temp_buf[ecg_temp_buf_tx_count]);
//				}
//				
//				resss[2]=printf("%d],\"version\":\"1.0.0\"}",
//				ecg_temp_buf[ecg_temp_buf_tx_count]);
//				
//				printf("\r\n");
//				
//				printf("%d,%d,%d\r\n",resss[0],resss[1],resss[2]);
			StatusReport();//�ϱ�����

				ecg_temp_buf_store_count=0;
			}
			
			

		}
		
//		if(times%500==0)
//		{
//			//
//		}
//		
//		if(times%10==0)
//		{
////				if(usart2_rxcounter)
////				{
////					user_main_info("���أ�\r\n");
////					deal_MQTT_message(usart2_rxbuf,usart2_rxcounter);
////					user_main_printf("\r\n");
////				}
//		}
		
	}
		

	  
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
