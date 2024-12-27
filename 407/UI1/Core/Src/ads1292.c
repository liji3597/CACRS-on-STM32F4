//-----------------------------------------------------------------
// 头文件包含
//-----------------------------------------------------------------
#include "ads1292.h"
#include "spi.h"
#include "delay.h"
#include "callback.h"
//#include <stdlib.h>

 #define DEBUG_ADS1292	//寄存器printf调试
 
//-----------------------------------------------------------------

u8 ADS1292_REG[12];		//ads1292寄存器数组
ADS1292_CONFIG1 	Ads1292_Config1		={DATA_RATE};																				//CONFIG1
ADS1292_CONFIG2 	Ads1292_Config2		={PDB_LOFF_COMP,PDB_REFBUF,VREF,CLK_EN,INT_TEST};		//CONFIG2
ADS1292_CHSET 		Ads1292_Ch1set		={CNNNLE1_POWER,CNNNLE1_GAIN,CNNNLE1_MUX};					//CH1SET
ADS1292_CHSET 		Ads1292_Ch2set		={CNNNLE2_POWER,CNNNLE2_GAIN,CNNNLE2_MUX};					//CH2SET
ADS1292_RLD_SENS	Ads1292_Rld_Sens	={PDB_RLD,RLD_LOFF_SENSE,RLD2N,RLD2P,RLD1N,RLD1P};	//RLD_SENS
ADS1292_LOFF_SENS	Ads1292_Loff_Sens	={FLIP2,FLIP1,LOFF2N,LOFF2P,LOFF1N,LOFF1P};					//LOFF_SENS
ADS1292_RESP1			Ads1292_Resp1			={RESP_DEMOD_EN1,RESP_MOD_EN,RESP_PH,RESP_CTRL};		//RSP1
ADS1292_RESP2			Ads1292_Resp2			={CALIB,FREQ,RLDREF_INT};														//RSP2



//读取72位的数据1100+LOFF_STAT[4:0]+GPIO[1:0]+13个0+2CHx24位，共9字节 （1字节8位）
//	1100	LOFF_STAT[4			3			2			1			0	]	//导联脱落相关的信息在LOFF_STAT寄存器里
//									RLD		1N2N	1N2P	1N1N	1N1P	
//	例	C0 00 00 FF E1 1A FF E1 52	

u8 ADS1292_Read_Data(u8 *data)//72M时钟下函数耗时大约10us  8M时钟下 函数耗时大约 100us  
{		
		u8 i;	
		
		ADS1292_CS=0;//读9个字节的数据  c00000 xxxxxx(呼吸) xxxxxx(心电)
		//delay_us(10);
		for(i=0;i<9;i++)
		{	
				*data=ADS1292_SPI(0X00);	
				data++;
		}
		//delay_us(10);
		ADS1292_CS=1;		
		return 0;
}


