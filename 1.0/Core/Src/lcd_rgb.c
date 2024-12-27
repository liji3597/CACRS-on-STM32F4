/**
  ******************************************************************************
  *	@file  		lcd_rgb.c
  *	@version 	V1.0
  * @date    	2021-06-01
  *	@author  	֪�пƼ�
  *	@brief   	����RGB��ʾ��������ʾ
  ******************************************************************************
  * @attention
  *
  *	ʵ��ƽ̨��STM32F429IGT6���İ�(�ͺţ�FK429M2) + 800*480�ֱ��ʵ�RGB��Ļ
  *
>>>>> ��Ҫ˵����
  *
  *	1. 429M2 ���İ� ʹ�õ����ⲿSDRAM��Ϊ�Դ棬��ʼ��ַ0xD0000000, SDRAM ��СΪ32M�ֽ�
  *	2. �����ǵ�����ʾ����˫����ʾ�������ܳ��� SDRAM �Ĵ�С
  *	3. �ڸ����������ʱ����Ļ����΢��������˸�����������󣬵ȴ�Ƭ�̻��������ϵ缴�ɻָ�����
  * 4. LTDCʱ���� lcd_rgb.h �ļ���ĺ� LCD_CLK ���ã�����Ϊ33MHz����ˢ������60֡���ң����߻��߹��Ͷ��������˸	
  *
>>>>> ����˵����
  *
  *	1. �����ֿ�ʹ�õ���С�ֿ⣬���õ��˶�Ӧ�ĺ�����ȥȡģ���û����Ը����������������ɾ��
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lcd_rgb.h"

/* Private variables ---------------------------------------------------------*/
static pFONT *LCD_Fonts;		// Ӣ������
static pFONT *LCD_CHFonts;		// ��������

//LCD��ز����ṹ��
struct	
{
	uint32_t Color; 				//	LCD��ǰ������ɫ
	uint32_t BackColor;			//	����ɫ
	uint32_t ColorMode;			//	��ɫ��ʽ
	uint32_t LayerMemoryAdd;	//	���Դ��ַ
	uint8_t  BytesPerPixel;		//	ÿ��������ռ�ֽ���	
	uint8_t  Layer; 				//	��ǰ��
	uint8_t  Direction;			//	��ʾ����
	uint8_t  ShowNum_Mode;		// ���ñ�����ʾʱ����λ��0���ǲ��ո�
}LCD;

/* Private user code ---------------------------------------------------------*/

/*************************************************************************************************
*	�� �� ��:	LCD_SetLayer
*	��ڲ���:	layer - Ҫ��ʾ�Ͳ����Ĳ㣬��������Ϊ0��1����ѡ�� layer0 �� layer1
*	�� �� ֵ:	��
*	��������:	����Ҫ��ʾ�Ͳ����Ĳ㣬�л���Ӧ���Դ��ַ����ɫ��ʽ��
*	˵    ��:	LTDC��˳���ǹ̶��ģ�layer1 �� layer0֮�ϣ�������������ʾʱ��
*					layer1 ��ǰ���㣬ͨ��ʹ�ô�͸��ɫ����ɫ��ʽ��layer0 �Ǳ����㣬
*					ֻ��������ʱ��Ĭ��ֻ���� layer0
*************************************************************************************************/

void LCD_SetLayer(uint8_t layer)
{
#if LCD_NUM_LAYERS == 2		// �������˫��
	
	if (layer == 0)			// ������õ��� layer0
	{
		LCD.LayerMemoryAdd = LCD_MemoryAdd; 	// ��ȡ layer0 ���Դ��ַ
		LCD.ColorMode      = ColorMode_0;		// ��ȡ layer0 ����ɫ��ʽ
		LCD.BytesPerPixel  = BytesPerPixel_0;	// ��ȡ layer0 ��ÿ�����������ֽ����Ĵ�С
	}
	else if(layer == 1)	 // ������õ��� layer1
	{
		LCD.LayerMemoryAdd = LCD_MemoryAdd + LCD_MemoryAdd_OFFSET;	// ��ȡ layer1 ���Դ��ַ
		LCD.ColorMode      = ColorMode_1;                           // ��ȡ layer1 ����ɫ��ʽ
		LCD.BytesPerPixel  = BytesPerPixel_1;		                  // ��ȡ layer1 ��ÿ�����������ֽ����Ĵ�С
	}
	LCD.Layer = layer;	//��¼��ǰ���ڵĲ�
	
#else		// ���ֻ�������㣬Ĭ�ϲ��� layer0
	
	LCD.LayerMemoryAdd = LCD_MemoryAdd;		// ��ȡ layer0 ���Դ��ַ
	LCD.ColorMode      = ColorMode_0;      // ��ȡ layer0 ����ɫ��ʽ
	LCD.BytesPerPixel  = BytesPerPixel_0;	// ��ȡ layer0 ��ÿ�����������ֽ����Ĵ�С
	LCD.Layer = 0;		// ��������Ϊ layer0
	
#endif

}  

/***************************************************************************************************************
*	�� �� ��:	LCD_SetColor
*
*	��ڲ���:	Color - Ҫ��ʾ����ɫ��ʾ����0xff0000FF ��ʾ��͸������ɫ��0xAA0000FF ��ʾ͸����Ϊ66.66%����ɫ
*
*	��������:	�˺�������������ʾ�ַ������㻭�ߡ���ͼ����ɫ
*
*	˵    ��:	1. Ϊ�˷����û�ʹ���Զ�����ɫ����ڲ��� Color ʹ��32λ����ɫ��ʽ���û����������ɫ��ʽ��ת��
*					2. 32λ����ɫ�У��Ӹ�λ����λ�ֱ��Ӧ A��R��G��B  4����ɫͨ��
*					3. ��8λ��͸��ͨ���У�ff��ʾ��͸����0��ʾ��ȫ͸��
*					4. ����ʹ��ARGB1555��ARGB8888��֧��͸��ɫ����ɫ��ʽ����Ȼ͸��ɫ�������ã�����ARGB1555��֧��һλ
*						͸��ɫ��������͸���Ͳ�͸������״̬��ARGB8888֧��255��͸����
*					5. ����˵����͸������ָ �����㡢layer0��layer1 ֮���͸��
*
***************************************************************************************************************/

