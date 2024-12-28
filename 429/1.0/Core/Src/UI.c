#include "lcd_rgb.h"
#include "UI.h"
#include "ecg_ana.h"

/*****示波器界面部分，包括初始化、更新数据点、波形显示****/


//示波器窗口初始化
void ECG_UI_init(osc_window* win1,uint16_t loca_x_set,uint16_t loca_y_set,uint16_t length_set,uint16_t width_set)
{
	//设置示波器窗口的基本参数
	win1->length = length_set-4;
	win1->width =  width_set-4;
	win1->loca_x = loca_x_set+2;
	win1->loca_y = loca_y_set+2;
	win1->y_min = 0;
	win1->y_max = 0;
	win1->y_min_last = 0;
	win1->y_max_last = 0;
	win1->y_ratio = 0;
	//画示波器边框
	LCD_SetColor(LCD_WHITE);		//	设置画笔色
	LCD_DrawLine(loca_x_set,loca_y_set,loca_x_set + length_set,loca_y_set);
	LCD_DrawLine(loca_x_set,loca_y_set + width_set,loca_x_set + length_set,loca_y_set + width_set);
	LCD_DrawLine(loca_x_set,loca_y_set,loca_x_set,loca_y_set + width_set);
	LCD_DrawLine(loca_x_set + length_set,loca_y_set,loca_x_set + length_set,loca_y_set + width_set);	
	//显示固定的信息
//	LCD_ShowString(0,130,"hr:",WHITE,BLACK,32,0);
//	LCD_ShowString(80,130,"tp:",WHITE,BLACK,32,0);
//	LCD_ShowString(176,130,"pc:",WHITE,BLACK,32,0);
//	LCD_ShowString(256,130,"dt:",WHITE,BLACK,32,0);
}

//示波器刷新显示：按照心电波形刷新特点自己写的，仅供参考，不一定适用于真实的示波器显示
//特点：幅值自适应，会对波形进行幅值缩放，始终让波形保持在窗口中。

void ECG_UI_refresh(osc_window* win1, uint32_t x_raw,int32_t y_raw)
{
	win1->cursor_x = x_raw % win1->length + win1->loca_x;	//计算当前刷新点的横坐标：传入值%总长度 + 窗口起点坐标
	
	if(win1->cursor_x < win1->last_cursor_x)	//如果开始新的一页
	{
		win1->y_max_last = win1->y_max;
		win1->y_min_last = win1->y_min;
		win1->y_min = y_raw;	//重置y轴最小值
		win1->y_max = y_raw;	//重置y轴最大值
		win1->y_ratio = (float)win1->width/(float)(win1->y_max_last-win1->y_min_last);	//更新y轴缩放，由上一页的最大值和最小值算出
		win1->y_ratio -= win1->y_ratio*0.4;
	}
	
	if(y_raw < win1->y_min)
	{
		win1->y_min = y_raw;	//更新y轴历史最小值
	}
	if(y_raw > win1->y_max)
	{
		win1->y_max = y_raw;	//更新y轴历史最大值
	}
	
	if(win1->y_ratio > 0)	//跳过首次刷新
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
//		LCD_DrawLine(win1->cursor_x,win1->loca_y,win1->cursor_x,win1->loca_y+win1->width,BLACK);//清空上次波形
		LCD_SetColor(LCD_BLACK);		//	设置画笔色LCD_GREEN
		LCD_FillRect(win1->cursor_x,win1->loca_y,1,win1->width+1);	//清空上次波形(效率比上面的高)

		if(win1->cursor_x > win1->last_cursor_x)
		{
//			if(win1->last_cursor_y <= win1->cursor_y)	//画线函数有缺陷，需要处理成终点>=起点的形式
//			{
				LCD_SetColor(LCD_GREEN);		//	设置画笔色
				LCD_DrawLine(win1->last_cursor_x,win1->last_cursor_y,win1->cursor_x,win1->cursor_y);				
//			}
//			else
//			{
//				LCD_SetColor(LCD_GREEN);		//	设置画笔色
//				LCD_DrawLine(win1->last_cursor_x,win1->cursor_y,win1->cursor_x,win1->last_cursor_y);				
//			}
//			
			//分析波形(计算心率)
			ecg_analyse(win1,x_raw,0);	
		}
	}
	win1->last_cursor_x = win1->cursor_x;	//更新上次的刷新x坐标
	win1->last_cursor_y = win1->cursor_y;	//更新上次的刷新y坐标
}
