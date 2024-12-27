/**
  ******************************************************************************
  *	@file  	 	callback.c
  *	@version 	V1.0
  * @date    	2021-06-01
  *	@author  	֪�пƼ�
  *	@brief   	hal���л�������Ļص������Լ�RAM��LCD��ʼ������
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
/*�ⲿ�ж�*/
uint8_t key=0;			//����ֵ
uint32_t lcd_colour;
__IO uint32_t sys_tick = 0;
volatile uint8_t ads1292_recive_flag=2;	//���ݶ�ȡ��ɱ�־
volatile uint8_t ads1292_Cache[9];	//���ݻ�����

//osc_window ecg_win;
//	uint32_t ecg_x_value = 0;	//ecgʾ�����������
/*����1����״̬*/
uint16_t USART1_RX_STA=0;       		//����״̬���	bit15��	������ɱ�־	bit14��	���յ�0x0d	bit13~0��	���յ�����Ч�ֽ���Ŀ	
uint8_t USART1_RxBuffer[RXBUFFERSIZE];	//HAL��ʹ�õĴ��ڽ��ջ���
uint8_t USART1_RX_BUF[USART_REC_LEN];	//���ջ���,���USART_REC_LEN���ֽ�.

//usart2���ͺͽ�������
uint8_t usart2_txbuf[2700];//256
uint8_t usart2_rxbuf[512];
uint8_t usart2_rxone[1];
uint8_t usart2_rxcounter;


/*SRAM���ָ��*/
//FMC_SDRAM_CommandTypeDef command;	// ����ָ��

//��ȡ��ʪ����Ϣ
float temperature;


/*ADS1292*/
u32 ch1_data;
u32 ch2_data;
int32_t ch2_temp;
uint16_t ECG_num;
uint16_t EGC_sum_count;

float Input_data2; 					// ���뻺����
float Output_data2;         // ���������
uint8_t heart_rate;

uint32_t ecg_temp_buf_store_count = 0;	//TCP���ݻ���ʱʹ�õ������±�
int32_t ecg_temp_buf[600];	//���ڷ����������������Ļ�����

unsigned int RunningSteps=0;    			//����

uint32_t ADCConvertedValue;	//ADC DMA  buffer

/* Private user code ---------------------------------------------------------*/

/*************************************************************************************************
*	�� �� ��:	fputc
*	��ڲ���:	ch - Ҫ������ַ� ��  f - �ļ�ָ�루�����ò�����
*	�� �� ֵ:   	����ʱ�����ַ�������ʱ���� EOF��-1��
*	��������:	�ض��� fputc ������Ŀ����ʹ�� printf ����
*	˵    ��:	��		
*************************************************************************************************/
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 
//	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);	// ���͵��ֽ�����	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*************************************************************************************************
*	�� �� ��:	HAL_TIM_PeriodElapsedCallback
*	��ڲ���:	htim - TIM_HandleTypeDef�ṹ�����������ʾ�����TIM�������
*	�� �� ֵ:	��
*	��������:	�жϻص���������ʱ�����������ж�ʱ����LED1��״̬ȡ��	
*	˵    ��:	�ú������� TIM3_IRQHandler ���汻���� ��TIM3_IRQHandler �жϴ������� stm32f4xx_it.c�ļ��У�			
*************************************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)//�����жϣ����������ʱִ��
{
    if(htim==(&htim3))
    {
        LED0=!LED0;        //LED1��ת
    }
		
		 if(htim==(&htim2))
    {
//        LED0=!LED0;        //LED1��ת
					sys_tick++;
    }
												 
}