void LCD_SetColor(uint32_t Color)
{
	uint16_t Alpha_Value = 0, Red_Value = 0, Green_Value = 0, Blue_Value = 0; //������ɫͨ����ֵ

	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB565	)	//��32λɫת��Ϊ16λɫ
	{
		Red_Value   = (uint16_t)((Color&0x00F80000)>>8);
		Green_Value = (uint16_t)((Color&0x0000FC00)>>5);
		Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);
		LCD.Color = (uint16_t)(Red_Value | Green_Value | Blue_Value);		
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB1555 )	//��32λɫת��ΪARGB1555��ɫ
	{
		if( (Color & 0xFF000000) == 0 )	//�ж��Ƿ�ʹ��͸��ɫ
			Alpha_Value = 0x0000;
		else
			Alpha_Value = 0x8000;

		Red_Value   = (uint16_t)((Color&0x00F80000)>>9);	
		Green_Value = (uint16_t)((Color&0x0000F800)>>6);
		Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);
		LCD.Color = (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);	
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB4444 )	//��32λɫת��ΪARGB4444��ɫ
	{

		Alpha_Value = (uint16_t)((Color&0xf0000000)>>16);
		Red_Value   = (uint16_t)((Color&0x00F00000)>>12);	
		Green_Value = (uint16_t)((Color&0x0000F000)>>8);
		Blue_Value  = (uint16_t)((Color&0x000000F8)>>4);
		LCD.Color = (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);	
	}	
	else
		LCD.Color = Color;	//24λɫ��32λɫ����Ҫת��
}

/***************************************************************************************************************
*	�� �� ��:	LCD_SetBackColor
*
*	��ڲ���:	Color - Ҫ��ʾ����ɫ��ʾ����0xff0000FF ��ʾ��͸������ɫ��0xAA0000FF ��ʾ͸����Ϊ66.66%����ɫ
*
*	��������:	���ñ���ɫ,�˺������������Լ���ʾ�ַ��ı���ɫ
*
*	˵    ��:	1. Ϊ�˷����û�ʹ���Զ�����ɫ����ڲ��� Color ʹ��32λ����ɫ��ʽ���û����������ɫ��ʽ��ת��
*					2. 32λ����ɫ�У��Ӹ�λ����λ�ֱ��Ӧ A��R��G��B  4����ɫͨ��
*					3. ��8λ��͸��ͨ���У�ff��ʾ��͸����0��ʾ��ȫ͸��
*					4. ����ʹ��ARGB1555��ARGB8888��֧��͸��ɫ����ɫ��ʽ����Ȼ͸��ɫ�������ã�����ARGB1555��֧��һλ
*						͸��ɫ��������͸���Ͳ�͸������״̬��ARGB8888֧��255��͸����
*					5. ����˵����͸������ָ �����㡢layer0��layer1֮���͸��
*
***************************************************************************************************************/

void LCD_SetBackColor(uint32_t Color)
{
	uint16_t Alpha_Value = 0, Red_Value = 0, Green_Value = 0, Blue_Value = 0;  //������ɫͨ����ֵ

	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB565	)	//��32λɫת��Ϊ16λɫ
	{
		Red_Value   	= (uint16_t)((Color&0x00F80000)>>8);
		Green_Value 	= (uint16_t)((Color&0x0000FC00)>>5);
		Blue_Value  	= (uint16_t)((Color&0x000000F8)>>3);
		LCD.BackColor	= (uint16_t)(Red_Value | Green_Value | Blue_Value);	
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB1555 )	//��32λɫת��ΪARGB1555��ɫ
	{
		if( (Color & 0xFF000000) == 0 )	//�ж��Ƿ�ʹ��͸��ɫ
			Alpha_Value = 0x0000;
		else
			Alpha_Value = 0x8000;

		Red_Value   	= (uint16_t)((Color&0x00F80000)>>9);
		Green_Value 	= (uint16_t)((Color&0x0000F800)>>6);
		Blue_Value  	= (uint16_t)((Color&0x000000F8)>>3);
		LCD.BackColor 	= (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);	
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB4444 )	//��32λɫת��ΪARGB4444��ɫ
	{

		Alpha_Value 	= (uint16_t)((Color&0xf0000000)>>16);
		Red_Value   	= (uint16_t)((Color&0x00F00000)>>12);	
		Green_Value 	= (uint16_t)((Color&0x0000F000)>>8);
		Blue_Value  	= (uint16_t)((Color&0x000000F8)>>4);
		LCD.BackColor 	= (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);	
	}		
	
	else	
		LCD.BackColor = Color;	//24λɫ��32λɫ����Ҫת��
	
}


/***************************************************************************************************************
*	�� �� ��:	LCD_SetFont
*
*	��ڲ���:	*fonts - Ҫ���õ�ASCII����
*
*	��������:	����ASCII���壬��ѡ��ʹ�� 3216/2412/2010/1608/1206 ���ִ�С������
*
*	˵    ��:	1. ʹ��ʾ�� LCD_SetFont(&Font24) �������� 2412�� ASCII����
*					2. �����ģ����� lcd_fonts.c 			
*
***************************************************************************************************************/

void LCD_SetFont(pFONT *fonts)
{
	LCD_Fonts = fonts;
}

/***************************************************************************************************************
*	�� �� ��:	LCD_DisplayDirection
*
*	��ڲ���:	direction - Ҫ��ʾ�ķ���
*
*	��������:	����Ҫ��ʾ�ķ��򣬿�������� Direction_H ���������ʾ��Direction_V ������ֱ��ʾ
*
*	˵    ��:   ʹ��ʾ�� LCD_DisplayDirection(Direction_H) ����������Ļ������ʾ
*
***************************************************************************************************************/

void LCD_DisplayDirection(uint8_t direction)
{
	LCD.Direction = direction;
}

/***************************************************************************************************************
*	�� �� ��:	LCD_Clear
*
*	��������:	������������LCD���Ϊ LCD.BackColor ����ɫ��ʹ��DMA2Dʵ��
*
*	˵    ��:	���� LCD_SetBackColor() ����Ҫ����ı���ɫ���ٵ��øú�����������
*
***************************************************************************************************************/

void LCD_Clear(void)
{
	DMA2D->CR	  &=	~(DMA2D_CR_START);				//	ֹͣDMA2D
	DMA2D->CR		=	DMA2D_R2M;							//	�Ĵ�����SDRAM
	DMA2D->OPFCCR	=	LCD.ColorMode;						//	������ɫ��ʽ
	DMA2D->OOR		=	0;										//	������ƫ�� 
	DMA2D->OMAR		=	LCD.LayerMemoryAdd ;				// ��ַ
	DMA2D->NLR		=	(LCD_Width<<16)|(LCD_Height);	//	�趨���ȺͿ��
	DMA2D->OCOLR	=	LCD.BackColor;						//	��ɫ
	
// �ȴ� ��ֱ����ʹ����ʾ״̬ ����LTDC����ˢ��һ�������ݵ�ʱ��
// ��Ϊ����Ļû��ˢ��һ֡ʱ����ˢ��������˺�ѵ�����
// �û�Ҳ����ʹ�� �Ĵ��������ж� �����жϣ�����Ϊ�˱�֤���̵ļ���Լ���ֲ�ķ����ԣ�����ֱ��ʹ���жϼĴ����ķ���
//
//
	while( LTDC->CDSR != 0X00000001);	// �ж� ��ʾ״̬�Ĵ���LTDC_CDSR �ĵ�0λ VDES����ֱ����ʹ����ʾ״̬
	
	DMA2D->CR	  |=	DMA2D_CR_START;					//	����DMA2D
		
	while (DMA2D->CR & DMA2D_CR_START) ;				//	�ȴ��������
}

