/**
  ******************************************************************************
  *	@file  	 	callback.c
  *	@version 	V1.0
  * @date    	2021-06-01
  *	@author  	知行科技
  *	@brief   	hal库中基本外设的回调函数以及RAM、LCD初始化函数
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
  
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "callback.h"
#include "arm_math.h"
//#include "ecg_fir.h"
//#include "UI.h"
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/*外部中断*/
uint8_t key=0;			//按键值
uint32_t lcd_colour;
__IO uint32_t sys_tick = 0;
volatile uint8_t ads1292_recive_flag=2;	//数据读取完成标志
volatile uint8_t ads1292_Cache[9];	//数据缓冲区

//osc_window ecg_win;
//	uint32_t ecg_x_value = 0;	//ecg示波器横轴变量
/*串口1接收状态*/
uint16_t USART1_RX_STA=0;       		//接收状态标记	bit15，	接收完成标志	bit14，	接收到0x0d	bit13~0，	接收到的有效字节数目	
uint8_t USART1_RxBuffer[RXBUFFERSIZE];	//HAL库使用的串口接收缓冲
uint8_t USART1_RX_BUF[USART_REC_LEN];	//接收缓冲,最大USART_REC_LEN个字节.

//usart2发送和接收数组
uint8_t usart2_txbuf[2700];//256
uint8_t usart2_rxbuf[512];
uint8_t usart2_rxone[1];
uint8_t usart2_rxcounter;


/*SRAM相关指令*/
//FMC_SDRAM_CommandTypeDef command;	// 控制指令

//获取温湿度信息
float temperature;


/*ADS1292*/
u32 ch1_data;
u32 ch2_data;
int32_t ch2_temp;
uint16_t ECG_num;
uint16_t EGC_sum_count;

float Input_data2; 					// 输入缓冲区
float Output_data2;         // 输出缓冲区
uint8_t heart_rate;

uint32_t ecg_temp_buf_store_count = 0;	//TCP数据缓存时使用的数组下标
int32_t ecg_temp_buf[600];	//用于发送数据至服务器的缓存区

unsigned int RunningSteps=0;    			//步数

uint32_t ADCConvertedValue;	//ADC DMA  buffer

/* Private user code ---------------------------------------------------------*/

/*************************************************************************************************
*	函 数 名:	fputc
*	入口参数:	ch - 要输出的字符 ，  f - 文件指针（这里用不到）
*	返 回 值:   	正常时返回字符，出错时返回 EOF（-1）
*	函数功能:	重定向 fputc 函数，目的是使用 printf 函数
*	说    明:	无		
*************************************************************************************************/
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 
//	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);	// 发送单字节数据	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*************************************************************************************************
*	函 数 名:	HAL_TIM_PeriodElapsedCallback
*	入口参数:	htim - TIM_HandleTypeDef结构体变量，即表示定义的TIM（句柄）
*	返 回 值:	无
*	函数功能:	中断回调函数，定时器产生更新中断时，将LED1的状态取反	
*	说    明:	该函数会在 TIM3_IRQHandler 里面被调用 ，TIM3_IRQHandler 中断处理函数在 stm32f4xx_it.c文件中，			
*************************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)//更新中断（溢出）发生时执行
{
    if(htim==(&htim3))
    {
        LED0=!LED0;        //LED1反转
    }
		
		 if(htim==(&htim2))
    {
//        LED0=!LED0;        //LED1反转
					sys_tick++;
    }
												 
}

///*************************************************************************************************
//*	函 数 名:	HAL_GPIO_EXTI_Callback
//*	入口参数:	GPIO_Pin - uint16_t形变量，即表示中断引脚号
//*	返 回 值:	无
//*	函数功能:	外部中断服务函数，产生外部中断时，将key变量赋值	
//*	说    明:	该函数会在 EXTI9_5_IRQHandler 里面被调用 ，EXTI9_5_IRQHandler 中断处理函数在 stm32f4xx_it.c文件中，			
//*************************************************************************************************/
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//		if(GPIO_Pin==AD1292_DRDY_Pin)
//		{
//			if(ADS1292_DRDY==0)
//			{
//				ADS1292_Read_Data((u8*)ads1292_Cache);
//				
//				ch2_data = 0;										 // 通道2数据
//								
//				// 计算ADS1292R通道2的数据-心电图数据
//				ch2_data |= (uint32_t)ads1292_Cache[6] << 16;
//				ch2_data |= (uint32_t)ads1292_Cache[7] << 8;
//				ch2_data |= (uint32_t)ads1292_Cache[8] << 0;
//				
//				Input_data2=(float32_t)(ch2_data^0x800000);
//						
//				ads1292_recive_flag=1;
//				
////				arm_fir(&Input_data2,&Output_data2);

