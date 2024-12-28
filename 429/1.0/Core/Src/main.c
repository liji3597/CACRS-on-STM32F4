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

//定义心电图窗口
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
	LCD_SetTextFont(&CH_Font20);					// 设置2424中文字体,ASCII字体对应为2412
	mpu_dmp_get_data(&pitch,&row,&yaw);
	LCD_DisplayDecimals(700,400,pitch,5,2); 			//显示整数部分 
	LCD_DisplayDecimals(700,425,row,5,2); 			//显示整数部分 
	LCD_DisplayDecimals(700,450,yaw,5,2);
//	MPU_Get_Gyroscope(&gx,&gy,&gz);
//	MPU_Get_Accelerometer(&ax,&ay,&az);
//	LCD_ShowString(30,180,200,16,16,&g);
//	LCD_ShowString(30,200,200,16,16,&a);
		LCD_SetTextFont(&CH_Font32);					// 设置2424中文字体,ASCII字体对应为2412
		if(!dmp_get_pedometer_step_count(unStepCountTmp))		//得到步数    返回0表示函数调用成功
		{	
			if(*unStepCountTmp > 0) 				
			{
				*bushu = *unStepCountTmp;        	//存放步数
				LCD_DisplayNumber(700,350,*bushu,3); 			//显示整数部分                
			}
		}
}

//LMT70解算三阶方程对应四个参数
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
/*时间*/
	uint32_t lmt70_tick = 0 ;	//LMT70采样周期
	uint32_t step_tick = 0;		//步数显示时间
	uint32_t Report_tick = 0;
//	int ecg_temp_buf_tx_count = 0;
//u16 EGC_i;	
/*串口*/	
	uint8_t len;					//串口接收数据长度
	
//	int resss[3];
/*ADC采集*/	
	u16 adcx;
	u16 Value;
	
	u16 Temper_buf[200];
	float T_sum;	//温度求平均时的临时变量
	float T_avr;		//求平均的结果
	uint8_t T_sum_count;	//求平均时的累加计数	
	
//	float temp;
	
//	uint32_t ecg_x_value = 0;	//ecg示波器横轴变量
	
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

	delay_init(180);                //初始化延时函数

	HAL_UART_Receive_IT(&huart1, (u8 *)USART1_RxBuffer, RXBUFFERSIZE);//该函数会开启接收中断
	HAL_UART_Receive_IT(&huart2,usart2_rxone,1);			//打开USART1中断，接收订阅消息
	HAL_TIM_Base_Start_IT(&htim2);
//	HAL_TIM_Base_Start_IT(&htim3);							  	      //使能定时器3和定时器3更新中断：TIM_IT_UPDATE，，周期为500ms
	SDRAM_Initialization_Sequence(&hsdram2,&command);				  //配置SDRAM
	__HAL_SPI_ENABLE(&hspi5);                                // 使能SPI5
//	HAL_ADC_Start_IT(&hadc2);
//	HAL_ADC_Start_DMA(&hadc2,&ADCConvertedValue,1);
//	
	LCD_Init();						//配置RGB-LCD	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  	
//	LCD_Test();						//RGB屏幕测试
	LCD_SetBackColor(LCD_BLACK); 			//	设置背景色
	LCD_Clear(); 											// 清屏
	LCD_SetTextFont(&CH_Font32);					// 设置3232中文字体,ASCII字体对应为
	LCD_SetColor(LCD_WHITE);						// 设置画笔,白色
	LCD_DrawRect(0,0,799,38);
	LCD_DrawRect(606,40,193,109);
	LCD_DrawRect(606,150,193,109);
	LCD_DrawRect(606,260,193,109);
	LCD_DrawRect(606,370,193,109);
	LCD_DisplayText(190, 2,"无线运动数据采集传感器节点");	// 显示标题
		
	LCD_SetColor(LCD_GREEN);						// 设置画笔，绿色			
	LCD_DisplayText(610,  55,"心率：");
	LCD_SetColor(LIGHT_YELLOW);					// 设置画笔，亮黄色		
	LCD_DisplayText(610, 165,"温度：");	
	LCD_SetColor(LIGHT_CYAN);						// 设置画笔，亮蓝绿色
	LCD_DisplayText(610, 275,"步数：");
	LCD_SetColor(LIGHT_GREY);						// 设置画笔，灰色
	LCD_DisplayText(610, 385,"姿态：");
	
	ECG_UI_init(&ecg_win,0,40,604,439);		//示波器初始化
	arm_fir_init();
	
	MPU_Init();          //mpu6050初始化
/*初始化6050*/
	while(mpu_dmp_init())  	
	{
		LCD_DisplayString(30,130,"MPU6050 Error");
		delay_ms(200);
//		LCD_Fill(30,130,239,130+16,WHITE);						
		delay_ms(200);
	}
//	LCD_DisplayString(30,130,200,16,16,(u8*)"mpu6050 ok"); 		
	dmp_set_pedometer_step_count(0);    
	/*显示仰俯角，步数*/		
