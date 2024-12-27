#include "esp8266_mqtt.h"
#include "esp8266_at.h"
#include "callback.h"

char mqtt_message[2700];	//MQTT的上报消息缓存

//连接成功服务器回应 20 02 00 00
//客户端主动断开连接 e0 00
const uint8_t parket_connetAck[] = {0x20,0x02,0x00,0x00};
const uint8_t parket_disconnet[] = {0xe0,0x00};
const uint8_t parket_heart[] = {0xc0,0x00};
const uint8_t parket_heart_reply[] = {0xc0,0x00};
const uint8_t parket_subAck[] = {0x90,0x03};

volatile uint16_t MQTT_TxLen;

//MQTT发送数据
void MQTT_SendBuf(uint8_t *buf,uint16_t len)
{
	ESP8266_ATSendBuf(buf,len);
}	

//发送心跳包
void MQTT_SentHeart(void)
{
	MQTT_SendBuf((uint8_t *)parket_heart,sizeof(parket_heart));
}

//MQTT无条件断开
void MQTT_Disconnect()
{
	MQTT_SendBuf((uint8_t *)parket_disconnet,sizeof(parket_disconnet));
}

//MQTT初始化
void MQTT_Init(uint8_t *prx,uint16_t rxlen,uint8_t *ptx,uint16_t txlen)
{
	memset(usart2_txbuf,0,sizeof(usart2_txbuf)); //清空发送缓冲
	memset(usart2_rxbuf,0,sizeof(usart2_rxbuf)); //清空接收缓冲
	
	//无条件先主动断开
	MQTT_Disconnect();HAL_Delay(100);
	MQTT_Disconnect();HAL_Delay(100);
}

//MQTT连接服务器的打包函数
uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password)
{
	int ClientIDLen = strlen(ClientID);
	int UsernameLen = strlen(Username);
	int PasswordLen = strlen(Password);
	int DataLen;
	MQTT_TxLen=0;
	//可变报头+Payload  每个字段包含两个字节的长度标识
  DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);
	
	//固定报头
	//控制报文类型
  usart2_txbuf[MQTT_TxLen++] = 0x10;		//MQTT Message Type CONNECT
	//剩余长度(不包括固定头部)
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		usart2_txbuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );
    	
	//可变报头
	//协议名
	usart2_txbuf[MQTT_TxLen++] = 0;        		// Protocol Name Length MSB    
	usart2_txbuf[MQTT_TxLen++] = 4;        		// Protocol Name Length LSB    
	usart2_txbuf[MQTT_TxLen++] = 'M';        	// ASCII Code for M    
	usart2_txbuf[MQTT_TxLen++] = 'Q';        	// ASCII Code for Q    
	usart2_txbuf[MQTT_TxLen++] = 'T';        	// ASCII Code for T    
	usart2_txbuf[MQTT_TxLen++] = 'T';        	// ASCII Code for T    
	//协议级别
	usart2_txbuf[MQTT_TxLen++] = 4;        		// MQTT Protocol version = 4    
	//连接标志
	usart2_txbuf[MQTT_TxLen++] = 0xc2;        	// conn flags 
	usart2_txbuf[MQTT_TxLen++] = 0;        		// Keep-alive Time Length MSB    
	usart2_txbuf[MQTT_TxLen++] = 60;        	// Keep-alive Time Length LSB  60S心跳包  

	usart2_txbuf[MQTT_TxLen++] = BYTE1(ClientIDLen);// Client ID length MSB    
	usart2_txbuf[MQTT_TxLen++] = BYTE0(ClientIDLen);// Client ID length LSB  	
	memcpy(&usart2_txbuf[MQTT_TxLen],ClientID,ClientIDLen);
	MQTT_TxLen += ClientIDLen;
	
	if(UsernameLen > 0)
	{   
		usart2_txbuf[MQTT_TxLen++] = BYTE1(UsernameLen);		//username length MSB    
		usart2_txbuf[MQTT_TxLen++] = BYTE0(UsernameLen);    	//username length LSB    
		memcpy(&usart2_txbuf[MQTT_TxLen],Username,UsernameLen);
		MQTT_TxLen += UsernameLen;
	}
	
	if(PasswordLen > 0)
	{    
		usart2_txbuf[MQTT_TxLen++] = BYTE1(PasswordLen);		//password length MSB    
		usart2_txbuf[MQTT_TxLen++] = BYTE0(PasswordLen);    	//password length LSB  
		memcpy(&usart2_txbuf[MQTT_TxLen],Password,PasswordLen);
		MQTT_TxLen += PasswordLen; 
	}    
	
	uint8_t cnt=2;
	uint8_t wait;
	while(cnt--)
	{
		memset(usart2_rxbuf,0,sizeof(usart2_rxbuf));
		MQTT_SendBuf(usart2_txbuf,MQTT_TxLen);
		wait=30;//等待3s时间
		while(wait--)
		{
			//CONNECT
			if(usart2_rxbuf[0]==parket_connetAck[0] && usart2_rxbuf[1]==parket_connetAck[1]) //连接成功			   
			{
				return 1;//连接成功
			}
			HAL_Delay(100);			
		}
	}
	return 0;
}

