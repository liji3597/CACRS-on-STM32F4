#include "esp8266_mqtt.h"
#include "esp8266_at.h"
#include "callback.h"

char mqtt_message[2700];	//MQTT���ϱ���Ϣ����

//���ӳɹ���������Ӧ 20 02 00 00
//�ͻ��������Ͽ����� e0 00
const uint8_t parket_connetAck[] = {0x20,0x02,0x00,0x00};
const uint8_t parket_disconnet[] = {0xe0,0x00};
const uint8_t parket_heart[] = {0xc0,0x00};
const uint8_t parket_heart_reply[] = {0xc0,0x00};
const uint8_t parket_subAck[] = {0x90,0x03};

volatile uint16_t MQTT_TxLen;

//MQTT��������
void MQTT_SendBuf(uint8_t *buf,uint16_t len)
{
	ESP8266_ATSendBuf(buf,len);
}	

//����������
void MQTT_SentHeart(void)
{
	MQTT_SendBuf((uint8_t *)parket_heart,sizeof(parket_heart));
}

//MQTT�������Ͽ�
void MQTT_Disconnect()
{
	MQTT_SendBuf((uint8_t *)parket_disconnet,sizeof(parket_disconnet));
}

//MQTT��ʼ��
void MQTT_Init(uint8_t *prx,uint16_t rxlen,uint8_t *ptx,uint16_t txlen)
{
	memset(usart2_txbuf,0,sizeof(usart2_txbuf)); //��շ��ͻ���
	memset(usart2_rxbuf,0,sizeof(usart2_rxbuf)); //��ս��ջ���
	
	//�������������Ͽ�
	MQTT_Disconnect();HAL_Delay(100);
	MQTT_Disconnect();HAL_Delay(100);
}

//MQTT���ӷ������Ĵ������
uint8_t MQTT_Connect(char *ClientID,char *Username,char *Password)
{
	int ClientIDLen = strlen(ClientID);
	int UsernameLen = strlen(Username);
	int PasswordLen = strlen(Password);
	int DataLen;
	MQTT_TxLen=0;
	//�ɱ䱨ͷ+Payload  ÿ���ֶΰ��������ֽڵĳ��ȱ�ʶ
  DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);
	
	//�̶���ͷ
	//���Ʊ�������
  usart2_txbuf[MQTT_TxLen++] = 0x10;		//MQTT Message Type CONNECT
	//ʣ�೤��(�������̶�ͷ��)
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		usart2_txbuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );
    	
	//�ɱ䱨ͷ
	//Э����
	usart2_txbuf[MQTT_TxLen++] = 0;        		// Protocol Name Length MSB    
	usart2_txbuf[MQTT_TxLen++] = 4;        		// Protocol Name Length LSB    
	usart2_txbuf[MQTT_TxLen++] = 'M';        	// ASCII Code for M    
	usart2_txbuf[MQTT_TxLen++] = 'Q';        	// ASCII Code for Q    
	usart2_txbuf[MQTT_TxLen++] = 'T';        	// ASCII Code for T    
	usart2_txbuf[MQTT_TxLen++] = 'T';        	// ASCII Code for T    
	//Э�鼶��
	usart2_txbuf[MQTT_TxLen++] = 4;        		// MQTT Protocol version = 4    
	//���ӱ�־
	usart2_txbuf[MQTT_TxLen++] = 0xc2;        	// conn flags 
	usart2_txbuf[MQTT_TxLen++] = 0;        		// Keep-alive Time Length MSB    
	usart2_txbuf[MQTT_TxLen++] = 60;        	// Keep-alive Time Length LSB  60S������  

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
		wait=30;//�ȴ�3sʱ��
		while(wait--)
		{
			//CONNECT
			if(usart2_rxbuf[0]==parket_connetAck[0] && usart2_rxbuf[1]==parket_connetAck[1]) //���ӳɹ�			   
			{
				return 1;//���ӳɹ�
			}
			HAL_Delay(100);			
		}
	}
	return 0;
}