/*******************************************************************************
* 功  能	: 设置寄存器 
* 描  述	: CONFIG1设置采样频率500HZ
						CH1SET通道1 2的PGA增益设置为2
*******************************************************************************/
void ADS1292_SET_REGBUFF(void)   //
{
	ADS1292_REG[ID] =	ADS1292_DEVICE;//ID只读  0X73
	 
	ADS1292_REG[CONFIG1] =	0x00;		//0000 0aaa	[7] 0连续转换模式  [6:3] 必须为0  
	ADS1292_REG[CONFIG1] |=	Ads1292_Config1.Data_Rate;//[2:0] aaa 采样率设置采样率 125HZ
	
	ADS1292_REG[CONFIG2] =	0x00;		//1abc d0e1	[7] 必须为1  [2] 必须为0  [0] 设置测试信号为1HZ、±1mV方波 
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Pdb_Loff_Comp<<6;	//[6]a 导联脱落比较器是否掉电
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Pdb_Refbuf<<5;		//[5]b 内部参考缓冲器是否掉电
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Vref<<4;					//[4]c 内部参考电压设置，默认2.42V
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Clk_EN<<3;				//[3]d CLK引脚输出时钟脉冲？
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Int_Test<<1;			//[1]e 是否打开内部测试信号,
	ADS1292_REG[CONFIG2] |=	0x80;//设置默认位
	
	ADS1292_REG[LOFF] =	0xF0;//[7:5]	设置导联脱落比较器阈值 [4]	必须为1 		[3:2] 导联脱落电流幅值		[1]	必须为0	[0]	导联脱落检测方式 0 DC 1 AC 

	ADS1292_REG[CH1SET] =	0x00;	 //abbb cccc
	ADS1292_REG[CH1SET] |=Ads1292_Ch1set.PD<<7;		//[7]  a 		通道1断电？
	ADS1292_REG[CH1SET] |=Ads1292_Ch1set.GAIN<<4;	//[6:4]bbb	设置PGA增益
	ADS1292_REG[CH1SET] |=Ads1292_Ch1set.MUX;			//[3:0]cccc	设置通道1输入方式

	ADS1292_REG[CH2SET] =	0x00;	//abbb cccc
	ADS1292_REG[CH2SET] |=Ads1292_Ch2set.PD<<7;		//[7]  a 		通道2断电？
	ADS1292_REG[CH2SET] |=Ads1292_Ch2set.GAIN<<4;	//[6:4]bbb	设置PGA增益
	ADS1292_REG[CH2SET] |=Ads1292_Ch2set.MUX;			//[3:0]cccc	设置通道2输入方式
	
	ADS1292_REG[RLD_SENS] = 0X00; //11ab cdef	[7:6] 11 PGA斩波频率	fMOD/4 
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Pdb_Rld<<5;					//[5]a	该位决定RLD缓冲电源状态
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld_Loff_Sense<<4;	//[4]b	该位使能RLD导联脱落检测功能
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld2N<<3;						//[3]c	这个位控制通道2负输入	用于右腿驱动的输出
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld2P<<2;						//[2]d	该位控制通道2正输入		用于右腿驱动的输出
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld1N<<1;						//[1]e	这个位控制通道1负输入	用于右腿驱动的输出
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld1P;							//[0]f	该位控制通道1正输入		用于右腿驱动的输出	
	ADS1292_REG[RLD_SENS] |=	0xc0;//设置默认位

	ADS1292_REG[LOFF_SENS] = 0X00;  //00ab cdef	[7:6] 必须为0
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Flip2<<5;		//[5]a	这个位用于控制导联脱落检测通道2的电流的方向
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Flip1<<4;		//[4]b	这个位控制用于导联脱落检测通道1的电流的方向
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff2N<<3;	//[3]c	该位控制通道2负输入端的导联脱落检测
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff2P<<2;	//[2]d	该位控制通道2正输入端的导联脱落检测
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff1N<<1;	//[1]e	该位控制通道1负输入端的导联脱落检测
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff1P;			//[0]f	该位控制通道1正输入端的导联脱落检测
	
	ADS1292_REG[LOFF_STAT] =	0x00;		//[6]0 设置fCLK和fMOD之间的模分频比 fCLK=fMOD/4  [4:0]只读，导联脱落和电极连接状态
	
	ADS1292_REG[RESP1] = 0X00;//abcc cc1d
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_DemodEN<<7;//[7]a		这个位启用和禁用通道1上的解调电路		
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_modEN<<6;	//[6]b		这个位启用和禁用通道1上的调制电路	
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_ph<<2;			//[5:2]c	这些位控制呼吸解调控制信号的相位	
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_Ctrl;			//[0]d		这个位设置呼吸回路的模式
	ADS1292_REG[RESP1] |=	0x02;//设置默认位	
	
	ADS1292_REG[RESP2] = 0x00; //a000 0bc1	[6:3]必须为0 [0]必须为1
	ADS1292_REG[RESP2] |=	Ads1292_Resp2.Calib<<7;				//[7]a 启动通道偏移校正？
	ADS1292_REG[RESP2] |=	Ads1292_Resp2.freq<<2;				//[2]b 呼吸频率设置
	ADS1292_REG[RESP2] |=	Ads1292_Resp2.Rldref_Int<<1;	//[1]c RLDREF信号源外部馈电？
	ADS1292_REG[RESP2] |= 0X01;//设置默认位	
 
	ADS1292_REG[GPIO] =	0x0C;			//GPIO设为输入		[7:4]必须为0	 [3:2]11 GPIO为输入 [1:0] 设置输入时，指示引脚电平，设置输出时控制引脚电平
}