///*************************************************************************************************
//*	�� �� ��:	HAL_GPIO_EXTI_Callback
//*	��ڲ���:	GPIO_Pin - uint16_t�α���������ʾ�ж����ź�
//*	�� �� ֵ:	��
//*	��������:	�ⲿ�жϷ������������ⲿ�ж�ʱ����key������ֵ	
//*	˵    ��:	�ú������� EXTI9_5_IRQHandler ���汻���� ��EXTI9_5_IRQHandler �жϴ������� stm32f4xx_it.c�ļ��У�			
//*************************************************************************************************/
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//		if(GPIO_Pin==AD1292_DRDY_Pin)
//		{
//			if(ADS1292_DRDY==0)
//			{
//				ADS1292_Read_Data((u8*)ads1292_Cache);
//				
//				ch2_data = 0;										 // ͨ��2����
//								
//				// ����ADS1292Rͨ��2������-�ĵ�ͼ����
//				ch2_data |= (uint32_t)ads1292_Cache[6] << 16;
//				ch2_data |= (uint32_t)ads1292_Cache[7] << 8;
//				ch2_data |= (uint32_t)ads1292_Cache[8] << 0;
//				
//				Input_data2=(float32_t)(ch2_data^0x800000);
//						
//				ads1292_recive_flag=1;
//				
////				arm_fir(&Input_data2,&Output_data2);