/***************************************************************************************************************
*	�� �� ��:	LCD_ClearRect
*
*	��ڲ���:	x - ��ʼˮƽ���꣬ȡֵ��Χ0~799 
*					y - ��ʼ��ֱ���꣬ȡֵ��Χ0~479
*					width  - Ҫ�������ĺ��򳤶�
*					height - Ҫ��������������
*
*	��������:	�ֲ�������������ָ��λ�ö�Ӧ���������Ϊ LCD.BackColor ����ɫ
*
*	˵    ��:	1. ���� LCD_SetBackColor() ����Ҫ����ı���ɫ���ٵ��øú�����������
*					2. ʹ��ʾ�� LCD_ClearRect( 10, 10, 100, 50) ���������(10,10)��ʼ�ĳ�100��50������
*
***************************************************************************************************************/

void LCD_ClearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{

	DMA2D->CR	  &=	~(DMA2D_CR_START);				//	ֹͣDMA2D
	DMA2D->CR		=	DMA2D_R2M;							//	�Ĵ�����SDRAM
	DMA2D->OPFCCR	=	LCD.ColorMode;						//	������ɫ��ʽ
	DMA2D->OCOLR	=	LCD.BackColor ;					//	��ɫ
	
	if(LCD.Direction == Direction_H)  //�������
	{		
		DMA2D->OOR		=	LCD_Width - width;				//	������ƫ�� 
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*(LCD_Width * y + x);	// ��ַ;
		DMA2D->NLR		=	(width<<16)|(height);			//	�趨���ȺͿ��		
	}
	else	//�������
	{		
		DMA2D->OOR		=	LCD_Width - height;		//	������ƫ�� 
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*((LCD_Height - x - 1 - width)*LCD_Width + y);	// ��ַ
		DMA2D->NLR		=	(width)|(height<<16);	//	�趨���ȺͿ��		
	}		

	DMA2D->CR	  |=	DMA2D_CR_START;					//	����DMA2D
		
	while (DMA2D->CR & DMA2D_CR_START) ;			//	�ȴ��������

}


/***************************************************************************************************************
*	�� �� ��:	LCD_DrawPoint
*
*	��ڲ���:	x - ��ʼˮƽ���꣬ȡֵ��Χ0~799 
*					y - ��ʼ��ֱ���꣬ȡֵ��Χ0~479
*					color  - Ҫ���Ƶ���ɫ��ʹ��32λ����ɫ��ʽ���û����������ɫ��ʽ��ת��
*
*	��������:	��ָ���������ָ����ɫ�ĵ�
*
*	˵    ��:	1. ֱ���ڶ�Ӧ���Դ�λ��д����ɫֵ������ʵ�ֻ���Ĺ���
*					2. ʹ��ʾ�� LCD_DrawPoint( 10, 10, 0xff0000FF) ��������(10,10)������ɫ�ĵ�
*
***************************************************************************************************************/

void LCD_DrawPoint(uint16_t x,uint16_t y,uint32_t color)
{

/*----------------------- 32λɫ ARGB8888 ģʽ ----------------------*/
		
	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB8888 ) 
	{
		if (LCD.Direction == Direction_H) //ˮƽ����
		{
			*(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*(x + y*LCD_Width) ) = color ; 	
		}
		else if(LCD.Direction == Direction_V)	//��ֱ����
		{
			*(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
		}
	}
/*----------------------------- 24λɫ RGB888 ģʽ -------------------------*/	
	
	else if ( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB888 )
	{		
		if (LCD.Direction == Direction_H) //ˮƽ����
		{
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 3*(x + y*LCD_Width) ) = color ; 
			*(__IO uint8_t*)( LCD.LayerMemoryAdd + 3*(x + y*LCD_Width) + 2 ) = color>>16 ; 	
		}
		else if(LCD.Direction == Direction_V)	//��ֱ����
		{
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) ) = color ; 
			*(__IO uint8_t*)( LCD.LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) +2) = color>>16 ; 	
		}	
	}

/*----------------------- 16λɫ ARGB1555��RGB565����ARGB4444 ģʽ ----------------------*/	
	else		
	{
		if (LCD.Direction == Direction_H) //ˮƽ����
		{
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*(x + y*LCD_Width) ) = color ; 	
		}
		else if(LCD.Direction == Direction_V)	//��ֱ����
		{
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
		}	
	}
}

/***************************************************************************************************************
*	�� �� ��:	LCD_ReadPoint
*
*	��ڲ���:	x - ��ʼˮƽ���꣬ȡֵ��Χ0~799 
*					y - ��ʼ��ֱ���꣬ȡֵ��Χ0~479
*
*	�� �� ֵ��  ��ȡ������ɫ
*
*	��������:	��ȡָ����������ɫ����ʹ��16��24λɫģʽʱ������������ɫ���ݶ�ӦΪ16λ��24λ
*
*	˵    ��:	1. ֱ�Ӷ�ȡ��Ӧ���Դ�ֵ������ʵ�ֶ���Ĺ���
*					2. ʹ��ʾ�� color = LCD_ReadPoint( 10, 10) ��color Ϊ��ȡ���������(10,10)����ɫ
*
***************************************************************************************************************/

uint32_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
	uint32_t color = 0;

/*----------------------- 32λɫ ARGB8888 ģʽ ----------------------*/
	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB8888 ) 
	{
		if (LCD.Direction == Direction_H) //ˮƽ����
		{
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*(x + y*LCD_Width) ); 	
		}
		else if(LCD.Direction == Direction_V)	//��ֱ����
		{
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*((LCD_Height - x - 1)*LCD_Width + y) );
		}
	}
	
/*----------------------------- 24λɫ RGB888 ģʽ -------------------------*/	
	else if ( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB888 )
	{
		if (LCD.Direction == Direction_H) //ˮƽ����
		{
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 3*(x + y*LCD_Width) ) &0x00ffffff; 	
		}
		else if(LCD.Direction == Direction_V)	//��ֱ����
		{
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) ) &0x00ffffff; 	
		}	
	}
	
/*----------------------- 16λɫ ARGB1555��RGB565����ARGB4444 ģʽ ----------------------*/	
	else		
	{
		if (LCD.Direction == Direction_H) //ˮƽ����
		{
			color = *(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*(x + y*LCD_Width) ); 	
		}
		else if(LCD.Direction == Direction_V)	//��ֱ����
		{
			color = *(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*((LCD_Height - x - 1)*LCD_Width + y) );
		}	
	}
	return color;
}  
 
