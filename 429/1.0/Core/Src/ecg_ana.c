#include "ECG_ANA.h"
//#include "lcd.h"
//#include "tcp_client.h"

/***********************
ECG_ANA���ĵ��������ҪΪ���ʼ���
***********************/

//�������2ms	ÿ����������ʱ������2*ecg_x_ratio��Ĭ��ecg_x_ratio = 5��
//����ʹ�õ��ĵ�ģ�������������ڣ�760ms������ 79��/min��ÿ������ռ��������760/(2*ecg_x_ratio)
//����ʹ�õ�ģ���� ECG(R��)���ֵ 3.0-3.6mV 
//����ʹ�õ�ģ���� R����ȣ�75ms
//������ QRS���������ڣ�60~100ms
//���������ʣ�60-120

uint16_t ecg_wave_buf[320];	//���ڻ�����ֵ�Ļ�����
uint32_t ecg_wave_sum;		//������ֵ��͵���ʱ����
uint32_t ecg_wave_sum_count;//��ͼ���
uint16_t ecg_wave_avr = 0;	//��ֵ
uint16_t ecg_wave_R_threshold = 0;	//���ڼ��R������ֵ

extern __IO uint32_t sys_tick;
uint32_t R_tick = 0;			//���Ƽ��R�������ڣ���ֹ����

extern uint8_t heart_rate;		//����
uint8_t heart_rate_tmp = 0;		//������ʱ����
uint16_t heart_rate_sum = 0;	//�����ۼӺ�
uint8_t heart_rate_sum_count = 0;	//�����ۼӼ���
uint8_t heart_rate_err_flag = 0;	//���ʴ����־λ

//ʹ�ù�ȥ320����Ļ�����ֵ���R��ʵ�����ʲ���
void ecg_analyse(osc_window* win1,uint32_t time,uint8_t time_ratio)
{
	ecg_wave_buf[ecg_wave_sum_count++] = win1->cursor_y;	//��¼yֵ
	if(ecg_wave_sum_count >= 320)
	{
		ecg_wave_sum_count = 0;
	}
	ecg_wave_sum -= ecg_wave_buf[ecg_wave_sum_count];	//ȥ��������β��ֵ
	ecg_wave_sum += win1->cursor_y;				//�ۼ�ͷ��ֵ
	ecg_wave_avr = ecg_wave_sum/320.0;				//��ƽ��
	

	ecg_wave_R_threshold = ecg_wave_avr-(float)ecg_wave_avr*0.3;//R����ֵ = 1.3*��ֵ

		if(ecg_wave_R_threshold < win1->loca_y)	//�޷�
	{
		ecg_wave_R_threshold = win1->loca_y;
	}
	if(ecg_wave_R_threshold > win1->loca_y+win1->width)	//�޷�
	{
		ecg_wave_R_threshold = win1->loca_y+win1->width;
	}

	
	LCD_DrawPoint(win1->cursor_x,ecg_wave_R_threshold,LCD_GREEN);		//����R����ֵ��
	
	if(win1->cursor_y < ecg_wave_R_threshold && sys_tick-R_tick>400)	//��ʾ����
	{
		heart_rate_tmp = 60000/(sys_tick - R_tick);	//��������
		if(heart_rate_tmp > 50 && heart_rate_tmp < 120)	//������50-120֮�����Ч
		{
			heart_rate_err_flag = 0;	//������Ч
			heart_rate_sum += heart_rate_tmp;
			heart_rate_sum_count++;
			if(heart_rate_sum_count >= 3)	//��4�����ʾ�ֵ
			{
				heart_rate = heart_rate_sum/heart_rate_sum_count;
				heart_rate_sum = 0;
				heart_rate_sum_count = 0;
			}
		}
		else
		{
			//��������3����Ч��������ʱ���ж�Ϊ������
			heart_rate_err_flag++;
			if(heart_rate_err_flag >= 3)
			{
				heart_rate = 0;
				heart_rate_sum = 0;
				heart_rate_sum_count = 0;
			}
		}
		R_tick = sys_tick;
	}
	else if(sys_tick-R_tick>20000)	//����2sδ��⵽��Ч����
	{
//		LCD_ShowIntNum(0,140,0,3,GREEN,BLACK,32);	//��ʾ0
			LCD_DisplayNumber( 718, 101, 0, 3);		// ��ʾС��;
	}
}

