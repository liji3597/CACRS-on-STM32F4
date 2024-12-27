#ifndef __UI_H__
#define __UI_H__
#include "callback.h"

typedef struct
{
	uint16_t loca_x;	//�������x����
	uint16_t loca_y;	//�������y����
	uint16_t length;	//���ڳ���
	uint16_t width;		//���ڿ��
	uint16_t cursor_x;		//��ǰˢ��λ��x����
	uint16_t last_cursor_x;	//�ϴ�ˢ��λ��x����
	uint16_t cursor_y;		//��ǰˢ��λ��y����
	uint16_t last_cursor_y;	//�ϴ�ˢ��λ��y����
	int32_t y_max;		//yֵ��ʷ���ֵ
	int32_t y_min;		//yֵ��ʷ��Сֵ
	float y_ratio;		//y�����ű���
	int32_t y_max_last;		//��һҳyֵ��ʷ���ֵ
	int32_t y_min_last;		//��һҳyֵ��ʷ���ֵ
	
}osc_window;

void ECG_UI_init(osc_window* win1,uint16_t loca_x_set,uint16_t loca_y_set,uint16_t length_set,uint16_t width_set);
void ECG_UI_refresh(osc_window* win1, uint32_t x_raw,int32_t y_raw);

#endif