/***************************************************************************************************************
*	�� �� ��:	LCD_DisplayChar
*
*	��ڲ���:	x - ��ʼˮƽ���꣬ȡֵ��Χ0~799 
*					y - ��ʼ��ֱ���꣬ȡֵ��Χ0~479
*					c  - ASCII�ַ�
*
*	��������:	��ָ��������ʾָ�����ַ�
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetFont(&Font24) ����Ϊ 2412��ASCII����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0xff0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0xff000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayChar( 10, 10, 'a') ��������(10,10)��ʾ�ַ� 'a'
*
***************************************************************************************************************/

void LCD_DisplayChar(uint16_t x, uint16_t y,uint8_t add)
{
	uint16_t  index = 0, counter = 0;
   uint8_t   disChar;	//��ģ��ֵ
	uint16_t  Xaddress = x; //ˮƽ����
	
	add = add - 32; 
	for(index = 0; index < LCD_Fonts->Sizes; index++)
	{
		disChar = LCD_Fonts->pTable[add*LCD_Fonts->Sizes + index]; //��ȡ�ַ���ģֵ
		for(counter = 0; counter < 8; counter++)
		{ 
			if(disChar & 0x01)	
			{		
				LCD_DrawPoint(Xaddress,y,LCD.Color);	//��ǰģֵ��Ϊ0ʱ��ʹ�û���ɫ���
			}
			else		
			{		
				LCD_DrawPoint(Xaddress,y,LCD.BackColor);	//����ʹ�ñ���ɫ���Ƶ�
			}
			disChar >>= 1;
			Xaddress++;  //ˮƽ�����Լ�
			
			if( (Xaddress - x)==LCD_Fonts->Width ) //���ˮƽ����ﵽ���ַ���ȣ����˳���ǰѭ��
			{													//������һ�еĻ���
				Xaddress = x;
				y++;
				break;
			}
		}	
	}
}

/***************************************************************************************************************
*	�� �� ��:	LCD_DisplayString
*
*	��ڲ���:	x - ��ʼˮƽ���꣬ȡֵ��Χ0~799 
*					y - ��ʼ��ֱ���꣬ȡֵ��Χ0~479
*					p - ASCII�ַ������׵�ַ
*
*	��������:	��ָ��������ʾָ�����ַ���
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetFont(&Font24) ����Ϊ 2412��ASCII����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0xff0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0xff000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayString( 10, 10, "FANKE") ������ʼ����Ϊ(10,10)�ĵط���ʾ�ַ���"FANKE"
*
***************************************************************************************************************/

void LCD_DisplayString( uint16_t x, uint16_t y,  char *p) 
{  
	while ((x < LCD_Width) && (*p != 0))	//�ж���ʾ�����Ƿ񳬳���ʾ�������ַ��Ƿ�Ϊ���ַ�
	{
		 LCD_DisplayChar( x,y,*p);
		 x += LCD_Fonts->Width; //��ʾ��һ���ַ�
		 p++;	//ȡ��һ���ַ���ַ
	}
}

/***************************************************************************************************************
*	�� �� ��:	LCD_SetTextFont
*
*	��ڲ���:	*fonts - Ҫ���õ��ı�����
*
*	��������:	�����ı����壬�������ĺ�ASCII�ַ���
*
*	˵    ��:	1. ��ѡ��ʹ�� 3232/2424/2020/1616/1212 ���ִ�С���������壬
*						���Ҷ�Ӧ������ASCII����Ϊ 3216/2412/2010/1608/1206
*					2. �����ģ����� lcd_fonts.c 
*					3. �����ֿ�ʹ�õ���С�ֿ⣬���õ��˶�Ӧ�ĺ�����ȥȡģ
*					4. ʹ��ʾ�� LCD_SetTextFont(&CH_Font24) �������� 2424�����������Լ�2412��ASCII�ַ�����
*
***************************************************************************************************************/

void LCD_SetTextFont(pFONT *fonts)
{
	LCD_CHFonts = fonts;		// ������������
	switch(fonts->Width )
	{
		case 12:	LCD_Fonts = &Font12;	break;	// ����ASCII�ַ�������Ϊ 1206
		case 16:	LCD_Fonts = &Font16;	break;	// ����ASCII�ַ�������Ϊ 1608
		case 20:	LCD_Fonts = &Font20;	break;	// ����ASCII�ַ�������Ϊ 2010	
		case 24:	LCD_Fonts = &Font24;	break;	// ����ASCII�ַ�������Ϊ 2412
		case 32:	LCD_Fonts = &Font32;	break;	// ����ASCII�ַ�������Ϊ 3216		
		default: break;
	}

}

/***************************************************************************************************************
*	�� �� ��:	LCD_DisplayChinese
*
*	��ڲ���:	x - ��ʼˮƽ���꣬ȡֵ��Χ0~799 
*					y - ��ʼ��ֱ���꣬ȡֵ��Χ0~479
*					pText - �����ַ�
*
*	��������:	��ָ��������ʾָ���ĵ��������ַ�
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetTextFont(&CH_Font24) ����Ϊ 2424�����������Լ�2412��ASCII�ַ�����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0xff0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0xff000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayChinese( 10, 10, "��") ��������(10,10)��ʾ�����ַ�"��"
*
***************************************************************************************************************/

void LCD_DisplayChinese(uint16_t x, uint16_t y, char *pText) 
{
	uint16_t  i=0,index = 0, counter = 0;	// ��������
	uint16_t  addr;	// ��ģ��ַ
   uint8_t   disChar;	//��ģ��ֵ
	uint16_t  Xaddress = x; //ˮƽ����

	while(1)
	{		
		// �Ա������еĺ��ֱ��룬���Զ�λ�ú�����ģ�ĵ�ַ		
		if ( *(LCD_CHFonts->pTable + (i+1)*LCD_CHFonts->Sizes + 0)==*pText && *(LCD_CHFonts->pTable + (i+1)*LCD_CHFonts->Sizes + 1)==*(pText+1) )	
		{   
			addr=i;	// ��ģ��ַƫ��
			break;
		}				
		i+=2;	// ÿ�������ַ�����ռ���ֽ�

		if(i >= LCD_CHFonts->Table_Rows)	break;	// ��ģ�б�������Ӧ�ĺ���	
	}	
	

	for(index = 0; index <LCD_CHFonts->Sizes; index++)
	{	
		disChar = *(LCD_CHFonts->pTable + (addr)*LCD_CHFonts->Sizes + index);	// ��ȡ��Ӧ����ģ��ַ
		
		for(counter = 0; counter < 8; counter++)
		{ 
			if(disChar & 0x01)	
			{		
				LCD_DrawPoint(Xaddress,y,LCD.Color);	//��ǰģֵ��Ϊ0ʱ��ʹ�û���ɫ���
			}
			else		
			{		
				LCD_DrawPoint(Xaddress,y,LCD.BackColor);	//����ʹ�ñ���ɫ���Ƶ�
			}
			disChar >>= 1;
			Xaddress++;  //ˮƽ�����Լ�
			
			if( (Xaddress - x)==LCD_CHFonts->Width ) 	//	���ˮƽ����ﵽ���ַ���ȣ����˳���ǰѭ��
			{														//	������һ�еĻ���
				Xaddress = x;
				y++;
				break;
			}
		}	
	}	

}