//MQTT����/ȡ���������ݴ������
//topic       ���� 
//qos         ��Ϣ�ȼ� 
//whether     ����/ȡ�����������
uint8_t MQTT_SubscribeTopic(char *topic,uint8_t qos,uint8_t whether)
{    
	MQTT_TxLen=0;
	int topiclen = strlen(topic);
	
	int DataLen = 2 + (topiclen+2) + (whether?1:0);//�ɱ䱨ͷ�ĳ��ȣ�2�ֽڣ�������Ч�غɵĳ���
	//�̶���ͷ
	//���Ʊ�������
	if(whether) usart2_txbuf[MQTT_TxLen++] = 0x82; //��Ϣ���ͺͱ�־����
	else	usart2_txbuf[MQTT_TxLen++] = 0xA2;    //ȡ������

	//ʣ�೤��
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		usart2_txbuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );	
	
	//�ɱ䱨ͷ
	usart2_txbuf[MQTT_TxLen++] = 0;				//��Ϣ��ʶ�� MSB
	usart2_txbuf[MQTT_TxLen++] = 0x01;           //��Ϣ��ʶ�� LSB
	//��Ч�غ�
	usart2_txbuf[MQTT_TxLen++] = BYTE1(topiclen);//���ⳤ�� MSB
	usart2_txbuf[MQTT_TxLen++] = BYTE0(topiclen);//���ⳤ�� LSB   
	memcpy(&usart2_txbuf[MQTT_TxLen],topic,topiclen);
	MQTT_TxLen += topiclen;

	if(whether)
	{
		usart2_txbuf[MQTT_TxLen++] = qos;//QoS����
	}
	
	uint8_t cnt=2;
	uint8_t wait;
	while(cnt--)
	{
		memset(usart2_rxbuf,0,sizeof(usart2_rxbuf));
		MQTT_SendBuf(usart2_txbuf,MQTT_TxLen);
		wait=30;//�ȴ�3sʱ��
		while(wait--)
		{
			if(usart2_rxbuf[0]==parket_subAck[0] && usart2_rxbuf[1]==parket_subAck[1]) //���ĳɹ�			   
			{
				return 1;//���ĳɹ�
			}
			HAL_Delay(100);			
		}
	}
	if(cnt) return 1;	//���ĳɹ�
	return 0;
}

