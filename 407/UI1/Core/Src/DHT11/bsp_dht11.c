/**
  ******************************************************************************
  * @file    bsp_dht11.c
  * @author  fire
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   ��ʪ�ȴ�����Ӧ�ú����ӿ�
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��  STM32 F407 ������  
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
#include "bsp_dht11.h"
#include "delay.h" 

/* ����������ĺ궨���аѺ������ʱ�����滻��SysTick����ʱ���������������Ǹ��ͻ����Ǹ��� */


/*
 * ��������DHT11_GPIO_Config
 * ����  ������DHT11�õ���I/O��
 * ����  ����
 * ���  ����
 */
void DHT11_GPIO_Config(void)
{		
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*����DHT11_PORT������ʱ��*/
	__HAL_RCC_GPIOE_CLK_ENABLE(); 

	/*ѡ��Ҫ���Ƶ�DHT11_PORT����*/															   
  	GPIO_InitStructure.Pin = DHT11_PIN;	

	/*��������ģʽΪͨ���������*/
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;   
  
  /*�������ŵ��������Ϊ�������*/
//  GPIO_InitStructure.Type = GPIO_OType_PP;
  
  /*��������Ϊ����ģʽ*/
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  
	/*������������Ϊ50MHz */   
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; 

	/*���ÿ⺯������ʼ��DHT11_PORT*/
  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructure); 
}

/*
 * ��������DHT11_Mode_IPU
 * ����  ��ʹDHT11-DATA���ű�Ϊ��������ģʽ
 * ����  ����
 * ���  ����
 */
static void DHT11_Mode_IPU(void)
{
 	  GPIO_InitTypeDef GPIO_InitStructure;

	  	/*ѡ��Ҫ���Ƶ�DHT11_PORT����*/	
	  GPIO_InitStructure.Pin = DHT11_PIN;

	   /*��������ģʽΪ��������ģʽ*/ 
	  GPIO_InitStructure.Mode = GPIO_MODE_INPUT ; 
  
	  GPIO_InitStructure.Pull = GPIO_NOPULL;
  
  	/*������������Ϊ50MHz */   
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; 
  
	  /*���ÿ⺯������ʼ��DHT11_PORT*/
	  HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 
}

/*
 * ��������DHT11_Mode_Out_PP
 * ����  ��ʹDHT11-DATA���ű�Ϊ�������ģʽ
 * ����  ����
 * ���  ����
 */
static void DHT11_Mode_Out_PP(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;

	/*ѡ��Ҫ���Ƶ�DHT11_PORT����*/															   
  GPIO_InitStructure.Pin = DHT11_PIN;	

	/*��������ģʽΪͨ���������*/
  	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;   
  
  /*�������ŵ��������Ϊ�������*/
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  
  /*��������Ϊ����ģʽ*/
	GPIO_InitStructure.Pull = GPIO_PULLUP;
  
	/*������������Ϊ50MHz */   
  GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; 

	/*���ÿ⺯������ʼ��DHT11_PORT*/
  	HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStructure);	 	 
}

/* 
 * ��DHT11��ȡһ���ֽڣ�MSB����
 */
static uint8_t Read_Byte(void)
{
	uint8_t i, temp=0;

	for(i=0;i<8;i++)    
	{	 
		/*ÿbit��50us�͵�ƽ���ÿ�ʼ����ѯֱ���ӻ����� ��50us �͵�ƽ ����*/  
		while(DHT11_DATA_IN()==GPIO_PIN_RESET);

		/*DHT11 ��26~28us�ĸߵ�ƽ��ʾ��0������70us�ߵ�ƽ��ʾ��1����
		 *ͨ����� x us��ĵ�ƽ��������������״ ��x ���������ʱ 
		 */
		delay_us(40); //��ʱx us �����ʱ��Ҫ��������0������ʱ�伴��	   	  

		if(DHT11_DATA_IN()==GPIO_PIN_SET)/* x us����Ϊ�ߵ�ƽ��ʾ���ݡ�1�� */
		{
			/* �ȴ�����1�ĸߵ�ƽ���� */
			while(DHT11_DATA_IN()==GPIO_PIN_SET);

			temp|=(uint8_t)(0x01<<(7-i));  //�ѵ�7-iλ��1��MSB���� 
		}
		else	 // x us��Ϊ�͵�ƽ��ʾ���ݡ�0��
		{			   
			temp&=(uint8_t)~(0x01<<(7-i)); //�ѵ�7-iλ��0��MSB����
		}
	}
	return temp;
}
/*
 * һ�����������ݴ���Ϊ40bit����λ�ȳ�
 * 8bit ʪ������ + 8bit ʪ��С�� + 8bit �¶����� + 8bit �¶�С�� + 8bit У��� 
 */
uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{  
  uint16_t count;
	/*���ģʽ*/
	DHT11_Mode_Out_PP();
	/*��������*/
	DHT11_DATA_OUT(DHT11_LOW);
	/*��ʱ18ms*/
	delay_us(20000);

	/*�������� ������ʱ30us*/
	DHT11_DATA_OUT(DHT11_HIGH); 

	delay_us(30);   //��ʱ30us

	/*������Ϊ���� �жϴӻ���Ӧ�ź�*/ 
	DHT11_Mode_IPU();

	/*�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������*/   
	if(DHT11_DATA_IN()==GPIO_PIN_RESET)     
	{
    count=0;
		/*��ѯֱ���ӻ����� ��80us �͵�ƽ ��Ӧ�źŽ���*/  
		while(DHT11_DATA_IN()==GPIO_PIN_RESET)
    {
      count++;
      if(count>1000)  return 0;
      delay_us(10); 
    }    
    
    count=0;
		/*��ѯֱ���ӻ������� 80us �ߵ�ƽ �����źŽ���*/
		while(DHT11_DATA_IN()==GPIO_PIN_SET)
    {
      count++;
      if(count>1000)  return 0;
      delay_us(10); 
    }  
		/*��ʼ��������*/   
		DHT11_Data->humi_int= Read_Byte();

		DHT11_Data->humi_deci= Read_Byte();

		DHT11_Data->temp_int= Read_Byte();

		DHT11_Data->temp_deci= Read_Byte();

		DHT11_Data->check_sum= Read_Byte();

		/*��ȡ���������Ÿ�Ϊ���ģʽ*/
		DHT11_Mode_Out_PP();
		/*��������*/
		DHT11_DATA_OUT(DHT11_HIGH);

		/*����ȡ�������Ƿ���ȷ*/
		if(DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + DHT11_Data->temp_int+ DHT11_Data->temp_deci)
			return 1;
		else 
			return 0;
	}
	else
	{		
		return 0;
	}   
}

