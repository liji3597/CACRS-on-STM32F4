#ifndef __ES8266_MQTT_H
#define __ES8266_MQTT_H

#include "stm32f4xx_hal.h"

#define BYTE0(dwTemp)       (*( char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))

#define USER_MAIN_DEBUG

#ifdef USER_MAIN_DEBUG
#define user_main_printf(format, ...) printf( format "\r\n",##__VA_ARGS__)
#define user_main_info(format, ...) printf("��main��info:" format "\r\n",##__VA_ARGS__)
#define user_main_debug(format, ...) printf("��main��debug:" format "\r\n",##__VA_ARGS__)
#define user_main_error(format, ...) printf("��main��error:" format "\r\n",##__VA_ARGS__)
#else
#define user_main_printf(format, ...)
#define user_main_info(format, ...)
#define user_main_debug(format, ...)
#define user_main_error(format, ...)
#endif

//�˴������Լ���wifi������
#define WIFI_NAME "secret"
#define WIFI_PASSWD "guo12345"

//�˴��ǰ����Ʒ������ĵ�½����
//#define MQTT_BROKERADDRESS "a1juWYHkZEC.iot-as-mqtt.cn-shanghai.aliyuncs.com"
#define MQTT_BROKERADDRESS "8.141.154.35"
#define MQTT_CLIENTID "404|securemode=3,signmethod=hmacsha1,timestamp=1637827727886|"
#define MQTT_USARNAME "TESTXL&a1juWYHkZEC"
#define MQTT_PASSWD "E08C23771773D2D8CEF51C9BE7D863BF0B20712A"
//#define	MQTT_PUBLISH_TOPIC "/sys/a1juWYHkZEC/TESTXL/thing/event/property/post"
//#define MQTT_SUBSCRIBE_TOPIC "/sys/a1juWYHkZEC/TESTXL/thing/service/property/set"
#define	MQTT_PUBLISH_TOPIC "/broadcast/a1juWYHkZEC/b"
#define MQTT_SUBSCRIBE_TOPIC "/broadcast/a1juWYHkZEC/c"
//�˴�����ѭ��������ʱ�궨��
#define LOOPTIME 30 	//��������ѭ����ʱʱ�䣺30ms
#define COUNTER_LEDBLINK			(300/LOOPTIME)		//LED������˸ʱ�䣺300ms
#define COUNTER_RUNINFOSEND		(5000/LOOPTIME)		//���д�����ʾ��5s
#define COUNTER_MQTTHEART     (5000/LOOPTIME)		//MQTT������������5s
#define COUNTER_STATUSREPORT	(3000/LOOPTIME)		//״̬�ϴ���3s


//MQTT���ӷ�����
extern uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password);
//MQTT��Ϣ����
extern uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether);
//MQTT��Ϣ����
extern uint8_t MQTT_PublishData(char *topic, char *message, uint8_t qos);
//MQTT����������
extern void MQTT_SentHeart(void);

extern void ES8266_MQTT_Init(void);

extern void StatusReport(void);
extern void deal_MQTT_message(uint8_t* buf,uint16_t len);
#endif