////				ECG_UI_refresh(&ecg_win,ecg_x_value++,Output_data2);	//刷新示波器串口（以屏幕像素点为单位）

//				printf("B: %8d,%8d,%d\r\n",(u32)Input_data2,(u32)Output_data2,heart_rate);
//				
//				if(ecg_temp_buf_store_count < 600)
//				{
//					ecg_temp_buf[ecg_temp_buf_store_count++] = ecg_win.width - ecg_win.cursor_y + ecg_win.loca_y;
//				}
//				
////				LCD_SetColor(lcd_colour);
//				
//			}
//		}
//		else
//		{
//			delay_ms(10);      //消抖
//			switch(GPIO_Pin)
//			{		
//					case KEY0_Pin:
//							if(KEY0==0)  
//							{
//					key=KEY0_PRES;
//							}
//							break;
//			}
//		}
//}

/******************************************************************************************************
*	函 数 名: HAL_UART_RxCpltCallback
*	入口参数: huart - UART_HandleTypeDef定义的变量，即表示定义的UART
*	返 回 值: 无
*	函数功能: 串口接收回调函数
*	说    明: 串口协议，以/r/n为结束标志
*******************************************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//如果是串口1
	{
		if((USART1_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART1_RX_STA&0x4000)//接收到了0x0d
			{
				if(USART1_RxBuffer[0]!=0x0a)USART1_RX_STA=0;//接收错误,重新开始
				else USART1_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(USART1_RxBuffer[0]==0x0d)USART1_RX_STA|=0x4000;
				else
				{
					USART1_RX_BUF[USART1_RX_STA&0X3FFF]=USART1_RxBuffer[0] ;
					USART1_RX_STA++;
					if(USART1_RX_STA>(USART_REC_LEN-1))USART1_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}
		
		HAL_UART_Receive_IT(&huart1, (u8 *)USART1_RxBuffer, RXBUFFERSIZE);//该函数会开启接收中断
	}
	if(huart->Instance == USART2)	// 判断是由哪个串口触发的中断
	{
		//将接收到的数据放入接收usart1接收数组
		usart2_rxbuf[usart2_rxcounter] = usart2_rxone[0];
		usart2_rxcounter++;	//接收数量＋1

//		if((usart2_rxcounter&0x8000)==0)//接收未完成
//		{
//			if(usart2_rxcounter&0x4000)//接收到了0x0d
//			{
//				if(usart2_rxone[0]!=0x0a)usart2_rxcounter=0;//接收错误,重新开始
//				else usart2_rxcounter|=0x8000;	//接收完成了 
//			}
//			else //还没收到0X0D
//			{	
//				if(usart2_rxone[0]==0x0d)usart2_rxcounter|=0x4000;
//				else
//				{
//					usart2_rxbuf[usart2_rxcounter&0X3FFF]=usart2_rxone[0] ;
//					usart2_rxcounter++;
//					if(usart2_rxcounter>(USART_REC_LEN-1))usart2_rxcounter=0;//接收数据错误,重新开始接收	  
//				}		 
//			}
//		}   		 
	
		
		
		//重新使能串口1接收中断
		HAL_UART_Receive_IT(&huart2,usart2_rxone,1);		
	}
}

///******************************************************************************************************
//*	函 数 名: SDRAM_Initialization_Sequence
//*	入口参数: hsdram - SDRAM_HandleTypeDef定义的变量，即表示定义的sdram
//*				 Command	- 控制指令
//*	返 回 值: 无
//*	函数功能: SDRAM 参数配置
//*	说    明: 配置SDRAM相关时序和控制方式
//*******************************************************************************************************/
//void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
//{
//  __IO uint32_t tmpmrd = 0;
//  
//  /* Configure a clock configuration enable command */
//  Command->CommandMode 					= FMC_SDRAM_CMD_CLK_ENABLE;	// 开启SDRAM时钟 
//  Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK; 	// 选择要控制的区域
//  Command->AutoRefreshNumber 			= 1;
//  Command->ModeRegisterDefinition 	= 0;
//  
//  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);	// 发送控制指令
//  //SDRAM_delay(1);		// 延时等待
//  delay_ms(5);
//  
//  /* Configure a PALL (precharge all) command */ 
//  Command->CommandMode 					= FMC_SDRAM_CMD_PALL;		// 预充电命令
//  Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;	// 选择要控制的区域
//  Command->AutoRefreshNumber 			= 1;
//  Command->ModeRegisterDefinition 	= 0;
//  
//  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);  // 发送控制指令
//  
//  /* Configure a Auto-Refresh command */ 
//  Command->CommandMode 					= FMC_SDRAM_CMD_AUTOREFRESH_MODE;	// 使用自动刷新
//  Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;          // 选择要控制的区域
//  Command->AutoRefreshNumber			= 8;                                // 自动刷新次数
//  Command->ModeRegisterDefinition 	= 0;
//  
//  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);	// 发送控制指令
//  
//  /* Program the external memory mode register */
//  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
//                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
//                     SDRAM_MODEREG_CAS_LATENCY_3           |
//                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
//                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
//  
//  Command->CommandMode					= FMC_SDRAM_CMD_LOAD_MODE;	// 加载模式寄存器命令
//  Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;	// 选择要控制的区域
//  Command->AutoRefreshNumber 			= 1;
//  Command->ModeRegisterDefinition 	= tmpmrd;
//  
//  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);	// 发送控制指令
//  
//  hsdram->Instance->SDRTR |= ((uint32_t)((1386)<< 1));	// 设置刷新计数器 
//}