//通过SPI总线与ADS1292通信
u8 ADS1292_SPI(u8 com)
{	
		return SPI3_ReadWrite_Byte(com);
}

//写命令
void ADS1292_Send_CMD(u8 data)
{
		ADS1292_CS=0;
		delay_us(100);
		ADS1292_SPI(data);		
		delay_us(100);	
		ADS1292_CS=1;
}


/*ADS1291、ADS1292和ADS1292R串行接口以字节形式解码命令，需要4个tCLK周期来解码和执行.
因此，在发送多字节命令时，4 tCLK周期必须将一个字节(或操作码)的结束与下一个字节(或操作码)分开。
假设CLK（时钟）为512 kHz，则tSDECODE (4 tCLK)为7.8125 us。
当SCLK（数据速率）为16mhz时，一个字节可以在500ns中传输，此字节传输时间不符合tSDECODE规范;
因此，必须插入一个延迟，以便第二个字节的末尾晚于7.3125us到达。
如果SCLK为1 MHz，则在8u秒内传输一个字节。由于此传输时间超过tSDECODE规范，处理器可以不延迟地发送后续字节。
在后面的场景中，可以对串行端口进行编程，使其从每个循环的单字节传输转移到多个字节*/

//读写多个寄存器
void ADS1292_WR_REGS(u8 reg,u8 len,u8 *data)
{
		u8 i;
		ADS1292_CS=0;	
		delay_us(100);
		ADS1292_SPI(reg);
		delay_us(100);
		ADS1292_SPI(len-1);
		if(reg&0x40) //写
		{
				for(i=0;i<len;i++)
				{
						delay_us(100);		
						ADS1292_SPI(*data);
						data++;				
				}			
		}
		else //读		
		{
				for(i=0;i<len;i++)
				{
						delay_us(100);		
						*data = ADS1292_SPI(0);
						data++;
				}
		}			
		delay_us(100);	
		ADS1292_CS=1;
}

//寄存器数组写入寄存器
u8 ADS1292_WRITE_REGBUFF(void)
{
		u8 i,res=0;
		u8 REG_Cache[12];	//存储寄存器数据
		ADS1292_SET_REGBUFF();//设置寄存器数组		
		ADS1292_WR_REGS(WREG|CONFIG1,11,ADS1292_REG+1);//数组变量写入寄存器
		delay_ms(10);		
		ADS1292_WR_REGS(RREG|ID,12,REG_Cache);//读寄存器
		delay_ms(10);	
		
	#ifdef DEBUG_ADS1292	
		printf("WRITE REG:\r\n");
		for(i=0;i<12;i++	)//要写的数据								
				printf("%d %x\r\n",i,ADS1292_REG[i]);	
		printf("READ REG:\r\n");
	#endif	
	
	
		for(i=0;i<12;i++	)	//检查寄存器	
		{						
				if(ADS1292_REG[i] != REG_Cache[i])
				{
						if(i!= 0 && i!=8 && i != 11)	//0 8 和11是ID 导联脱落和GPIO相关
								res=1;
						else
								continue;
				}					
			#ifdef DEBUG_ADS1292
				printf("%d %x\r\n",i,REG_Cache[i]); //读到的数据			
			#endif
		}	

		#ifdef DEBUG_ADS1292	
			if(res == 0)
					printf("REG write success\r\n");
			else		
					printf("REG write err\r\n");
		#endif
		return res;				
}

