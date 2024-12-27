#ifndef  __ESP8266_TEST_H
#define	 __ESP8266_TEST_H



#include "stm32f4xx.h"



/********************************** 用户需要设置的参数**********************************/
#define      macUser_ESP8266_ApSsid                       "502"                //要连接的热点的名称
#define      macUser_ESP8266_ApPwd                        "bing0508"           //要连接的热点的密钥

#define      macUser_ESP8266_TcpServer_IP                 "192.168.0.102"      //要连接的服务器的 IP
#define      macUser_ESP8266_TcpServer_Port               "8080"               //要连接的服务器的端口



/********************************** 外部全局变量 ***************************************/
extern volatile uint8_t ucTcpClosedFlag;
/* 定义控制IO的宏 */

#define LED1_TOGGLE		    HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_6);
#define LED1_OFF			HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_RESET);
#define LED1_ON				HAL_GPIO_WritePin(GPIOF,GPIO_PIN_6,GPIO_PIN_SET);

#define LED2_TOGGLE		    HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_7);
#define LED2_OFF			HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_RESET);
#define LED2_ON				HAL_GPIO_WritePin(GPIOF,GPIO_PIN_7,GPIO_PIN_SET);

#define LED3_TOGGLE		    HAL_GPIO_TogglePin(GPIOF,GPIO_PIN_8);
#define LED3_OFF			HAL_GPIO_WritePin(GPIOF,GPIO_PIN_8,GPIO_PIN_RESET);
#define LED3_ON				HAL_GPIO_WritePin(GPIOF,GPIO_PIN_8,GPIO_PIN_SET);

/* 基本混色，后面高级用法使用PWM可混出全彩颜色,且效果更好 */

//红
#define LED_RED  \
					LED1_ON;\
					LED2_OFF;\
					LED3_OFF

//绿
#define LED_GREEN		\
					LED1_OFF;\
					LED2_ON;\
					LED3_OFF

//蓝
#define LED_BLUE	\
					LED1_OFF;\
					LED2_OFF;\
					LED3_ON

					
//黄(红+绿)					
#define LED_YELLOW	\
					LED1_ON;\
					LED2_ON;\
					LED3_OFF
//紫(红+蓝)
#define LED_PURPLE	\
					LED1_ON;\
					LED2_OFF;\
					LED3_ON

//青(绿+蓝)
#define LED_CYAN \
					LED1_OFF;\
					LED2_ON;\
					LED3_ON
					
//白(红+绿+蓝)
#define LED_WHITE	\
					LED1_ON;\
					LED2_ON;\
					LED3_ON
					
//黑(全部关闭)
#define LED_RGBOFF	\
					LED1_OFF;\
					LED2_OFF;\
					LED3_OFF		





/********************************** 测试函数声明 ***************************************/
void ESP8266_StaTcpClient_Unvarnish_ConfigTest(void);
void ESP8266_CheckRecvDataTest(void);

#endif