///******************************************************************************************************
//*	函 数 名: ADC1_GetVaule
//*	入口参数: 无
//*	返 回 值: ADC1转换值
//*	函数功能: 进行AD转换，采集电压
//*	说    明: 多次采样求平均值，可以提高测量的精度，该函数返回的是AD转换得到的寄存器值
//*******************************************************************************************************/

////uint16_t  ADC1_GetVaule(void)
////{
////	uint8_t  i = 0;
////	uint32_t AD_Vaule = 0;	// AD转换值
////	
////	for(i=0;i<30;i++)	// 进行30次转换
////	{
////		HAL_ADC_Start(&hadc1);  									// 启动转换

////		AD_Vaule = AD_Vaule + HAL_ADC_GetValue(&hadc1);	// 求和	
////	}
////	AD_Vaule	= AD_Vaule / 30;	// 取平均值

////	return (uint16_t)AD_Vaule;	// 返回AD转换值
////}

///******************************************************************************************************
//*	函 数 名: ADC_GetTemp
//*	入口参数: 无
//*	返 回 值: 温度值，舍弃小数位
//*	函数功能: 完成单次转换，获取STM32芯片温度
//*	说    明: 无
//*******************************************************************************************************/

////float ADC_GetTemp(void)
////{	
////	float Temp_AdVaule;		// 温度AD采样值
////	float		Temp_Vaule;		// 实际温度值
////	
////	Temp_AdVaule = ADC1_GetVaule();			// 获取AD值
////	
////	Temp_Vaule = Temp_AdVaule * (3.3/4096);				//	计算电压值				
////	Temp_Vaule = ( Temp_Vaule - 0.76f ) /0.0025f +25; 	// 计算温度值
////	
////	return (float)Temp_Vaule;		// 返回温度值，舍弃小数位
////}

////-----------------------------------------------------------------
//// u16 Get_Adc(u32 ch)
////-----------------------------------------------------------------
////
//// 函数功能: 获得ADC值
//// 入口参数: u32 ch：通道值 0~16，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
//// 返回参数: 转换结果
//// 注意事项: 此函数会被HAL_ADC_Init()调用
////
////-----------------------------------------------------------------
//u16 Get_Adc(void)   
//{
////	ADC_ChannelConfTypeDef ADC2_ChanConf;
//	
////	ADC2_ChanConf.Channel=ch;                            // 通道
////	ADC2_ChanConf.Rank=1;                                // 第1个序列，序列1
////	ADC2_ChanConf.SamplingTime=ADC_SAMPLETIME_480CYCLES; // 采样时间
////	ADC2_ChanConf.Offset=0;                 
////	HAL_ADC_ConfigChannel(&hadc2,&ADC2_ChanConf);  // 通道配置