//MQTT订阅/取消订阅数据打包函数
//topic       主题 
//qos         消息等级 
//whether     订阅/取消订阅请求包
uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether)
{    
	MQTT_TxLen=0;
	int topiclen = strlen(topic);
	
	int DataLen = 2 + (topiclen+2) + (whether?1:0);//可变报头的长度（2字节）加上有效载荷的长度
	//固定报头
	//控制报文类型
	if(whether) usart2_txbuf[MQTT_TxLen++] = 0x82; //消息类型和标志订阅
	else	usart2_txbuf[MQTT_TxLen++] = 0xA2;    //取消订阅

	//剩余长度
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		usart2_txbuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );	
	
	//可变报头
	usart2_txbuf[MQTT_TxLen++] = 0;				//消息标识符 MSB
	usart2_txbuf[MQTT_TxLen++] = 0x01;           //消息标识符 LSB
	//有效载荷
	usart2_txbuf[MQTT_TxLen++] = BYTE1(topiclen);//主题长度 MSB
	usart2_txbuf[MQTT_TxLen++] = BYTE0(topiclen);//主题长度 LSB   
	memcpy(&usart2_txbuf[MQTT_TxLen],topic,topiclen);
	MQTT_TxLen += topiclen;

	if(whether)
	{
		usart2_txbuf[MQTT_TxLen++] = qos;//QoS级别
	}
	
	uint8_t cnt=2;
	uint8_t wait;
	while(cnt--)
	{
		memset(usart2_rxbuf,0,sizeof(usart2_rxbuf));
		MQTT_SendBuf(usart2_txbuf,MQTT_TxLen);
		wait=30;//等待3s时间
		while(wait--)
		{
			if(usart2_rxbuf[0]==parket_subAck[0] && usart2_rxbuf[1]==parket_subAck[1]) //订阅成功			   
			{
				return 1;//订阅成功
			}
			HAL_Delay(100);			
		}
	}
	if(cnt) return 1;	//订阅成功
	return 0;
}

//MQTT发布数据打包函数
//topic   主题 
//message 消息
//qos     消息等级 
uint8_t MQTT_PublishData(char *topic, char *message, uint8_t qos)
{  
	int topicLength = strlen(topic);    
	int messageLength = strlen(message);     
	static uint16_t id=0;
	int DataLen;
	MQTT_TxLen=0;
	//有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
	//QOS为0时没有标识符
	//数据长度             主题名   报文标识符   有效载荷
	if(qos)	DataLen = (2+topicLength) + 2 + messageLength;       
	else	DataLen = (2+topicLength) + messageLength;   

    //固定报头
	//控制报文类型
	usart2_txbuf[MQTT_TxLen++] = 0x30;    // MQTT Message Type PUBLISH  

	//剩余长度
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		usart2_txbuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );	
	
	usart2_txbuf[MQTT_TxLen++] = BYTE1(topicLength);//主题长度MSB
	usart2_txbuf[MQTT_TxLen++] = BYTE0(topicLength);//主题长度LSB 
	memcpy(&usart2_txbuf[MQTT_TxLen],topic,topicLength);//拷贝主题
	MQTT_TxLen += topicLength;
        
	//报文标识符
	if(qos)
	{
			usart2_txbuf[MQTT_TxLen++] = BYTE1(id);
			usart2_txbuf[MQTT_TxLen++] = BYTE0(id);
			id++;
	}
	memcpy(&usart2_txbuf[MQTT_TxLen],message,messageLength);
  MQTT_TxLen += messageLength;
        
	MQTT_SendBuf(usart2_txbuf,MQTT_TxLen);
  return MQTT_TxLen;
}

