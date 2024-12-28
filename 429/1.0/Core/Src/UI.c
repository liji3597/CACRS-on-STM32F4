#include "lcd_rgb.h"
#include "UI.h"
#include "ecg_ana.h"

/*****ʾ�������沿�֣�������ʼ�����������ݵ㡢������ʾ****/


//ʾ�������ڳ�ʼ��
void ECG_UI_init(osc_window* win1,uint16_t loca_x_set,uint16_t loca_y_set,uint16_t length_set,uint16_t width_set)
{
	//����ʾ�������ڵĻ�������
	win1->length = length_set-4;
	win1->width =  width_set-4;
	win1->loca_x = loca_x_set+2;
	win1->loca_y = loca_y_set+2;
	win1->y_min = 0;
	win1->y_max = 0;
	win1->y_min_last = 0;
	win1->y_max_last = 0;
	win1->y_ratio = 0;
	//��ʾ�����߿�
	LCD_SetColor(LCD_WHITE);		//	���û���ɫ
	LCD_DrawLine(loca_x_set,loca_y_set,loca_x_set + length_set,loca_y_set);
	LCD_DrawLine(loca_x_set,loca_y_set + width_set,loca_x_set + length_set,loca_y_set + width_set);
	LCD_DrawLine(loca_x_set,loca_y_set,loca_x_set,loca_y_set + width_set);
	LCD_DrawLine(loca_x_set + length_set,loca_y_set,loca_x_set + length_set,loca_y_set + width_set);	
	//��ʾ�̶�����Ϣ
//	LCD_ShowString(0,130,"hr:",WHITE,BLACK,32,0);
//	LCD_ShowString(80,130,"tp:",WHITE,BLACK,32,0);
//	LCD_ShowString(176,130,"pc:",WHITE,BLACK,32,0);
//	LCD_ShowString(256,130,"dt:",WHITE,BLACK,32,0);
}

//ʾ����ˢ����ʾ�������ĵ粨��ˢ���ص��Լ�д�ģ������ο�����һ����������ʵ��ʾ������ʾ
//�ص㣺��ֵ����Ӧ����Բ��ν��з�ֵ���ţ�ʼ���ò��α����ڴ����С�

void ECG_UI_refresh(osc_window* win1, uint32_t x_raw,int32_t y_raw)
{
	win1->cursor_x = x_raw % win1->length + win1->loca_x;	//���㵱ǰˢ�µ�ĺ����꣺����ֵ%�ܳ��� + �����������
	
	if(win1->cursor_x < win1->last_cursor_x)	//�����ʼ�µ�һҳ
	{
		win1->y_max_last = win1->y_max;
		win1->y_min_last = win1->y_min;
		win1->y_min = y_raw;	//����y����Сֵ
		win1->y_max = y_raw;	//����y�����ֵ
		win1->y_ratio = (float)win1->width/(float)(win1->y_max_last-win1->y_min_last);	//����y�����ţ�����һҳ�����ֵ����Сֵ���
		win1->y_ratio -= win1->y_ratio*0.4;
	}
	
	if(y_raw < win1->y_min)
	{
		win1->y_min = y_raw;	//����y����ʷ��Сֵ
	}
	if(y_raw > win1->y_max)
	{
		win1->y_max = y_raw;	//����y����ʷ���ֵ
	}
	
	if(win1->y_ratio > 0)	//�����״�ˢ��
	{
		win1->cursor_y = (win1->loca_y + (float)win1->width*0.8) - (float)(y_raw-win1->y_min_last)*win1->y_ratio;
		if(win1->cursor_y < win1->loca_y)
		{
			win1->cursor_y = win1->loca_y;
		}
		if(win1->cursor_y > win1->loca_y+win1->width)
		{
			win1->cursor_y = win1->loca_y+win1->width;
		}
//		LCD_DrawLine(win1->cursor_x,win1->loca_y,win1->cursor_x,win1->loca_y+win1->width,BLACK);//����ϴβ���
		LCD_SetColor(LCD_BLACK);		//	���û���ɫLCD_GREEN
		LCD_FillRect(win1->cursor_x,win1->loca_y,1,win1->width+1);	//����ϴβ���(Ч�ʱ�����ĸ�)

		if(win1->cursor_x > win1->last_cursor_x)
		{
//			if(win1->last_cursor_y <= win1->cursor_y)	//���ߺ�����ȱ�ݣ���Ҫ������յ�>=������ʽ
//			{
				LCD_SetColor(LCD_GREEN);		//	���û���ɫ
				LCD_DrawLine(win1->last_cursor_x,win1->last_cursor_y,win1->cursor_x,win1->cursor_y);				
//			}
//			else
//			{
//				LCD_SetColor(LCD_GREEN);		//	���û���ɫ
//				LCD_DrawLine(win1->last_cursor_x,win1->cursor_y,win1->cursor_x,win1->last_cursor_y);				
//			}
//			
			//��������(��������)
			ecg_analyse(win1,x_raw,0);	
		}
	}
	win1->last_cursor_x = win1->cursor_x;	//�����ϴε�ˢ��x����
	win1->last_cursor_y = win1->cursor_y;	//�����ϴε�ˢ��y����
}