/***************************************************************************************************************
*	�� �� ��:	LCD_DisplayText
*
*	��ڲ���:	x - ��ʼˮƽ���꣬ȡֵ��Χ0~799 
*					y - ��ʼ��ֱ���꣬ȡֵ��Χ0~479
*					pText - �ַ�����������ʾ���Ļ���ASCII�ַ�
*
*	��������:	��ָ��������ʾָ�����ַ���
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetTextFont(&CH_Font24) ����Ϊ 2424�����������Լ�2412��ASCII�ַ�����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0xff0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0xff000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayChinese( 10, 10, "���ͿƼ�STM32") ��������(10,10)��ʾ�ַ���"���ͿƼ�STM32"
*
***************************************************************************************************************/

void LCD_DisplayText(uint16_t x, uint16_t y, char *pText) 
{  
 	
	while(*pText != 0)	// �ж��Ƿ�Ϊ���ַ�
	{
		if(*pText<=0x7F)	// �ж��Ƿ�ΪASCII��
		{
			LCD_DisplayChar(x,y,*pText);	// ��ʾASCII
			x+=LCD_Fonts->Width;				// ˮƽ���������һ���ַ���
			pText++;								// �ַ�����ַ+1
		}
		else					// ���ַ�Ϊ����
		{			
			LCD_DisplayChinese(x,y,pText);	// ��ʾ����
			x+=LCD_CHFonts->Width;				// ˮƽ���������һ���ַ���
			pText+=2;								// �ַ�����ַ+2�����ֵı���Ҫ2�ֽ�
		}
	}	
}

/***************************************************************************************************************
*	�� �� ��:	LCD_ShowNumMode
*
*	��ڲ���:	mode - ���ñ�������ʾģʽ
*
*	��������:	���ñ�����ʾʱ����λ��0���ǲ��ո񣬿�������� Fill_Space ���ո�Fill_Zero �����
*
*	˵    ��:   1. ֻ�� LCD_DisplayNumber() ��ʾ���� �� LCD_DisplayDecimals()��ʾС�� �����������õ�
*					2. ʹ��ʾ�� LCD_ShowNumMode(Fill_Zero) ���ö���λ���0������ 123 ������ʾΪ 000123
*
***************************************************************************************************************/

void LCD_ShowNumMode(uint8_t mode)
{
	LCD.ShowNum_Mode = mode;
}

/*****************************************************************************************************************************************
*	�� �� ��:	LCD_DisplayNumber
*
*	��ڲ���:	x - ��ʼˮƽ���꣬ȡֵ��Χ0~799 
*					y - ��ʼ��ֱ���꣬ȡֵ��Χ0~479
*					number - Ҫ��ʾ������,��Χ�� -2147483648~2147483647 ֮��
*					len - ���ֵ�λ�������λ������len��������ʵ�ʳ�������������������š��������Ҫ��ʾ��������Ԥ��һ��λ�ķ�����ʾ�ռ�
*
*	��������:	��ָ��������ʾָ������������
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetTextFont(&CH_Font24) ����Ϊ 2424�����������Լ�2412��ASCII�ַ�����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0xff0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0xff000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayNumber( 10, 10, a, 5) ��������(10,10)��ʾָ������a,�ܹ�5λ������λ��0��ո�
*						���� a=123 ʱ������� LCD_ShowNumMode()����������ʾ  123(ǰ�������ո�λ) ����00123
*						
*****************************************************************************************************************************************/

void  LCD_DisplayNumber( uint16_t x, uint16_t y, int32_t number, uint8_t len) 
{  
	char   Number_Buffer[15];				// ���ڴ洢ת������ַ���

	if( LCD.ShowNum_Mode == Fill_Zero)	// ����λ��0
	{
		sprintf( Number_Buffer , "%0.*d",len, number );	// �� number ת�����ַ�����������ʾ		
	}
	else			// ����λ���ո�
	{	
		sprintf( Number_Buffer , "%*d",len, number );	// �� number ת�����ַ�����������ʾ		
	}
	
	LCD_DisplayString( x, y,(char *)Number_Buffer) ;  // ��ת���õ����ַ�����ʾ����
	
}

/***************************************************************************************************************************************
*	�� �� ��:	LCD_DisplayDecimals
*
*	��ڲ���:	x - ��ʼˮƽ���꣬ȡֵ��Χ0~799 
*					y - ��ʼ��ֱ���꣬ȡֵ��Χ0~479
*					decimals - Ҫ��ʾ������, double��ȡֵ1.7 x 10^��-308��~ 1.7 x 10^��+308����������ȷ��׼ȷ����Чλ��Ϊ15~16λ
*
*       			len - ������������λ��������С����͸��ţ�����ʵ�ʵ���λ��������ָ������λ��������ʵ�ʵ��ܳ���λ�����
*							ʾ��1��С�� -123.123 ��ָ�� len <=8 �Ļ�����ʵ���ճ���� -123.123
*							ʾ��2��С�� -123.123 ��ָ�� len =10 �Ļ�����ʵ�����   -123.123(����ǰ����������ո�λ) 
*							ʾ��3��С�� -123.123 ��ָ�� len =10 �Ļ��������ú��� LCD_ShowNumMode() ����Ϊ���0ģʽʱ��ʵ����� -00123.123 
*
*					decs - Ҫ������С��λ������С����ʵ��λ��������ָ����С��λ����ָ���Ŀ�������������
*							 ʾ����1.12345 ��ָ�� decs Ϊ4λ�Ļ�����������Ϊ1.1235
*
*	��������:	��ָ��������ʾָ���ı���������С��
*
*	˵    ��:	1. ������Ҫ��ʾ�����壬����ʹ�� LCD_SetTextFont(&CH_Font24) ����Ϊ 2424�����������Լ�2412��ASCII�ַ�����
*					2.	������Ҫ��ʾ����ɫ������ʹ�� LCD_SetColor(0xff0000FF) ����Ϊ��ɫ
*					3. �����ö�Ӧ�ı���ɫ������ʹ�� LCD_SetBackColor(0xff000000) ����Ϊ��ɫ�ı���ɫ
*					4. ʹ��ʾ�� LCD_DisplayDecimals( 10, 10, a, 5, 3) ��������(10,10)��ʾ�ֱ���a,�ܳ���Ϊ5λ�����б���3λС��
*						
*****************************************************************************************************************************************/