/******************************  进入错误模式代码  *****************************/

//进入错误模式等待手动重启
void Enter_ErrorMode(uint8_t mode)
{
	HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
	LCD_SetColor(0XffF6003C);						// 设置画笔
	while(1)
	{
		switch(mode){
			case 0:user_main_error("ESP8266初始化失败！\r\n");
					LCD_DisplayString(30,90,"init  fail");
				break;
			case 1:user_main_error("ESP8266连接热点失败！\r\n");
					LCD_DisplayString(30,170,"WIFI  fail");
				break;
			case 2:user_main_error("ESP8266连接阿里云服务器失败！\r\n");
					LCD_DisplayString(30,250,"connect  fail");
				break;
			case 3:user_main_error("ESP8266阿里云MQTT登陆失败！\r\n");
					LCD_DisplayString(30,330,"sign  fail");
				break;
			case 4:user_main_error("ESP8266阿里云MQTT订阅主题失败！\r\n");
					LCD_DisplayString(30,410,"subscribe  fail");
				break;
			default:user_main_info("Nothing\r\n");
				break;
		}
		user_main_info("请重启开发板");
		//HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin);
		HAL_Delay(200);
	}
}

//MQTT初始化函数
void ES8266_MQTT_Init(void)
{
	uint8_t status=0;
	LCD_SetFont(&Font32);
	LCD_SetColor(LCD_WHITE);
	
	//初始化
	if(ESP8266_Init())
	{
		user_main_info("ESP8266初始化成功！\r\n");
		LCD_DisplayString(30,50,"ESP8266_init  OK");
		status++;
	}
	else Enter_ErrorMode(0);
	
	//连接热点
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			user_main_info("ESP8266连接热点成功！\r\n");
			LCD_DisplayString(30,130,"ESP8266_WIFI  OK!");
			status++;
		}
		else Enter_ErrorMode(1);
	}
	
	//连接阿里云IOT服务器
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			user_main_info("ESP8266连接阿里云服务器成功！\r\n");
			LCD_DisplayString(30,210,"ESP8266_connect  OK!");
			status++;
		}
		else Enter_ErrorMode(2);
	}
	
	//登陆MQTT
	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			user_main_info("ESP8266阿里云MQTT登陆成功！\r\n");
			LCD_DisplayString(30,290,"ESP8266_Sign in  OK!");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//订阅主题
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			user_main_info("ESP8266阿里云MQTT订阅主题成功！\r\n");
			LCD_DisplayString(30,370,"ESP8266_subscribe  OK!");
		}
		else Enter_ErrorMode(4);
	}
	
//	LCD_SetBackColor(LCD_BLACK); 			//	设置背景色
//	LCD_Clear(); 											// 清屏
//	LCD_SetTextFont(&CH_Font32);					// 设置2424中文字体,ASCII字体对应为2412
//	LCD_SetColor(LCD_WHITE);						// 设置画笔,白色
//	LCD_DisplayText(336, 20,"环境监测");	// 显示文本	
//	LCD_SetColor(LIGHT_CYAN);					// 设置画笔，蓝绿色			
//	LCD_DisplayText(28,100,"实时温度：");
//	LCD_SetColor(LIGHT_YELLOW);				// 设置画笔，亮黄色		
//	LCD_DisplayText(400, 100,"内核温度：");	
//	LCD_DisplayText(12,150,"DHT11温度：");
//	LCD_SetColor(LIGHT_CYAN);					// 设置画笔，蓝绿色	
//	LCD_DisplayText(384, 150,"DHT11湿度：");	
//	LCD_DisplayText(28,200,"光照强度：");
//	LCD_SetColor(LIGHT_YELLOW);				// 设置画笔，亮黄色		
//	LCD_DisplayText(400, 200,"烟雾浓度：");	
}

