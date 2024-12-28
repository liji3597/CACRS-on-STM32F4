#ifndef __DHT11_H
#define __DHT11_H 
#include "sys.h"   

 
//IO��������
#define DHT11_IO_IN()  {GPIOC->MODER&=~(3<<(11*2));GPIOC->MODER|=0<<(11*2);}	//PB12����ģʽ
#define DHT11_IO_OUT() {GPIOC->MODER&=~(3<<(11*2));GPIOC->MODER|=1<<(11*2);} 	//PB12���ģʽ
 
////IO��������											   
#define	DHT11_DQ_OUT PCout(11) //���ݶ˿�	PC11
#define	DHT11_DQ_IN  PCin(11)  //���ݶ˿�	PC11


u8 DHT11_Init(void);//��ʼ��DHT11
u8 DHT11_Read_Data(u8 *temp,u8 *humi);//��ȡ��ʪ��
u8 DHT11_Read_Byte(void);//����һ���ֽ�
u8 DHT11_Read_Bit(void);//����һ��λ
u8 DHT11_Check(void);//����Ƿ����DHT11
void DHT11_Rst(void);//��λDHT11    
#endif















