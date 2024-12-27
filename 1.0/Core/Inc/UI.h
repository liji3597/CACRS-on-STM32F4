#ifndef __UI_H__
#define __UI_H__
#include "callback.h"

typedef struct
{
	uint16_t loca_x;	//窗口起点x坐标
	uint16_t loca_y;	//窗口起点y坐标
	uint16_t length;	//窗口长度
	uint16_t width;		//窗口宽度
	uint16_t cursor_x;		//当前刷新位置x坐标
	uint16_t last_cursor_x;	//上次刷新位置x坐标
	uint16_t cursor_y;		//当前刷新位置y坐标
	uint16_t last_cursor_y;	//上次刷新位置y坐标
	int32_t y_max;		//y值历史最大值
	int32_t y_min;		//y值历史最小值
	float y_ratio;		//y轴缩放比例
	int32_t y_max_last;		//上一页y值历史最大值
	int32_t y_min_last;		//上一页y值历史最大值
	
}osc_window;

void ECG_UI_init(osc_window* win1,uint16_t loca_x_set,uint16_t loca_y_set,uint16_t length_set,uint16_t width_set);
void ECG_UI_refresh(osc_window* win1, uint32_t x_raw,int32_t y_raw);

#endif