void  LCD_DisplayDecimals( uint16_t x, uint16_t y, double decimals, uint8_t len, uint8_t decs) 
{  
	char  Number_Buffer[20];				// ���ڴ洢ת������ַ���
	
	if( LCD.ShowNum_Mode == Fill_Zero)	// ����λ���0ģʽ
	{
		sprintf( Number_Buffer , "%0*.*lf",len,decs, decimals );	// �� number ת�����ַ�����������ʾ		
	}
	else		// ����λ���ո�
	{
		sprintf( Number_Buffer , "%*.*lf",len,decs, decimals );	// �� number ת�����ַ�����������ʾ		
	}
	
	LCD_DisplayString( x, y,(char *)Number_Buffer) ;	// ��ת���õ����ַ�����ʾ����
}



/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawImage
*
*	��ڲ���: x - ˮƽ���꣬ȡֵ��Χ 0~799
*			 	 y - ��ֱ���꣬ȡֵ��Χ 0~479
*			 	 width  - ͼƬ��ˮƽ��ȣ����ȡֵ800
*				 height - ͼƬ�Ĵ�ֱ��ȣ����ȡֵ480
*				*pImage - ͼƬ���ݴ洢�����׵�ַ
*
*	��������: ��ָ�����괦��ʾͼƬ
*
*	˵    ��: Ҫ��ʾ��ͼƬ��Ҫ���Ƚ���ȡģ����ֻ����ʾһ����ɫ��ʹ�� LCD_SetColor() �������û���ɫ
*						 
*****************************************************************************************************************************************/

void 	LCD_DrawImage(uint16_t x,uint16_t y,uint16_t width,uint16_t height,const uint8_t *pImage) 
{  
   uint8_t   disChar;	//��ģ��ֵ
	uint16_t  Xaddress = x; //ˮƽ����
	uint16_t  i=0,j=0,m=0;
	
	for(i = 0; i <height; i++)
	{
		for(j = 0; j <(float)width/8; j++)
		{
			disChar = *pImage;

			for(m = 0; m < 8; m++)
			{ 
				if(disChar & 0x01)	
				{		
					LCD_DrawPoint(Xaddress,y,LCD.Color);	//��ǰģֵ��Ϊ0ʱ��ʹ�û���ɫ���
				}
				else		
				{		
					LCD_DrawPoint(Xaddress,y,LCD.BackColor);	//����ʹ�ñ���ɫ���Ƶ�
				}
				disChar >>= 1;
				Xaddress++;  //ˮƽ�����Լ�
				
				if( (Xaddress - x)==width ) //���ˮƽ����ﵽ���ַ���ȣ����˳���ǰѭ��
				{													//������һ�еĻ���
					Xaddress = x;
					y++;
					break;
				}
			}	
			pImage++;			
		}
	}	
}

/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawLine
*
*	��ڲ���: x1 - ��� ˮƽ���꣬ȡֵ��Χ 0~799
*			 	 y1 - ��� ��ֱ���꣬ȡֵ��Χ 0~479
*
*				 x2 - �յ� ˮƽ���꣬ȡֵ��Χ 0~799
*            y2 - �յ� ��ֱ���꣬ȡֵ��Χ 0~479
*
*	��������: ������֮�仭��
*
*	˵    ��: �ú�����ֲ��ST�ٷ������������
*						 
*****************************************************************************************************************************************/

#define ABS(X)  ((X) > 0 ? (X) : -(X))    

//	����������
//	������x1��y1Ϊ������꣬x2��y2Ϊ�յ�����
//
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
	curpixel = 0;

	deltax = ABS(x2 - x1);        /* The difference between the x's */
	deltay = ABS(y2 - y1);        /* The difference between the y's */
	x = x1;                       /* Start x off at the first pixel */
	y = y1;                       /* Start y off at the first pixel */

	if (x2 >= x1)                 /* The x-values are increasing */
	{
	 xinc1 = 1;
	 xinc2 = 1;
	}
	else                          /* The x-values are decreasing */
	{
	 xinc1 = -1;
	 xinc2 = -1;
	}

	if (y2 >= y1)                 /* The y-values are increasing */
	{
	 yinc1 = 1;
	 yinc2 = 1;
	}
	else                          /* The y-values are decreasing */
	{
	 yinc1 = -1;
	 yinc2 = -1;
	}

	if (deltax >= deltay)         /* There is at least one x-value for every y-value */
	{
	 xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
	 yinc2 = 0;                  /* Don't change the y for every iteration */
	 den = deltax;
	 num = deltax / 2;
	 numadd = deltay;
	 numpixels = deltax;         /* There are more x-values than y-values */
	}
	else                          /* There is at least one y-value for every x-value */
	{
	 xinc2 = 0;                  /* Don't change the x for every iteration */
	 yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
	 den = deltay;
	 num = deltay / 2;
	 numadd = deltax;
	 numpixels = deltay;         /* There are more y-values than x-values */
	}
	for (curpixel = 0; curpixel <= numpixels; curpixel++)
	{
	 LCD_DrawPoint(x,y,LCD.Color);             /* Draw the current pixel */
	 num += numadd;              /* Increase the numerator by the top of the fraction */
	 if (num >= den)             /* Check if numerator >= denominator */
	 {
		num -= den;               /* Calculate the new numerator value */
		x += xinc1;               /* Change the x as appropriate */
		y += yinc1;               /* Change the y as appropriate */
	 }
	 x += xinc2;                 /* Change the x as appropriate */
	 y += yinc2;                 /* Change the y as appropriate */
	}

}

/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawRect
*
*	��ڲ���: x - ˮƽ���꣬ȡֵ��Χ 0~799
*			 	 y - ��ֱ���꣬ȡֵ��Χ 0~479
*			 	 width  - ͼƬ��ˮƽ��ȣ����ȡֵ800
*				 height - ͼƬ�Ĵ�ֱ��ȣ����ȡֵ480
*
*	��������: ��ָ��λ�û���ָ������ľ�������
*
*	˵    ��: �ú�����ֲ��ST�ٷ������������
*						 
*****************************************************************************************************************************************/

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	/* draw horizontal lines */
	LCD_DrawLine(x, y, x+width, y);
	LCD_DrawLine(x, y+height, x+width, y+height);

	/* draw vertical lines */
	LCD_DrawLine(x, y, x, y+height);
	LCD_DrawLine(x+width, y, x+width, y+height);
}