//	LCD_DisplayString(30,160,200,16,16,"pitch:   ");	 
//	LCD_DisplayString(30,180,200,16,16,"row  :   ");
//	LCD_DisplayString(30,200,200,16,16,"yaw  :   ");
//	LCD_DisplayString(30,220,200,16,16,"step :   ");			
	
	ES8266_MQTT_Init();
	StatusReport();
		
	ADS1292_PowerOnInit();						 // ADS1292上电初始化
	while(Set_ADS1292_Collect(0))//0 正常采集  //1 1mV1Hz内部侧试信号 //2 内部短接噪声测试
	{	
			printf("1292寄存器设置失败\r\n");
			delay_ms(500);
			LED1=!LED1;
	}

	Value = Get_Adc();
	
  while (1)
  {
	  
/*串口*/
	if(USART1_RX_STA&0x8000)
	{					   
		len=USART1_RX_STA&0x3fff;		//得到此次接收到的数据长度
		printf("\r\n您发送的消息为:\r\n");
		HAL_UART_Transmit(&huart1,(uint8_t*)USART1_RX_BUF,len,1000);	//发送接收到的数据
		while(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_TC)!=SET);			//等待发送结束
		printf("\r\n\r\n");//插入换行
		USART1_RX_STA=0;
	}
/*心率*/
	if(ads1292_recive_flag)
	{
//			arm_fir(&Input_data2,&Output_data2);

//			ECG_UI_refresh(&ecg_win,ecg_x_value++,Output_data2);	//刷新示波器串口（以屏幕像素点为单位）

//			printf("B: %8d,%8d,%d\r\n",(u32)Input_data2,(u32)Output_data2,heart_rate);

			ads1292_recive_flag=0;
	}
	/*中断*/	  
	if(key==KEY0_PRES)//KEY0按下
	{
		LED1=!LED1;
		key=0;
	}
	//如果接收缓存有数据
	else
	{
//		delay_ms(10);  
//		times++;
		
	/*串口发数据*/			
//		if(times%5000==0)
//		{
//			//printf("\r\n串口运行中\r\n");
//		}
		//if(times%50==0)printf("计数：%d\r\n",times/50);  
		//if(times%30==0)LED0=!LED0;//闪烁LED,提示系统正在运行.
		
/*温度测量*/		
		if(sys_tick-lmt70_tick> 50)
		{
			lmt70_tick = sys_tick;
			
			//判断去除跳变	
			adcx=Get_Adc();
			if((adcx-Value)>30||(Value-adcx)>30)
				 Value =  Value;
			else
				Value=adcx;
			
//			
//			temp=(float)Value*(3300.0/4095);      		// 获取计算后的带小数的实际电压值
//			temperature=(-0.193)*temp+212.009-16;								// 计算温度值
			
			
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
			if(T_sum_count % 20 == 0)	//2s计算一次
			{
				LCD_SetColor(LIGHT_CYAN);									// 设置画笔	，蓝绿色	
				LCD_ShowNumMode(Fill_Space);								// 多余位填充空格
				
//				T_avr = T_sum/T_sum_count ;		//计算平均温度+补偿温度+ delta_tmp
				T_avr=(float)T_avr*(3300.0/4095);
				temperature=a*T_avr*T_avr*T_avr+b*T_avr*T_avr+c*T_avr+d;	
//				temperature=(-0.193)*T_avr+210.509;								// 计算温度值				
				
				LCD_SetColor(LIGHT_YELLOW);					// 设置画笔，亮黄色	
				lcd_colour=LIGHT_YELLOW;
				LCD_DisplayDecimals( 686, 211, temperature,  5,2);		// 显示小数;
				LCD_DisplayText(766, 211,"℃");
				
				LCD_SetColor(LCD_GREEN);												// 设置画笔，绿色	
				lcd_colour=LCD_GREEN;
				LCD_DisplayNumber( 718, 101, heart_rate, 3);		//显示心率
//				T_sum_count = 0;
//				T_sum = 0;
			}
		}
		if(sys_tick-step_tick> 50)
		{
			step_tick = sys_tick;
			
//			pedometer_step(&unStepCountTmp,&bushu);						/*计步测量*/
			LCD_SetTextFont(&CH_Font20);					// 设置2424中文字体,ASCII字体对应为2412
			mpu_dmp_get_data(&pitch,&row,&yaw);
			LCD_SetColor(LIGHT_GREY);						// 设置画笔，灰色
			lcd_colour=LIGHT_GREY;
			LCD_DisplayDecimals(718,400,pitch,5,2); 			//显示整数部分 
			LCD_DisplayDecimals(718,425,row,5,2); 				//显示整数部分 
			LCD_DisplayDecimals(718,450,yaw,5,2);
			
			LCD_SetTextFont(&CH_Font32);					// 设置2424中文字体,ASCII字体对应为2412
			if(!dmp_get_pedometer_step_count(&unStepCountTmp))		//得到步数    返回0表示函数调用成功
			{	
//				if(unStepCountTmp > 0) 				
//				{
					RunningSteps = unStepCountTmp;        	//存放步数
					LCD_SetColor(LIGHT_CYAN);						// 设置画笔，亮蓝绿色
					lcd_colour=LIGHT_CYAN;
					LCD_DisplayNumber(718,321,RunningSteps,3); 			//显示整数部分                
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
			StatusReport();//上报数据

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
////					user_main_info("下载：\r\n");
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