////				ECG_UI_refresh(&ecg_win,ecg_x_value++,Output_data2);	//ˢ��ʾ�������ڣ�����Ļ���ص�Ϊ��λ��

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
//			delay_ms(10);      //����
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
*	�� �� ��: HAL_UART_RxCpltCallback
*	��ڲ���: huart - UART_HandleTypeDef����ı���������ʾ�����UART
*	�� �� ֵ: ��
*	��������: ���ڽ��ջص�����
*	˵    ��: ����Э�飬��/r/nΪ������־
*******************************************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//����Ǵ���1
	{
		if((USART1_RX_STA&0x8000)==0)//����δ���
		{
			if(USART1_RX_STA&0x4000)//���յ���0x0d
			{
				if(USART1_RxBuffer[0]!=0x0a)USART1_RX_STA=0;//���մ���,���¿�ʼ
				else USART1_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(USART1_RxBuffer[0]==0x0d)USART1_RX_STA|=0x4000;
				else
				{
					USART1_RX_BUF[USART1_RX_STA&0X3FFF]=USART1_RxBuffer[0] ;
					USART1_RX_STA++;
					if(USART1_RX_STA>(USART_REC_LEN-1))USART1_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}
		
		HAL_UART_Receive_IT(&huart1, (u8 *)USART1_RxBuffer, RXBUFFERSIZE);//�ú����Ὺ�������ж�
	}
	if(huart->Instance == USART2)	// �ж������ĸ����ڴ������ж�
	{
		//�����յ������ݷ������usart1��������
		usart2_rxbuf[usart2_rxcounter] = usart2_rxone[0];
		usart2_rxcounter++;	//����������1

//		if((usart2_rxcounter&0x8000)==0)//����δ���
//		{
//			if(usart2_rxcounter&0x4000)//���յ���0x0d
//			{
//				if(usart2_rxone[0]!=0x0a)usart2_rxcounter=0;//���մ���,���¿�ʼ
//				else usart2_rxcounter|=0x8000;	//��������� 
//			}
//			else //��û�յ�0X0D
//			{	
//				if(usart2_rxone[0]==0x0d)usart2_rxcounter|=0x4000;
//				else
//				{
//					usart2_rxbuf[usart2_rxcounter&0X3FFF]=usart2_rxone[0] ;
//					usart2_rxcounter++;
//					if(usart2_rxcounter>(USART_REC_LEN-1))usart2_rxcounter=0;//�������ݴ���,���¿�ʼ����	  
//				}		 
//			}
//		}   		 
	
		
		
		//����ʹ�ܴ���1�����ж�
		HAL_UART_Receive_IT(&huart2,usart2_rxone,1);		
	}
}

///******************************************************************************************************
//*	�� �� ��: SDRAM_Initialization_Sequence
//*	��ڲ���: hsdram - SDRAM_HandleTypeDef����ı���������ʾ�����sdram
//*				 Command	- ����ָ��
//*	�� �� ֵ: ��
//*	��������: SDRAM ��������
//*	˵    ��: ����SDRAM���ʱ��Ϳ��Ʒ�ʽ
//*******************************************************************************************************/
//void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command)
//{
//  __IO uint32_t tmpmrd = 0;
//  
//  /* Configure a clock configuration enable command */
//  Command->CommandMode 					= FMC_SDRAM_CMD_CLK_ENABLE;	// ����SDRAMʱ�� 
//  Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK; 	// ѡ��Ҫ���Ƶ�����
//  Command->AutoRefreshNumber 			= 1;
//  Command->ModeRegisterDefinition 	= 0;
//  
//  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);	// ���Ϳ���ָ��
//  //SDRAM_delay(1);		// ��ʱ�ȴ�
//  delay_ms(5);
//  
//  /* Configure a PALL (precharge all) command */ 
//  Command->CommandMode 					= FMC_SDRAM_CMD_PALL;		// Ԥ�������
//  Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;	// ѡ��Ҫ���Ƶ�����
//  Command->AutoRefreshNumber 			= 1;
//  Command->ModeRegisterDefinition 	= 0;
//  
//  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);  // ���Ϳ���ָ��
//  
//  /* Configure a Auto-Refresh command */ 
//  Command->CommandMode 					= FMC_SDRAM_CMD_AUTOREFRESH_MODE;	// ʹ���Զ�ˢ��
//  Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;          // ѡ��Ҫ���Ƶ�����
//  Command->AutoRefreshNumber			= 8;                                // �Զ�ˢ�´���
//  Command->ModeRegisterDefinition 	= 0;
//  
//  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);	// ���Ϳ���ָ��
//  
//  /* Program the external memory mode register */
//  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2          |
//                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
//                     SDRAM_MODEREG_CAS_LATENCY_3           |
//                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |
//                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;
//  
//  Command->CommandMode					= FMC_SDRAM_CMD_LOAD_MODE;	// ����ģʽ�Ĵ�������
//  Command->CommandTarget 				= FMC_COMMAND_TARGET_BANK;	// ѡ��Ҫ���Ƶ�����
//  Command->AutoRefreshNumber 			= 1;
//  Command->ModeRegisterDefinition 	= tmpmrd;
//  
//  HAL_SDRAM_SendCommand(hsdram, Command, SDRAM_TIMEOUT);	// ���Ϳ���ָ��
//  
//  hsdram->Instance->SDRTR |= ((uint32_t)((1386)<< 1));	// ����ˢ�¼����� 
//}

///******************************************************************************************************
//*	�� �� ��: ADC1_GetVaule
//*	��ڲ���: ��
//*	�� �� ֵ: ADC1ת��ֵ
//*	��������: ����ADת�����ɼ���ѹ
//*	˵    ��: ��β�����ƽ��ֵ��������߲����ľ��ȣ��ú������ص���ADת���õ��ļĴ���ֵ
//*******************************************************************************************************/

////uint16_t  ADC1_GetVaule(void)
////{
////	uint8_t  i = 0;
////	uint32_t AD_Vaule = 0;	// ADת��ֵ
////	
////	for(i=0;i<30;i++)	// ����30��ת��
////	{
////		HAL_ADC_Start(&hadc1);  									// ����ת��

////		AD_Vaule = AD_Vaule + HAL_ADC_GetValue(&hadc1);	// ���	
////	}
////	AD_Vaule	= AD_Vaule / 30;	// ȡƽ��ֵ

////	return (uint16_t)AD_Vaule;	// ����ADת��ֵ
////}

///******************************************************************************************************
//*	�� �� ��: ADC_GetTemp
//*	��ڲ���: ��
//*	�� �� ֵ: �¶�ֵ������С��λ
//*	��������: ��ɵ���ת������ȡSTM32оƬ�¶�
//*	˵    ��: ��
//*******************************************************************************************************/

////float ADC_GetTemp(void)
////{	
////	float Temp_AdVaule;		// �¶�AD����ֵ
////	float		Temp_Vaule;		// ʵ���¶�ֵ
////	
////	Temp_AdVaule = ADC1_GetVaule();			// ��ȡADֵ
////	
////	Temp_Vaule = Temp_AdVaule * (3.3/4096);				//	�����ѹֵ				
////	Temp_Vaule = ( Temp_Vaule - 0.76f ) /0.0025f +25; 	// �����¶�ֵ
////	
////	return (float)Temp_Vaule;		// �����¶�ֵ������С��λ
////}

////-----------------------------------------------------------------
//// u16 Get_Adc(u32 ch)
////-----------------------------------------------------------------
////
//// ��������: ���ADCֵ
//// ��ڲ���: u32 ch��ͨ��ֵ 0~16��ȡֵ��ΧΪ��ADC_CHANNEL_0~ADC_CHANNEL_16
//// ���ز���: ת�����
//// ע������: �˺����ᱻHAL_ADC_Init()����
////
////-----------------------------------------------------------------
//u16 Get_Adc(void)   
//{
////	ADC_ChannelConfTypeDef ADC2_ChanConf;
//	
////	ADC2_ChanConf.Channel=ch;                            // ͨ��
////	ADC2_ChanConf.Rank=1;                                // ��1�����У�����1
////	ADC2_ChanConf.SamplingTime=ADC_SAMPLETIME_480CYCLES; // ����ʱ��
////	ADC2_ChanConf.Offset=0;                 
////	HAL_ADC_ConfigChannel(&hadc2,&ADC2_ChanConf);  // ͨ������

//	HAL_ADC_Start(&hadc2);                         // ����ADC

//	HAL_ADC_PollForConversion(&hadc2,10);          // ��ѯת��
// 
//	return (u16)HAL_ADC_GetValue(&hadc2);	         // �������һ��ADC2�������ת�����
//}

//-----------------------------------------------------------------
// u16 Get_Adc_Average(u32 ch,u8 times)
//-----------------------------------------------------------------
//
// ��������: ��ȡָ��ͨ����ת��ֵ��ȡtimes��,Ȼ��ƽ�� 
// ��ڲ���: u32 ch��ͨ��ֵ 0~16��ȡֵ��ΧΪ��ADC_CHANNEL_0~ADC_CHANNEL_16
//					 u8 times����ȡ����
// ���ز���: ͨ��ch��times��ת�����ƽ��ֵ
// ע������: �˺����ᱻHAL_ADC_Init()����
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
// ��������: SPI�ٶ����ú���
// ��ڲ���: u8 SPI_BaudRatePrescaler��SPI_BAUDRATEPRESCALER_2~SPI_BAUDRATEPRESCALER_2 256
// �� �� ֵ: ��
// ע������: SPI�ٶ�=fAPB1/��Ƶϵ����fAPB1ʱ��һ��Ϊ45Mhz
//
//-----------------------------------------------------------------
void SPI3_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler)); // �ж���Ч��
  __HAL_SPI_DISABLE(&hspi3);                               // �ر�SPI
  hspi3.Instance->CR1 &= 0XFFC7;                           // λ3-5���㣬�������ò�����
  hspi3.Instance->CR1 |= SPI_BaudRatePrescaler;            // ����SPI�ٶ�
  __HAL_SPI_ENABLE(&hspi3);                                // ʹ��SPI
}