/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawCircle
*
*	��ڲ���: x - Բ�� ˮƽ���꣬ȡֵ��Χ 0~799
*			 	 y - Բ�� ��ֱ���꣬ȡֵ��Χ 0~479
*			 	 r  - �뾶
*
*	��������: ������ (x,y) ���ư뾶Ϊ r ��Բ������
*
*	˵    ��: 1. �ú�����ֲ��ST�ٷ������������
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����
*
*****************************************************************************************************************************************/

void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r)
{
	int Xadd = -r, Yadd = 0, err = 2-2*r, e2;
	do {   

		LCD_DrawPoint(x-Xadd,y+Yadd,LCD.Color);
		LCD_DrawPoint(x+Xadd,y+Yadd,LCD.Color);
		LCD_DrawPoint(x+Xadd,y-Yadd,LCD.Color);
		LCD_DrawPoint(x-Xadd,y-Yadd,LCD.Color);
		
		e2 = err;
		if (e2 <= Yadd) {
			err += ++Yadd*2+1;
			if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
		}
		if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);
    
}

/***************************************************************************************************************************************
*	�� �� ��: LCD_DrawEllipse
*
*	��ڲ���: x - Բ�� ˮƽ���꣬ȡֵ��Χ 0~799
*			 	 y - Բ�� ��ֱ���꣬ȡֵ��Χ 0~479
*			 	 r1  - ˮƽ����ĳ���
*				 r2  - ��ֱ����ĳ���
*
*	��������: ������ (x,y) ����ˮƽ����Ϊ r1 ��ֱ����Ϊ r2 ����Բ����
*
*	˵    ��: 1. �ú�����ֲ��ST�ٷ������������
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����
*
*****************************************************************************************************************************************/

void LCD_DrawEllipse(int x, int y, int r1, int r2)
{
  int Xadd = -r1, Yadd = 0, err = 2-2*r1, e2;
  float K = 0, rad1 = 0, rad2 = 0;
   
  rad1 = r1;
  rad2 = r2;
  
  if (r1 > r2)
  { 
    do {
      K = (float)(rad1/rad2);
		 
		LCD_DrawPoint(x-Xadd,y+(uint16_t)(Yadd/K),LCD.Color);
		LCD_DrawPoint(x+Xadd,y+(uint16_t)(Yadd/K),LCD.Color);
		LCD_DrawPoint(x+Xadd,y-(uint16_t)(Yadd/K),LCD.Color);
		LCD_DrawPoint(x-Xadd,y-(uint16_t)(Yadd/K),LCD.Color);     
		 
      e2 = err;
      if (e2 <= Yadd) {
        err += ++Yadd*2+1;
        if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
      }
      if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);
  }
  else
  {
    Yadd = -r2; 
    Xadd = 0;
    do { 
      K = (float)(rad2/rad1);

		LCD_DrawPoint(x-(uint16_t)(Xadd/K),y+Yadd,LCD.Color);
		LCD_DrawPoint(x+(uint16_t)(Xadd/K),y+Yadd,LCD.Color);
		LCD_DrawPoint(x+(uint16_t)(Xadd/K),y-Yadd,LCD.Color);
		LCD_DrawPoint(x-(uint16_t)(Xadd/K),y-Yadd,LCD.Color);  
		 
      e2 = err;
      if (e2 <= Xadd) {
        err += ++Xadd*3+1;
        if (-Yadd == Xadd && e2 <= Yadd) e2 = 0;
      }
      if (e2 > Yadd) err += ++Yadd*3+1;     
    }
    while (Yadd <= 0);
  }
}

/***************************************************************************************************************************************
*	�� �� ��: LCD_FillRect
*
*	��ڲ���: x - ˮƽ���꣬ȡֵ��Χ 0~799
*			 	 y - ��ֱ���꣬ȡֵ��Χ 0~479
*			 	 width  - ͼƬ��ˮƽ��ȣ����ȡֵ800
*				 height - ͼƬ�Ĵ�ֱ��ȣ����ȡֵ480
*
*	��������: ������ (x,y) ���ָ�������ʵ�ľ���
*
*	˵    ��: 1. ʹ��DMA2Dʵ��
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����
*						 
*****************************************************************************************************************************************/

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	DMA2D->CR	  &=	~(DMA2D_CR_START);				//	ֹͣDMA2D
	DMA2D->CR		=	DMA2D_R2M;							//	�Ĵ�����SDRAM
	DMA2D->OPFCCR	=	LCD.ColorMode;						//	������ɫ��ʽ
	DMA2D->OCOLR	=	LCD.Color;							//	��ɫ
	
	if(LCD.Direction == Direction_H)  //�������
	{		
		DMA2D->OOR		=	LCD_Width - width;				//	������ƫ�� 
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*(LCD_Width * y + x);	// ��ַ;
		DMA2D->NLR		=	(width<<16)|(height);			//	�趨���ȺͿ��		
	}
	else	//�������
	{		
		DMA2D->OOR		=	LCD_Width - height;		//	������ƫ�� 
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*((LCD_Height - x - 1 - width)*LCD_Width + y);	// ��ַ
		DMA2D->NLR		=	(width)|(height<<16);	//	�趨���ȺͿ��		
	}		

	DMA2D->CR	  |=	DMA2D_CR_START;					//	����DMA2D
		
	while (DMA2D->CR & DMA2D_CR_START) ;			//	�ȴ��������
}

/***************************************************************************************************************************************
*	�� �� ��: LCD_FillCircle
*
*	��ڲ���: x - Բ�� ˮƽ���꣬ȡֵ��Χ 0~799
*			 	 y - Բ�� ��ֱ���꣬ȡֵ��Χ 0~479
*			 	 r  - �뾶
*
*	��������: ������ (x,y) ���뾶Ϊ r ��Բ������
*
*	˵    ��: 1. �ú�����ֲ��ST�ٷ������������
*				 2. Ҫ���Ƶ������ܳ�����Ļ����ʾ����
*
*****************************************************************************************************************************************/

