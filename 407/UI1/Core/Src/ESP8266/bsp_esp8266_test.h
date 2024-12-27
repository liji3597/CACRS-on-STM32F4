#ifndef  __ESP8266_TEST_H
#define	 __ESP8266_TEST_H



#include "stm32f4xx.h"



/********************************** �û���Ҫ���õĲ���**********************************/
#define      macUser_ESP8266_ApSsid                       "502"                //Ҫ���ӵ��ȵ������
#define      macUser_ESP8266_ApPwd                        "bing0508"           //Ҫ���ӵ��ȵ����Կ

#define      macUser_ESP8266_TcpServer_IP                 "192.168.0.102"      //Ҫ���ӵķ������� IP
#define      macUser_ESP8266_TcpServer_Port               "8080"               //Ҫ���ӵķ������Ķ˿�



/********************************** �ⲿȫ�ֱ��� ***************************************/
extern volatile uint8_t ucTcpClosedFlag;
/* �������IO�ĺ� */

#define LED1_TOGGLE		    HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_6);
#define LED1_OFF			HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_RESET);
#define LED1_ON				HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET);

#define LED2_TOGGLE		    HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_7);
#define LED2_OFF			HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_RESET);
#define LED2_ON				HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_SET);

#define LED3_TOGGLE		    HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_8);
#define LED3_OFF			HAL_GPIO_WritePin(GPIOF,GPIO_PIN_8,GPIO_PIN_RESET);
#define LED3_ON				HAL_GPIO_WritePin(GPIOF,GPIO_PIN_8,GPIO_PIN_SET);

/* ������ɫ������߼��÷�ʹ��PWM�ɻ��ȫ����ɫ,��Ч������ */

//��
#define LED_RED  \
					LED1_ON;\
					LED2_OFF;\
					LED3_OFF

//��
#define LED_GREEN		\
					LED1_OFF;\
					LED2_ON;\
					LED3_OFF

//��
#define LED_BLUE	\
					LED1_OFF;\
					LED2_OFF;\
					LED3_ON

					
//��(��+��)					
#define LED_YELLOW	\
					LED1_ON;\
					LED2_ON;\
					LED3_OFF
//��(��+��)
#define LED_PURPLE	\
					LED1_ON;\
					LED2_OFF;\
					LED3_ON

//��(��+��)
#define LED_CYAN \
					LED1_OFF;\
					LED2_ON;\
					LED3_ON
					
//��(��+��+��)
#define LED_WHITE	\
					LED1_ON;\
					LED2_ON;\
					LED3_ON
					
//��(ȫ���ر�)
#define LED_RGBOFF	\
					LED1_OFF;\
					LED2_OFF;\
					LED3_OFF		





/********************************** ���Ժ������� ***************************************/
void ESP8266_StaTcpClient_Unvarnish_ConfigTest(void);
void ESP8266_CheckRecvDataTest(void);

#endif