//	HAL_ADC_Start(&hadc2);                         // 开启ADC

//	HAL_ADC_PollForConversion(&hadc2,10);          // 轮询转换
// 
//	return (u16)HAL_ADC_GetValue(&hadc2);	         // 返回最近一次ADC2规则组的转换结果
//}

//-----------------------------------------------------------------
// u16 Get_Adc_Average(u32 ch,u8 times)
//-----------------------------------------------------------------
//
// 函数功能: 获取指定通道的转换值，取times次,然后平均 
// 入口参数: u32 ch：通道值 0~16，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_16
//					 u8 times：获取次数
// 返回参数: 通道ch的times次转换结果平均值
// 注意事项: 此函数会被HAL_ADC_Init()调用
//
//-----------------------------------------------------------------
//u16 Get_Adc_Average(u32 ch,u8 times)
//{
//	u32 temp_val=0;
//	u8 t;
//	for(t=0;t<times;t++)
//	{
//		temp_val+=Get_Adc(ch);
//		delay_ms(5);
//	}
//	return temp_val/times;
//} 

//void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
//{
//  if(hadc==(&hadc2))
//  {
////    DMA_CNT++;
//		printf("%d\r\n",ADCConvertedValue);

//  }    
//}


//-----------------------------------------------------------------
// void SPI5_SetSpeed(u8 SPI_BaudRatePrescaler)
//-----------------------------------------------------------------
//
// 函数功能: SPI速度设置函数
// 入口参数: u8 SPI_BaudRatePrescaler：SPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
// 返 回 值: 无
// 注意事项: SPI速度=fAPB1/分频系数，fAPB1时钟一般为45Mhz
//
//-----------------------------------------------------------------
void SPI3_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler)); // 判断有效性
  __HAL_SPI_DISABLE(&hspi3);                               // 关闭SPI
  hspi3.Instance->CR1 &= 0XFFC7;                           // 位3-5清零，用来设置波特率
  hspi3.Instance->CR1 |= SPI_BaudRatePrescaler;            // 设置SPI速度
  __HAL_SPI_ENABLE(&hspi3);                                // 使能SPI
}

//-----------------------------------------------------------------
// u8 SPI5_ReadWriteByte(u8 TxData)
//-----------------------------------------------------------------
//
// 函数功能: SPI5 读写一个字节
// 入口参数: u8 TxData： 要写入的字节
// 返 回 值: u8 Rxdata：读取到的字节
// 注意事项: 无
//
//-----------------------------------------------------------------
u8 SPI3_ReadWrite_Byte(u8 dat)
{
  u8 Rxdata;
  HAL_SPI_TransmitReceive(&hspi3, &dat, &Rxdata, 1, 1000);
  return Rxdata; // 返回收到的数据
}

//-----------------------------------------------------------------
// void SPI3_Send_Byte(u8 dat)
//-----------------------------------------------------------------
//
// 函数功能: SPI5发送1个字节数据
// 入口参数: u8 dat： 待发送的数据
// 返 回 值: 无
// 注意事项: 无
//
//-----------------------------------------------------------------
void SPI3_Send_Byte(u8 dat)
{
  u8 Rxdata;
  HAL_SPI_TransmitReceive(&hspi3, &dat, &Rxdata, 1, 1000);
}

//-----------------------------------------------------------------
// void SPI5_Send_Byte(u8 dat)
//-----------------------------------------------------------------
//
// 函数功能: SPI5读取1个字节数据
// 入口参数: u8 dat： 待发送的数据
// 返 回 值: 无
// 注意事项: 无
//
//-----------------------------------------------------------------
u8 SPI3_Read_Byte(void)
{
  u8 Txdata = 0xFF;
  u8 Rxdata;
  HAL_SPI_TransmitReceive(&hspi3, &Txdata, &Rxdata, 1, 1000);
  return Rxdata; // 返回收到的数据
}

//-----------------------------------------------------------------
// End Of File
//----------------------------------------------------------------- 