void LCD_FillCircle(uint16_t x, uint16_t y, uint16_t r)
{
  int32_t  D;    /* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (r << 1);
  
  CurX = 0;
  CurY = r;
  
  while (CurX <= CurY)
  {
    if(CurY > 0) 
    { 
		LCD_DrawLine(x - CurX, y - CurY,x - CurX,y - CurY + 2*CurY);
		LCD_DrawLine(x + CurX, y - CurY,x + CurX,y - CurY + 2*CurY); 
    }
    
    if(CurX > 0) 
    {
		LCD_DrawLine(x - CurY, y - CurX,x - CurY,y - CurX + 2*CurX);
		LCD_DrawLine(x + CurY, y - CurX,x + CurY,y - CurX + 2*CurX); 		 
    }
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
  
  LCD_DrawCircle(x, y, r);  
}


void LCD_Init(void)
{
	
	LCD_DisplayDirection(Direction_H); 	    //	������ʾ
	LCD_SetFont(&Font24);  					//	����Ĭ������	
	LCD_ShowNumMode(Fill_Space);			//	������ʾĬ�����ո�
	
	LCD_SetLayer(0);  
	LCD_SetBackColor(LCD_BLACK); 			//	���ñ���ɫ
	LCD_SetColor(LCD_WHITE);				//	���û�����ɫ
	LCD_Clear(); 								//	������ˢ����ɫ
  
	HAL_GPIO_WritePin(LTDC_Black_PORT, LTDC_Black_PIN, GPIO_PIN_SET);	// ��������
	
}

/*************************************************************************************************
*	�� �� ��:	LCD_Test
*
*	��������:	��Ļ����
*
*	˵    ��:	��	
*************************************************************************************************/

void LCD_Test(void)
{
	uint16_t time = 80;	// ��ʱʱ��
	uint16_t i;					// ��������

	int32_t	a = 0;			// �����������������ڲ���
	int32_t	b = 0;			// �����������������ڲ���

	double p = 3.1415926;	// ���帡�������������ڲ���
	double f = -1234.1234;	// ���帡�������������ڲ���
	
// ���Ƴ�ʼ���棬�������⡢LOGO�Լ�������>>>>>

	LCD_SetBackColor(LIGHT_MAGENTA); 			//	���ñ���ɫ��ʹ���Զ�����ɫ
	LCD_Clear(); 									//	������ˢ����ɫ
	
	LCD_SetTextFont(&CH_Font32);				// ����3232��������,ASCII�����ӦΪ3216
	LCD_SetColor(0xff333333);					//	���û���ɫ��ʹ���Զ�����ɫ
	LCD_DisplayText(334, 160,"��Ļ����");	// ��ʾ�ı�
	
	LCD_SetColor(0xfffd7923);					//	���û���ɫ��ʹ���Զ�����ɫ
//	LCD_DrawImage(  280, 218, 240, 83, Image_FANKE_240x83) ;		// ��ʾLOGOͼƬ

	LCD_SetColor(LIGHT_YELLOW);		//	���û���ɫ
	for(i=0;i<150;i++)
   {
		LCD_FillRect(100,330,4*i,6);	// ���ƾ��Σ�ʵ�ּ��׽�������Ч��
		HAL_Delay(3);	
   }
   
// ��ʾ����������������С��>>>>>
  
	LCD_SetBackColor(LCD_BLACK); 			//	���ñ���ɫ
	LCD_Clear(); 								// ����
	
	LCD_SetTextFont(&CH_Font32);					// ����2424��������,ASCII�����ӦΪ2412
	LCD_SetColor(LCD_WHITE);						// ���û���,��ɫ
	
	LCD_DisplayText(28, 20,"����λ���ո�");	// ��ʾ�ı�	
	LCD_DisplayText(400,20,"����λ���0");		// ��ʾ�ı�		
	
	LCD_SetColor(LIGHT_CYAN);					// ���û��ʣ�����ɫ
	LCD_DisplayText(28, 60,"��������");				
	LCD_DisplayText(28,100,"��С����");		
	LCD_DisplayText(28,140,"��������");					
				
	LCD_SetColor(LIGHT_YELLOW);				// ���û��ʣ�����ɫ		
	LCD_DisplayText(400, 60,"��������");	
	LCD_DisplayText(400, 100,"��С����");		
	LCD_DisplayText(400,140,"��������");	
			
	for(i=0;i<100;i++)
   {
		LCD_SetColor(LIGHT_CYAN);									// ���û���	������ɫ	
		LCD_ShowNumMode(Fill_Space);								// ����λ���ո�
		LCD_DisplayNumber( 160, 60, a+i*125, 8) ;				// ��ʾ����		
	   	LCD_DisplayDecimals( 160, 100, p+i*0.1,  6,3);		// ��ʾС��
		LCD_DisplayNumber( 160,140, b-i,     6) ;				// ��ʾ����			
		
		LCD_SetColor(LIGHT_YELLOW);								// ���û��ʣ�����ɫ	
		LCD_ShowNumMode(Fill_Zero);								// ����λ���0
		LCD_DisplayNumber( 560, 60, a+i*125, 8) ;				// ��ʾ����		
	   	LCD_DisplayDecimals( 560, 100, f+i*0.01, 11,4);		// ��ʾС��
		LCD_DisplayNumber( 560,140, b-i,     6) ;				// ��ʾ����				
			
		HAL_Delay(30);				
   }
	
 // ��ʾ�ı����������������С�����ĺ�ASCII�ַ� >>>>> 
																																	  
	LCD_SetColor(LCD_WHITE);                                                                              
	LCD_SetFont(&Font12); LCD_DisplayString(0,178,"!#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRST"); HAL_Delay(time);	
	LCD_SetFont(&Font16); LCD_DisplayString(0,190,"!#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRST"); HAL_Delay(time);	
	LCD_SetFont(&Font20); LCD_DisplayString(0,206,"!#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRST"); HAL_Delay(time);		
	LCD_SetFont(&Font24); LCD_DisplayString(0,226,"!#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRST"); HAL_Delay(time);		
	LCD_SetFont(&Font32); LCD_DisplayString(0,250,"!#$%&'()*+,-.0123456789:;<=>?@ABCDEFGHIJKLMNOPQRST"); HAL_Delay(time);	

	LCD_SetTextFont(&CH_Font12);			// ����1212��������,ASCII�����ӦΪ1206
	LCD_SetColor(0Xff8AC6D1);						// ���û���
	LCD_DisplayText(28, 290,"1212�������壺֪�пƼ�");	
	
	LCD_SetTextFont(&CH_Font16);			// ����1616��������,ASCII�����ӦΪ1608
	LCD_SetColor(0XffC5E1A5);						// ���û���
	LCD_DisplayText(28, 310,"1616�������壺֪�пƼ�");		
	
	LCD_SetTextFont(&CH_Font20);			// ����2020��������,ASCII�����ӦΪ2010
	LCD_SetColor(0Xff2D248A);						// ���û���
	LCD_DisplayText(28, 335,"2020�������壺֪�пƼ�");		

	LCD_SetTextFont(&CH_Font24);			// ����2424��������,ASCII�����ӦΪ2412
	LCD_SetColor(0XffFF585D);						// ���û���
	LCD_DisplayText(28, 365,"2424�������壺֪�пƼ�");		

	LCD_SetTextFont(&CH_Font32);			// ����3232��������,ASCII�����ӦΪ3216
	LCD_SetColor(0XffF6003C);						// ���û���
	LCD_DisplayText(28, 405,"3232�������壺֪�пƼ�");
	
	HAL_Delay(2000);	// ��ʱ
	
	LCD_SetBackColor(LCD_BLACK); 			//	���ñ���ɫ��ʹ���Զ�����ɫ
	LCD_Clear(); 									//	������ˢ����ɫ
}