//-----------------------------------------------------------------
// u8 SPI5_ReadWriteByte(u8 TxData)
//-----------------------------------------------------------------
//
// ��������: SPI5 ��дһ���ֽ�
// ��ڲ���: u8 TxData�� Ҫд����ֽ�
// �� �� ֵ: u8 Rxdata����ȡ�����ֽ�
// ע������: ��
//
//-----------------------------------------------------------------
u8 SPI3_ReadWrite_Byte(u8 dat)
{
  u8 Rxdata;
  HAL_SPI_TransmitReceive(&hspi3, &dat, &Rxdata, 1, 1000);
  return Rxdata; // �����յ�������
}

//-----------------------------------------------------------------
// void SPI3_Send_Byte(u8 dat)
//-----------------------------------------------------------------
//
// ��������: SPI5����1���ֽ�����
// ��ڲ���: u8 dat�� �����͵�����
// �� �� ֵ: ��
// ע������: ��
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
// ��������: SPI5��ȡ1���ֽ�����
// ��ڲ���: u8 dat�� �����͵�����
// �� �� ֵ: ��
// ע������: ��
//
//-----------------------------------------------------------------
u8 SPI3_Read_Byte(void)
{
  u8 Txdata = 0xFF;
  u8 Rxdata;
  HAL_SPI_TransmitReceive(&hspi3, &Txdata, &Rxdata, 1, 1000);
  return Rxdata; // �����յ�������
}

//-----------------------------------------------------------------
// End Of File
//----------------------------------------------------------------- 