void ADS1292_PowerOnInit(void)
{	
		u8 i;
		u8 REG_Cache[12];	
	
//		ADS_CLKSEL=1;//启用内部时钟
//		ADS_START=0; //停止数据输出	
//		ADS_RESET=0; //复位
//		delay_ms(1000);
//		ADS_RESET=1;//芯片上电，可以使用	
//		delay_ms(100);	//等待稳定
	
		ADS1292_Send_CMD(SDATAC);//发送停止连续读取数据命令
		delay_ms(100);	
		ADS1292_Send_CMD(RESET);//复位
		delay_ms(1000);		
		ADS1292_Send_CMD(SDATAC);//发送停止连续读取数据命令
		delay_ms(100);		
	
#ifdef DEBUG_ADS1292	
		ADS1292_WR_REGS(RREG|ID,12,REG_Cache);
		printf("read default REG:\r\n");
		for(i=0;i<12;i++	)	//读默认寄存器
				printf("%d %x\r\n",i,REG_Cache[i]);		
#endif
		//ADS1292_Send_CMD(STANDBY);//进入待机模式	
}

//设置通道1内部1mV测试信号
u8 ADS1292_Single_Test(void) //注意1292R开了呼吸解调，会对通道一的内部测试信号波形造成影响，这里只参考通道2即可，1292不受影响
{
		u8 res=0;
		Ads1292_Config2.Int_Test = INT_TEST_ON;//打开内部测试信号
		Ads1292_Ch1set.MUX=MUX_Test_signal;//测试信号输入	
		Ads1292_Ch2set.MUX=MUX_Test_signal;//测试信号输入	
		
		if(ADS1292_WRITE_REGBUFF())//写入寄存器
				res=1;	
		delay_ms(10);			
		return res;		
}
//设置内部噪声测试
u8 ADS1292_Noise_Test(void)
{
		u8 res=0;
		Ads1292_Config2.Int_Test = INT_TEST_OFF;//关内部测试信号
		Ads1292_Ch1set.MUX = MUX_input_shorted;//输入短路	
		Ads1292_Ch2set.MUX = MUX_input_shorted;//输入短路	

		if(ADS1292_WRITE_REGBUFF())//写入寄存器
				res=1;	
		delay_ms(10);			
		return res;			
}

//正常信号采集模式
u8 ADS1292_Single_Read(void)
{
		u8 res=0;
		Ads1292_Config2.Int_Test = INT_TEST_OFF;//关内部测试信号
		Ads1292_Ch1set.MUX = MUX_Normal_input;//普通电极输入
		Ads1292_Ch2set.MUX = MUX_Normal_input;//普通电极输入
	
		if(ADS1292_WRITE_REGBUFF())//写入寄存器
				res=1;
		delay_ms(10);		
		return res;		
}	

//配置ads1292采集方式
u8 Set_ADS1292_Collect(u8 mode)
{
		u8 res;
		
		delay_ms(10);
		switch(mode)//设置采集方式
		{
				case 0:
					res =ADS1292_Single_Read();
				break;
				case 1:
					res =ADS1292_Single_Test();
				break;
				case 2:
					res =ADS1292_Noise_Test();
				break;
		}
		if(res)return 1;			//寄存器设置失败
		ADS1292_Send_CMD(RDATAC); 	//启动连续模式
		delay_ms(10);
		ADS1292_Send_CMD(START);	//发送开始数据转换（等效于拉高START引脚）
		delay_ms(10);
		return 0;
}

/*功能：把采到的3个字节转成有符号32位数 */
s32 get_volt(u32 num)
{		
			s32 temp;			
			temp = num;
			temp <<= 8;
			temp >>= 8;
			return temp;
}