//单片机状态上报
void StatusReport(void)
{
	
	int resss;
	int ecg_temp_buf_tx_count = 0;
	//上报一次数据
	uint8_t led_r_status = HAL_GPIO_ReadPin(LED1_GPIO_Port,LED1_Pin) ? 0:1;
	uint8_t led_g_status = HAL_GPIO_ReadPin(LED1_GPIO_Port,LED1_Pin) ? 0:1;
	uint8_t led_b_status = HAL_GPIO_ReadPin(LED1_GPIO_Port,LED1_Pin) ? 0:1;
//	sprintf(mqtt_message,
//	"{\"method\":\"thing.service.property.set\",\"id\":\"181454577\",\"params\":{\
//		\"temperature\":%.1f,\
//		\"HeartRate\":%d,\
//		\"RunningSteps\":%d,\
//		\"Switch_LEDB\":%d\
//	},\"version\":\"1.0.0\"}",
//	(float)temperature,
//	(int)heart_rate,
//	(int)RunningSteps,

//	led_b_status
//	);
		resss = 0;
		resss = sprintf(mqtt_message,
		"{\"method\":\"thing.service.property.set\",\"id\":\"181454577\",\"params\":{\
			\"temperature\":%.1f,\
			\"HeartRate\":%d,\
			\"RunningSteps\":%d,\
			\"Switch_LEDB\":%d\
		},\"sites\":[",
		(float)temperature,
		(int)heart_rate,
		(int)RunningSteps,

		led_b_status
		);
	
		

		for(ecg_temp_buf_tx_count=0;ecg_temp_buf_tx_count<600-1;ecg_temp_buf_tx_count++)
		{
			resss += sprintf(mqtt_message+resss,"%d,",ecg_temp_buf[ecg_temp_buf_tx_count]);
		}
		
		resss=sprintf(mqtt_message+resss,"%d],\"version\":\"1.0.0\"}",
		ecg_temp_buf[ecg_temp_buf_tx_count]);

	

	MQTT_PublishData(MQTT_PUBLISH_TOPIC,mqtt_message,0);
}

//处理MQTT下发的消息
void deal_MQTT_message(uint8_t* buf,uint16_t len)
{
//	uint8_t data[512];
//	uint16_t data_len = len;
//	for(int i=0;i<data_len;i++)
//	{
//		data[i] = buf[i];
//		HAL_UART_Transmit(&huart1,&data[i],1,200);
//	}
	
	HAL_UART_Transmit(&huart1,buf,len,200);
	
	memset(usart2_rxbuf,0,sizeof(usart2_rxbuf)); //清空接收缓冲  
	usart2_rxcounter=0;
	//user_main_info("MQTT收到消息,数据长度=%d \n",data_len);
	
	//查找是否是设置红灯
//	int i = GetSubStrPos((char*)data,"LEDR");
//	if( i>0 )
//	{
//		uint8_t ledr_status = data[i+6]-'0';
//		HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
//		HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
//		HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
//		if(ledr_status)
//			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_RESET);
//		else
//			HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
//	}
//	
//	//查找是否是设置绿灯
//	i = GetSubStrPos((char*)data,"LEDG");
//	if( i>0 )
//	{
//		uint8_t ledr_status = data[i+6]-'0';
//		HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
//		HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
//		HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
//		if(ledr_status)
//			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_RESET);
//		else
//			HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
//	}
//	
//	//查找是否是设置蓝灯
//	i = GetSubStrPos((char*)data,"LEDB");
//	if( i>0 )
//	{
//		uint8_t ledr_status = data[i+6]-'0';
//		HAL_GPIO_WritePin(LED_R_GPIO_Port,LED_R_Pin,GPIO_PIN_SET);
//		HAL_GPIO_WritePin(LED_G_GPIO_Port,LED_G_Pin,GPIO_PIN_SET);
//		HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
//		if(ledr_status)
//			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_RESET);
//		else
//			HAL_GPIO_WritePin(LED_B_GPIO_Port,LED_B_Pin,GPIO_PIN_SET);
//	}

}