//MQTT�������ݴ������
//topic   ���� 
//message ��Ϣ
//qos     ��Ϣ�ȼ� 
uint8_t MQTT_PublishData(char *topic, char *message, uint8_t qos)
{  
	int topicLength = strlen(topic);    
	int messageLength = strlen(message);     
	static uint16_t id=0;
	int DataLen;
	MQTT_TxLen=0;
	//��Ч�غɵĳ����������㣺�ù̶���ͷ�е�ʣ�೤���ֶε�ֵ��ȥ�ɱ䱨ͷ�ĳ���
	//QOSΪ0ʱû�б�ʶ��
	//���ݳ���             ������   ���ı�ʶ��   ��Ч�غ�
	if(qos)	DataLen = (2+topicLength) + 2 + messageLength;       
	else	DataLen = (2+topicLength) + messageLength;   

    //�̶���ͷ
	//���Ʊ�������
	usart2_txbuf[MQTT_TxLen++] = 0x30;    // MQTT Message Type PUBLISH  

	//ʣ�೤��
	do
	{
		uint8_t encodedByte = DataLen % 128;
		DataLen = DataLen / 128;
		// if there are more data to encode, set the top bit of this byte
		if ( DataLen > 0 )
			encodedByte = encodedByte | 128;
		usart2_txbuf[MQTT_TxLen++] = encodedByte;
	}while ( DataLen > 0 );	
	
	usart2_txbuf[MQTT_TxLen++] = BYTE1(topicLength);//���ⳤ��MSB
	usart2_txbuf[MQTT_TxLen++] = BYTE0(topicLength);//���ⳤ��LSB 
	memcpy(&usart2_txbuf[MQTT_TxLen],topic,topicLength);//��������
	MQTT_TxLen += topicLength;
        
	//���ı�ʶ��
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

/******************************  �������ģʽ����  *****************************/

//�������ģʽ�ȴ��ֶ�����
void Enter_ErrorMode(uint8_t mode)
{
	HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
	LCD_SetColor(0XffF6003C);						// ���û���
	while(1)
	{
		switch(mode){
			case 0:user_main_error("ESP8266��ʼ��ʧ�ܣ�\r\n");
					LCD_DisplayString(30,90,"init  fail");
				break;
			case 1:user_main_error("ESP8266�����ȵ�ʧ�ܣ�\r\n");
					LCD_DisplayString(30,170,"WIFI  fail");
				break;
			case 2:user_main_error("ESP8266���Ӱ����Ʒ�����ʧ�ܣ�\r\n");
					LCD_DisplayString(30,250,"connect  fail");
				break;
			case 3:user_main_error("ESP8266������MQTT��½ʧ�ܣ�\r\n");
					LCD_DisplayString(30,330,"sign  fail");
				break;
			case 4:user_main_error("ESP8266������MQTT��������ʧ�ܣ�\r\n");
					LCD_DisplayString(30,410,"subscribe  fail");
				break;
			default:user_main_info("Nothing\r\n");
				break;
		}
		user_main_info("������������");
		//HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin);
		HAL_Delay(200);
	}
}

//MQTT��ʼ������
void ES8266_MQTT_Init(void)
{
	uint8_t status=0;
	LCD_SetFont(&Font32);
	LCD_SetColor(LCD_WHITE);
	
	//��ʼ��
	if(ESP8266_Init())
	{
		user_main_info("ESP8266��ʼ���ɹ���\r\n");
		LCD_DisplayString(30,50,"ESP8266_init  OK");
		status++;
	}
	else Enter_ErrorMode(0);
	
	//�����ȵ�
	if(status==1)
	{
		if(ESP8266_ConnectAP(WIFI_NAME,WIFI_PASSWD))
		{
			user_main_info("ESP8266�����ȵ�ɹ���\r\n");
			LCD_DisplayString(30,130,"ESP8266_WIFI  OK!");
			status++;
		}
		else Enter_ErrorMode(1);
	}
	
	//���Ӱ�����IOT������
	if(status==2)
	{
		if(ESP8266_ConnectServer("TCP",MQTT_BROKERADDRESS,1883)!=0)
		{
			user_main_info("ESP8266���Ӱ����Ʒ������ɹ���\r\n");
			LCD_DisplayString(30,210,"ESP8266_connect  OK!");
			status++;
		}
		else Enter_ErrorMode(2);
	}
	
	//��½MQTT
	if(status==3)
	{
		if(MQTT_Connect(MQTT_CLIENTID, MQTT_USARNAME, MQTT_PASSWD) != 0)
		{
			user_main_info("ESP8266������MQTT��½�ɹ���\r\n");
			LCD_DisplayString(30,290,"ESP8266_Sign in  OK!");
			status++;
		}
		else Enter_ErrorMode(3);
	}

	//��������
	if(status==4)
	{
		if(MQTT_SubscribeTopic(MQTT_SUBSCRIBE_TOPIC,0,1) != 0)
		{
			user_main_info("ESP8266������MQTT��������ɹ���\r\n");
			LCD_DisplayString(30,370,"ESP8266_subscribe  OK!");
		}
		else Enter_ErrorMode(4);
	}
	
//	LCD_SetBackColor(LCD_BLACK); 			//	���ñ���ɫ
//	LCD_Clear(); 											// ����
//	LCD_SetTextFont(&CH_Font32);					// ����2424��������,ASCII�����ӦΪ2412
//	LCD_SetColor(LCD_WHITE);						// ���û���,��ɫ
//	LCD_DisplayText(336, 20,"�������");	// ��ʾ�ı�	
//	LCD_SetColor(LIGHT_CYAN);					// ���û��ʣ�����ɫ			
//	LCD_DisplayText(28,100,"ʵʱ�¶ȣ�");
//	LCD_SetColor(LIGHT_YELLOW);				// ���û��ʣ�����ɫ		
//	LCD_DisplayText(400, 100,"�ں��¶ȣ�");	
//	LCD_DisplayText(12,150,"DHT11�¶ȣ�");
//	LCD_SetColor(LIGHT_CYAN);					// ���û��ʣ�����ɫ	
//	LCD_DisplayText(384, 150,"DHT11ʪ�ȣ�");	
//	LCD_DisplayText(28,200,"����ǿ�ȣ�");
//	LCD_SetColor(LIGHT_YELLOW);				// ���û��ʣ�����ɫ		
//	LCD_DisplayText(400, 200,"����Ũ�ȣ�");	
}

//��Ƭ��״̬�ϱ�
void StatusReport(void)
{
	
	int resss;
	int ecg_temp_buf_tx_count = 0;
	//�ϱ�һ������
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

//����MQTT�·�����Ϣ
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
	
	memset(usart2_rxbuf,0,sizeof(usart2_rxbuf)); //��ս��ջ���  
	usart2_rxcounter=0;
	//user_main_info("MQTT�յ���Ϣ,���ݳ���=%d \n",data_len);
	
	//�����Ƿ������ú��
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
//	//�����Ƿ��������̵�
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
//	//�����Ƿ�����������
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