/*
//-----------------------------------------------------------------
// void ADS1292_Init(void)
//-----------------------------------------------------------------
//
// 函数功能: ADS1292初始化
// 入口参数: 无
// 返 回 值: 无
// 注意事项: 无
//
//-----------------------------------------------------------------
void ADS1292_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOE_CLK_ENABLE();

  // ADS1292_DRDY -> PE9
  GPIO_InitStruct.Pin   = GPIO_PIN_9;       // 配置ADS1292_DRDY
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;  // 输入
  GPIO_InitStruct.Pull  = GPIO_PULLUP;      // 上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH; 	// 高速
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);  	// 初始化

  // ADS1292_START -> PE7
  // ADS1292_PWDN  -> PE8
  // ADS1292_CS  	 -> PE10
  // ADS1292_GPIO1 -> PE11
  // ADS1292_GPIO2 -> PE12
  GPIO_InitStruct.Pin   = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_10 |
                          GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP; 	// 推挽输出
  GPIO_InitStruct.Pull  = GPIO_PULLUP;         	// 上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;    	// 高速
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);     	// 初始化

//  SPI5_Init(); // SPI初始化
}

//-----------------------------------------------------------------
// void ADS1292_PowerOnInit(void)
//-----------------------------------------------------------------
//
// 函数功能: ADS1292上电复位
// 入口参数: 无
// 返 回 值: 无
// 注意事项: 无
//
//-----------------------------------------------------------------
void ADS1292_PowerOnInit(void)
{
	u8 device_id;

  ADS1292_START = 1;
  ADS1292_CS = 1;
  ADS1292_PWDN = 0; // 进入掉电模式
  delay_ms(1000);
  ADS1292_PWDN = 1; // 退出掉电模式
  delay_ms(1000);   // 等待稳定
  ADS1292_PWDN = 0; // 发出复位脉冲
  delay_us(10);
  ADS1292_PWDN = 1;
  delay_ms(1000); // 等待稳定，可以开始使用ADS1292R
	
	ADS1292_START = 0;
	ADS1292_CS = 0;
  SPI5_Send_Byte(SDATAC); // 发送停止连续读取数据命令
	delay_us(10);
	ADS1292_CS = 1;
	
	// 获取芯片ID
	device_id = ADS1292_Read_Reg(RREG | ID);
	while(device_id != 0x73)
	{
		printf("ERROR ID:%02x\r\n",device_id);
		device_id = ADS1292_Read_Reg(RREG | ID);
		HAL_Delay(1000);
	}
	
	delay_us(10);
  ADS1292_Write_Reg(WREG | CONFIG2,  0XE0); // 使用内部参考电压
  delay_ms(10);                            	// 等待内部参考电压稳定
  ADS1292_Write_Reg(WREG | CONFIG1,  0X03); // 设置转换速率为1KSPS
  delay_us(10);
  ADS1292_Write_Reg(WREG | LOFF,     0XF0);	// 该寄存器配置引出检测操作
  delay_us(10);
  ADS1292_Write_Reg(WREG | CH1SET,   0X00); // 增益6，连接到电极
  delay_us(10);
  ADS1292_Write_Reg(WREG | CH2SET,   0X00); // 增益6，连接到电极
  delay_us(10);
  ADS1292_Write_Reg(WREG | RLD_SENS, 0xEF);
  delay_us(10);
  ADS1292_Write_Reg(WREG | LOFF_SENS,0x0F);
  delay_us(10);
	ADS1292_Write_Reg(WREG | LOFF_STAT,0x00);
  delay_us(10);
  ADS1292_Write_Reg(WREG | RESP1,    0xEA); // 开启呼吸检测（ADS1292R特有）
  delay_us(10);
  ADS1292_Write_Reg(WREG | RESP2,    0x03);
  delay_us(10);
  ADS1292_Write_Reg(WREG | GPIO,     0x0C);
  delay_us(10);
}

//-----------------------------------------------------------------
// void ADS1292_Write_Reg(u8 com, u8 data)
//-----------------------------------------------------------------
//
// 函数功能: 对ADS1292的内部寄存器进行写操作
// 入口参数: 无
// 返 回 值: 无
// 注意事项: 无
//
//-----------------------------------------------------------------
void ADS1292_Write_Reg(u8 addr, u8 data)
{
	ADS1292_CS = 0;				// 片选拉低
  SPI5_Send_Byte(addr);	// 包含命令操作码和寄存器地址
  delay_us(10);
  SPI5_Send_Byte(0x00);	// 要读取的寄存器数+1
  delay_us(10);
  SPI5_Send_Byte(data);	// 写入的数据
	delay_us(10);
	ADS1292_CS = 1;				// 片选置高
}

//-----------------------------------------------------------------
// u8 ADS1292_Read_Reg(u8 addr)
//-----------------------------------------------------------------
//
// 函数功能: 对ADS1292的内部寄存器进行读操作
// 入口参数: 无
// 返 回 值: 无
// 注意事项: 无
//
//-----------------------------------------------------------------
u8 ADS1292_Read_Reg(u8 addr)
{
  u8 Rxdata;
	ADS1292_CS = 0;
  SPI5_Send_Byte(addr); 			// 包含命令操作码和寄存器地址
  delay_us(10);
  SPI5_Send_Byte(0x00); 			// 要读取的寄存器数+1
  delay_us(10);
  Rxdata = SPI5_Read_Byte(); 	// 读取的数据
	delay_us(10);
	ADS1292_CS = 1;
  return Rxdata;
}

//-----------------------------------------------------------------
// u8 ADS1292_Read_Data(u8 addr)
//-----------------------------------------------------------------
//
// 函数功能: 读取ADS1292的数据
// 入口参数: 无
// 返 回 值: 无
// 注意事项: 无
//
//-----------------------------------------------------------------
void ADS1292_Read_Data(u8 *data)
{
  u8 i;
	ADS1292_CS = 0;
  SPI5_Send_Byte(RDATAC);		// 发送启动连续读取数据命令
  delay_us(10);
	ADS1292_CS = 1;						
  ADS1292_START = 1; 				// 启动转换
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 1);	// 等待DRDY信号拉低
  ADS1292_CS = 0;
  for (i = 0; i < 9; i++)		// 连续读取9个数据
  {
    *data = SPI5_Read_Byte();
    data++;
  }
  ADS1292_START = 0;				// 停止转换
  SPI5_Send_Byte(SDATAC);		// 发送停止连续读取数据命令
	delay_us(10);
	ADS1292_CS = 1;
}
//-----------------------------------------------------------------
// End Of File
//----------------------------------------------------------------- 

const s16 b[32] = 
	{
        -2,   -101,    -64,    129,    -55,    198,    809,     69,    363,
     1098,  -1673,  -1616,   -235,  -7079,  -4845,  12973,  12973,  -4845,
    -7079,   -235,  -1616,  -1673,   1098,    363,     69,    809,    198,
      -55,    129,    -64,   -101,     -2
	};	

u8 XL=0;
s32 X[32]={0};
s32 Z[300]={0};
s32 ch2_temp;

void xl(void)   //0.0005755s  滤波及心率计算
{
//			int i=0,j=0,n=0,n1=0,m=0,m1=0,m2=0,t=0,p0=0,p1=0,e;
			int j=0,n=0,n1=0,m=0,m1=0,m2=0,t=0;
      int max=0,max_left=0,max_right=0,max1=0;
			s32 Max1,Max=0XFFFFFFFF;
	    s32 temp=0;
			s32 y=0;

	//滤波  12us

					X[0]=ch2_temp;				
					for(t=0;t<31;t++)     //FIR滤波
						 { 
							 temp += b[t] * X[t];
						 } 
						y=temp;	
					for (j = 31; j > -1 ; j--)  //右移
								X[j+1] = X[j];

					 
			        Z[0]=y;  
	//150us				 
//求第一个波峰  Max：波峰  max :波峰序列

						for(m=0;m<300;m++)
						{
							if (Z[m]>Max)
							{
								 Max=Z[m];
								 max=m;
							}	
						}	
						Max1=0.8*Max;   //取阈值
							
//向左求第二个波峰     max_left： 左波峰序列 
						for(n=max;n>0;n--)
							{
									if(Z[n]>Max1)
									{
										m1=n;	
										if((max-m1)>20)
										 {
											 max_left=m1;
										   break;
										 }											
									}	
							}						

//向右取第二个波峰	    max_right：右波峰序列
            for(n1=max;n1<300;n1++)
							{
							  if (Z[n1]>Max1)
									{
										m2=n1;
										if((m2-max)>20)
										  {
												 max_right=m2;
										   break;
										  }
									}	
							}				 									
//第二个峰值对应序列max1
									 
              if(max_right==0)max1=max_left;
              else max1=max_right;	
	 
//计算心率XL
							XL=abs(60*160/(max-max1));
            // printf("%d\r\n",XL);								
							for (j = 299; j > -1 ; j--)   //右移
							    Z[j+1] = Z[j];
												
}
*/

