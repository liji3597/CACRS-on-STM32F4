//-----------------------------------------------------------------
// ͷ�ļ�����
//-----------------------------------------------------------------
#include "ads1292.h"
#include "spi.h"
#include "delay.h"
#include "callback.h"
//#include <stdlib.h>

 #define DEBUG_ADS1292	//�Ĵ���printf����
 
//-----------------------------------------------------------------

u8 ADS1292_REG[12];		//ads1292�Ĵ�������
ADS1292_CONFIG1 	Ads1292_Config1		={DATA_RATE};																				//CONFIG1
ADS1292_CONFIG2 	Ads1292_Config2		={PDB_LOFF_COMP,PDB_REFBUF,VREF,CLK_EN,INT_TEST};		//CONFIG2
ADS1292_CHSET 		Ads1292_Ch1set		={CNNNLE1_POWER,CNNNLE1_GAIN,CNNNLE1_MUX};					//CH1SET
ADS1292_CHSET 		Ads1292_Ch2set		={CNNNLE2_POWER,CNNNLE2_GAIN,CNNNLE2_MUX};					//CH2SET
ADS1292_RLD_SENS	Ads1292_Rld_Sens	={PDB_RLD,RLD_LOFF_SENSE,RLD2N,RLD2P,RLD1N,RLD1P};	//RLD_SENS
ADS1292_LOFF_SENS	Ads1292_Loff_Sens	={FLIP2,FLIP1,LOFF2N,LOFF2P,LOFF1N,LOFF1P};					//LOFF_SENS
ADS1292_RESP1			Ads1292_Resp1			={RESP_DEMOD_EN1,RESP_MOD_EN,RESP_PH,RESP_CTRL};		//RSP1
ADS1292_RESP2			Ads1292_Resp2			={CALIB,FREQ,RLDREF_INT};														//RSP2



//��ȡ72λ������1100+LOFF_STAT[4:0]+GPIO[1:0]+13��0+2CHx24λ����9�ֽ� ��1�ֽ�8λ��
//	1100	LOFF_STAT[4			3			2			1			0	]	//����������ص���Ϣ��LOFF_STAT�Ĵ�����
//									RLD		1N2N	1N2P	1N1N	1N1P	
//	��	C0 00 00 FF E1 1A FF E1 52	

u8 ADS1292_Read_Data(u8 *data)//72Mʱ���º�����ʱ��Լ10us  8Mʱ���� ������ʱ��Լ 100us  
{		
		u8 i;	
		
		ADS1292_CS=0;//��9���ֽڵ�����  c00000 xxxxxx(����) xxxxxx(�ĵ�)
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
* ��  ��	: ���üĴ��� 
* ��  ��	: CONFIG1���ò���Ƶ��500HZ
						CH1SETͨ��1 2��PGA��������Ϊ2
*******************************************************************************/
void ADS1292_SET_REGBUFF(void)   //
{
	ADS1292_REG[ID] =	ADS1292_DEVICE;//IDֻ��  0X73
	 
	ADS1292_REG[CONFIG1] =	0x00;		//0000 0aaa	[7] 0����ת��ģʽ  [6:3] ����Ϊ0  
	ADS1292_REG[CONFIG1] |=	Ads1292_Config1.Data_Rate;//[2:0] aaa ���������ò����� 125HZ
	
	ADS1292_REG[CONFIG2] =	0x00;		//1abc d0e1	[7] ����Ϊ1  [2] ����Ϊ0  [0] ���ò����ź�Ϊ1HZ����1mV���� 
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Pdb_Loff_Comp<<6;	//[6]a ��������Ƚ����Ƿ����
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Pdb_Refbuf<<5;		//[5]b �ڲ��ο��������Ƿ����
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Vref<<4;					//[4]c �ڲ��ο���ѹ���ã�Ĭ��2.42V
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Clk_EN<<3;				//[3]d CLK�������ʱ�����壿
	ADS1292_REG[CONFIG2] |=	Ads1292_Config2.Int_Test<<1;			//[1]e �Ƿ���ڲ������ź�,
	ADS1292_REG[CONFIG2] |=	0x80;//����Ĭ��λ
	
	ADS1292_REG[LOFF] =	0xF0;//[7:5]	���õ�������Ƚ�����ֵ [4]	����Ϊ1 		[3:2] �������������ֵ		[1]	����Ϊ0	[0]	���������ⷽʽ 0 DC 1 AC 

	ADS1292_REG[CH1SET] =	0x00;	 //abbb cccc
	ADS1292_REG[CH1SET] |=Ads1292_Ch1set.PD<<7;		//[7]  a 		ͨ��1�ϵ磿
	ADS1292_REG[CH1SET] |=Ads1292_Ch1set.GAIN<<4;	//[6:4]bbb	����PGA����
	ADS1292_REG[CH1SET] |=Ads1292_Ch1set.MUX;			//[3:0]cccc	����ͨ��1���뷽ʽ

	ADS1292_REG[CH2SET] =	0x00;	//abbb cccc
	ADS1292_REG[CH2SET] |=Ads1292_Ch2set.PD<<7;		//[7]  a 		ͨ��2�ϵ磿
	ADS1292_REG[CH2SET] |=Ads1292_Ch2set.GAIN<<4;	//[6:4]bbb	����PGA����
	ADS1292_REG[CH2SET] |=Ads1292_Ch2set.MUX;			//[3:0]cccc	����ͨ��2���뷽ʽ
	
	ADS1292_REG[RLD_SENS] = 0X00; //11ab cdef	[7:6] 11 PGAն��Ƶ��	fMOD/4 
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Pdb_Rld<<5;					//[5]a	��λ����RLD�����Դ״̬
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld_Loff_Sense<<4;	//[4]b	��λʹ��RLD���������⹦��
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld2N<<3;						//[3]c	���λ����ͨ��2������	�����������������
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld2P<<2;						//[2]d	��λ����ͨ��2������		�����������������
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld1N<<1;						//[1]e	���λ����ͨ��1������	�����������������
	ADS1292_REG[RLD_SENS] |=Ads1292_Rld_Sens.Rld1P;							//[0]f	��λ����ͨ��1������		�����������������	
	ADS1292_REG[RLD_SENS] |=	0xc0;//����Ĭ��λ

	ADS1292_REG[LOFF_SENS] = 0X00;  //00ab cdef	[7:6] ����Ϊ0
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Flip2<<5;		//[5]a	���λ���ڿ��Ƶ���������ͨ��2�ĵ����ķ���
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Flip1<<4;		//[4]b	���λ�������ڵ���������ͨ��1�ĵ����ķ���
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff2N<<3;	//[3]c	��λ����ͨ��2������˵ĵ���������
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff2P<<2;	//[2]d	��λ����ͨ��2������˵ĵ���������
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff1N<<1;	//[1]e	��λ����ͨ��1������˵ĵ���������
	ADS1292_REG[LOFF_SENS] |=Ads1292_Loff_Sens.Loff1P;			//[0]f	��λ����ͨ��1������˵ĵ���������
	
	ADS1292_REG[LOFF_STAT] =	0x00;		//[6]0 ����fCLK��fMOD֮���ģ��Ƶ�� fCLK=fMOD/4  [4:0]ֻ������������͵缫����״̬
	
	ADS1292_REG[RESP1] = 0X00;//abcc cc1d
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_DemodEN<<7;//[7]a		���λ���úͽ���ͨ��1�ϵĽ����·		
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_modEN<<6;	//[6]b		���λ���úͽ���ͨ��1�ϵĵ��Ƶ�·	
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_ph<<2;			//[5:2]c	��Щλ���ƺ�����������źŵ���λ	
	ADS1292_REG[RESP1] |=Ads1292_Resp1.RESP_Ctrl;			//[0]d		���λ���ú�����·��ģʽ
	ADS1292_REG[RESP1] |=	0x02;//����Ĭ��λ	
	
	ADS1292_REG[RESP2] = 0x00; //a000 0bc1	[6:3]����Ϊ0 [0]����Ϊ1
	ADS1292_REG[RESP2] |=	Ads1292_Resp2.Calib<<7;				//[7]a ����ͨ��ƫ��У����
	ADS1292_REG[RESP2] |=	Ads1292_Resp2.freq<<2;				//[2]b ����Ƶ������
	ADS1292_REG[RESP2] |=	Ads1292_Resp2.Rldref_Int<<1;	//[1]c RLDREF�ź�Դ�ⲿ���磿
	ADS1292_REG[RESP2] |= 0X01;//����Ĭ��λ	
 
	ADS1292_REG[GPIO] =	0x0C;			//GPIO��Ϊ����		[7:4]����Ϊ0	 [3:2]11 GPIOΪ���� [1:0] ��������ʱ��ָʾ���ŵ�ƽ���������ʱ�������ŵ�ƽ
}

//ͨ��SPI������ADS1292ͨ��
u8 ADS1292_SPI(u8 com)
{	
		return SPI3_ReadWrite_Byte(com);
}

//д����
void ADS1292_Send_CMD(u8 data)
{
		ADS1292_CS=0;
		delay_us(100);
		ADS1292_SPI(data);		
		delay_us(100);	
		ADS1292_CS=1;
}


/*ADS1291��ADS1292��ADS1292R���нӿ����ֽ���ʽ���������Ҫ4��tCLK�����������ִ��.
��ˣ��ڷ��Ͷ��ֽ�����ʱ��4 tCLK���ڱ��뽫һ���ֽ�(�������)�Ľ�������һ���ֽ�(�������)�ֿ���
����CLK��ʱ�ӣ�Ϊ512 kHz����tSDECODE (4 tCLK)Ϊ7.8125 us��
��SCLK���������ʣ�Ϊ16mhzʱ��һ���ֽڿ�����500ns�д��䣬���ֽڴ���ʱ�䲻����tSDECODE�淶;
��ˣ��������һ���ӳ٣��Ա�ڶ����ֽڵ�ĩβ����7.3125us���
���SCLKΪ1 MHz������8u���ڴ���һ���ֽڡ����ڴ˴���ʱ�䳬��tSDECODE�淶�����������Բ��ӳٵط��ͺ����ֽڡ�
�ں���ĳ����У����ԶԴ��ж˿ڽ��б�̣�ʹ���ÿ��ѭ���ĵ��ֽڴ���ת�Ƶ�����ֽ�*/

//��д����Ĵ���
void ADS1292_WR_REGS(u8 reg,u8 len,u8 *data)
{
		u8 i;
		ADS1292_CS=0;	
		delay_us(100);
		ADS1292_SPI(reg);
		delay_us(100);
		ADS1292_SPI(len-1);
		if(reg&0x40) //д
		{
				for(i=0;i<len;i++)
				{
						delay_us(100);		
						ADS1292_SPI(*data);
						data++;				
				}			
		}
		else //��		
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

//�Ĵ�������д��Ĵ���
u8 ADS1292_WRITE_REGBUFF(void)
{
		u8 i,res=0;
		u8 REG_Cache[12];	//�洢�Ĵ�������
		ADS1292_SET_REGBUFF();//���üĴ�������		
		ADS1292_WR_REGS(WREG|CONFIG1,11,ADS1292_REG+1);//�������д��Ĵ���
		delay_ms(10);		
		ADS1292_WR_REGS(RREG|ID,12,REG_Cache);//���Ĵ���
		delay_ms(10);	
		
	#ifdef DEBUG_ADS1292	
		printf("WRITE REG:\r\n");
		for(i=0;i<12;i++	)//Ҫд������								
				printf("%d %x\r\n",i,ADS1292_REG[i]);	
		printf("READ REG:\r\n");
	#endif	
	
	
		for(i=0;i<12;i++	)	//���Ĵ���	
		{						
				if(ADS1292_REG[i] != REG_Cache[i])
				{
						if(i!= 0 && i!=8 && i != 11)	//0 8 ��11��ID ���������GPIO���
								res=1;
						else
								continue;
				}					
			#ifdef DEBUG_ADS1292
				printf("%d %x\r\n",i,REG_Cache[i]); //����������			
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
	
//		ADS_CLKSEL=1;//�����ڲ�ʱ��
//		ADS_START=0; //ֹͣ�������	
//		ADS_RESET=0; //��λ
//		delay_ms(1000);
//		ADS_RESET=1;//оƬ�ϵ磬����ʹ��	
//		delay_ms(100);	//�ȴ��ȶ�
	
		ADS1292_Send_CMD(SDATAC);//����ֹͣ������ȡ��������
		delay_ms(100);	
		ADS1292_Send_CMD(RESET);//��λ
		delay_ms(1000);		
		ADS1292_Send_CMD(SDATAC);//����ֹͣ������ȡ��������
		delay_ms(100);		
	
#ifdef DEBUG_ADS1292	
		ADS1292_WR_REGS(RREG|ID,12,REG_Cache);
		printf("read default REG:\r\n");
		for(i=0;i<12;i++	)	//��Ĭ�ϼĴ���
				printf("%d %x\r\n",i,REG_Cache[i]);		
#endif
		//ADS1292_Send_CMD(STANDBY);//�������ģʽ	
}

//����ͨ��1�ڲ�1mV�����ź�
u8 ADS1292_Single_Test(void) //ע��1292R���˺�����������ͨ��һ���ڲ������źŲ������Ӱ�죬����ֻ�ο�ͨ��2���ɣ�1292����Ӱ��
{
		u8 res=0;
		Ads1292_Config2.Int_Test = INT_TEST_ON;//���ڲ������ź�
		Ads1292_Ch1set.MUX=MUX_Test_signal;//�����ź�����	
		Ads1292_Ch2set.MUX=MUX_Test_signal;//�����ź�����	
		
		if(ADS1292_WRITE_REGBUFF())//д��Ĵ���
				res=1;	
		delay_ms(10);			
		return res;		
}
//�����ڲ���������
u8 ADS1292_Noise_Test(void)
{
		u8 res=0;
		Ads1292_Config2.Int_Test = INT_TEST_OFF;//���ڲ������ź�
		Ads1292_Ch1set.MUX = MUX_input_shorted;//�����·	
		Ads1292_Ch2set.MUX = MUX_input_shorted;//�����·	

		if(ADS1292_WRITE_REGBUFF())//д��Ĵ���
				res=1;	
		delay_ms(10);			
		return res;			
}

//�����źŲɼ�ģʽ
u8 ADS1292_Single_Read(void)
{
		u8 res=0;
		Ads1292_Config2.Int_Test = INT_TEST_OFF;//���ڲ������ź�
		Ads1292_Ch1set.MUX = MUX_Normal_input;//��ͨ�缫����
		Ads1292_Ch2set.MUX = MUX_Normal_input;//��ͨ�缫����
	
		if(ADS1292_WRITE_REGBUFF())//д��Ĵ���
				res=1;
		delay_ms(10);		
		return res;		
}	

//����ads1292�ɼ���ʽ
u8 Set_ADS1292_Collect(u8 mode)
{
		u8 res;
		
		delay_ms(10);
		switch(mode)//���òɼ���ʽ
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
		if(res)return 1;			//�Ĵ�������ʧ��
		ADS1292_Send_CMD(RDATAC); 	//��������ģʽ
		delay_ms(10);
		ADS1292_Send_CMD(START);	//���Ϳ�ʼ����ת������Ч������START���ţ�
		delay_ms(10);
		return 0;
}

/*���ܣ��Ѳɵ���3���ֽ�ת���з���32λ�� */
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
// ��������: ADS1292��ʼ��
// ��ڲ���: ��
// �� �� ֵ: ��
// ע������: ��
//
//-----------------------------------------------------------------
void ADS1292_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  __HAL_RCC_GPIOE_CLK_ENABLE();

  // ADS1292_DRDY -> PE9
  GPIO_InitStruct.Pin   = GPIO_PIN_9;       // ����ADS1292_DRDY
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;  // ����
  GPIO_InitStruct.Pull  = GPIO_PULLUP;      // ����
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH; 	// ����
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);  	// ��ʼ��

  // ADS1292_START -> PE7
  // ADS1292_PWDN  -> PE8
  // ADS1292_CS  	 -> PE10
  // ADS1292_GPIO1 -> PE11
  // ADS1292_GPIO2 -> PE12
  GPIO_InitStruct.Pin   = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_10 |
                          GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP; 	// �������
  GPIO_InitStruct.Pull  = GPIO_PULLUP;         	// ����
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;    	// ����
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);     	// ��ʼ��

//  SPI5_Init(); // SPI��ʼ��
}

//-----------------------------------------------------------------
// void ADS1292_PowerOnInit(void)
//-----------------------------------------------------------------
//
// ��������: ADS1292�ϵ縴λ
// ��ڲ���: ��
// �� �� ֵ: ��
// ע������: ��
//
//-----------------------------------------------------------------
void ADS1292_PowerOnInit(void)
{
	u8 device_id;

  ADS1292_START = 1;
  ADS1292_CS = 1;
  ADS1292_PWDN = 0; // �������ģʽ
  delay_ms(1000);
  ADS1292_PWDN = 1; // �˳�����ģʽ
  delay_ms(1000);   // �ȴ��ȶ�
  ADS1292_PWDN = 0; // ������λ����
  delay_us(10);
  ADS1292_PWDN = 1;
  delay_ms(1000); // �ȴ��ȶ������Կ�ʼʹ��ADS1292R
	
	ADS1292_START = 0;
	ADS1292_CS = 0;
  SPI5_Send_Byte(SDATAC); // ����ֹͣ������ȡ��������
	delay_us(10);
	ADS1292_CS = 1;
	
	// ��ȡоƬID
	device_id = ADS1292_Read_Reg(RREG | ID);
	while(device_id != 0x73)
	{
		printf("ERROR ID:%02x\r\n",device_id);
		device_id = ADS1292_Read_Reg(RREG | ID);
		HAL_Delay(1000);
	}
	
	delay_us(10);
  ADS1292_Write_Reg(WREG | CONFIG2,  0XE0); // ʹ���ڲ��ο���ѹ
  delay_ms(10);                            	// �ȴ��ڲ��ο���ѹ�ȶ�
  ADS1292_Write_Reg(WREG | CONFIG1,  0X03); // ����ת������Ϊ1KSPS
  delay_us(10);
  ADS1292_Write_Reg(WREG | LOFF,     0XF0);	// �üĴ�����������������
  delay_us(10);
  ADS1292_Write_Reg(WREG | CH1SET,   0X00); // ����6�����ӵ��缫
  delay_us(10);
  ADS1292_Write_Reg(WREG | CH2SET,   0X00); // ����6�����ӵ��缫
  delay_us(10);
  ADS1292_Write_Reg(WREG | RLD_SENS, 0xEF);
  delay_us(10);
  ADS1292_Write_Reg(WREG | LOFF_SENS,0x0F);
  delay_us(10);
	ADS1292_Write_Reg(WREG | LOFF_STAT,0x00);
  delay_us(10);
  ADS1292_Write_Reg(WREG | RESP1,    0xEA); // ����������⣨ADS1292R���У�
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
// ��������: ��ADS1292���ڲ��Ĵ�������д����
// ��ڲ���: ��
// �� �� ֵ: ��
// ע������: ��
//
//-----------------------------------------------------------------
void ADS1292_Write_Reg(u8 addr, u8 data)
{
	ADS1292_CS = 0;				// Ƭѡ����
  SPI5_Send_Byte(addr);	// �������������ͼĴ�����ַ
  delay_us(10);
  SPI5_Send_Byte(0x00);	// Ҫ��ȡ�ļĴ�����+1
  delay_us(10);
  SPI5_Send_Byte(data);	// д�������
	delay_us(10);
	ADS1292_CS = 1;				// Ƭѡ�ø�
}

//-----------------------------------------------------------------
// u8 ADS1292_Read_Reg(u8 addr)
//-----------------------------------------------------------------
//
// ��������: ��ADS1292���ڲ��Ĵ������ж�����
// ��ڲ���: ��
// �� �� ֵ: ��
// ע������: ��
//
//-----------------------------------------------------------------
u8 ADS1292_Read_Reg(u8 addr)
{
  u8 Rxdata;
	ADS1292_CS = 0;
  SPI5_Send_Byte(addr); 			// �������������ͼĴ�����ַ
  delay_us(10);
  SPI5_Send_Byte(0x00); 			// Ҫ��ȡ�ļĴ�����+1
  delay_us(10);
  Rxdata = SPI5_Read_Byte(); 	// ��ȡ������
	delay_us(10);
	ADS1292_CS = 1;
  return Rxdata;
}

//-----------------------------------------------------------------
// u8 ADS1292_Read_Data(u8 addr)
//-----------------------------------------------------------------
//
// ��������: ��ȡADS1292������
// ��ڲ���: ��
// �� �� ֵ: ��
// ע������: ��
//
//-----------------------------------------------------------------
void ADS1292_Read_Data(u8 *data)
{
  u8 i;
	ADS1292_CS = 0;
  SPI5_Send_Byte(RDATAC);		// ��������������ȡ��������
  delay_us(10);
	ADS1292_CS = 1;						
  ADS1292_START = 1; 				// ����ת��
  while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == 1);	// �ȴ�DRDY�ź�����
  ADS1292_CS = 0;
  for (i = 0; i < 9; i++)		// ������ȡ9������
  {
    *data = SPI5_Read_Byte();
    data++;
  }
  ADS1292_START = 0;				// ֹͣת��
  SPI5_Send_Byte(SDATAC);		// ����ֹͣ������ȡ��������
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

void xl(void)   //0.0005755s  �˲������ʼ���
{
//			int i=0,j=0,n=0,n1=0,m=0,m1=0,m2=0,t=0,p0=0,p1=0,e;
			int j=0,n=0,n1=0,m=0,m1=0,m2=0,t=0;
      int max=0,max_left=0,max_right=0,max1=0;
			s32 Max1,Max=0XFFFFFFFF;
	    s32 temp=0;
			s32 y=0;

	//�˲�  12us

					X[0]=ch2_temp;				
					for(t=0;t<31;t++)     //FIR�˲�
						 { 
							 temp += b[t] * X[t];
						 } 
						y=temp;	
					for (j = 31; j > -1 ; j--)  //����
								X[j+1] = X[j];

					 
			        Z[0]=y;  
	//150us				 
//���һ������  Max������  max :��������

						for(m=0;m<300;m++)
						{
							if (Z[m]>Max)
							{
								 Max=Z[m];
								 max=m;
							}	
						}	
						Max1=0.8*Max;   //ȡ��ֵ
							
//������ڶ�������     max_left�� �󲨷����� 
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

//����ȡ�ڶ�������	    max_right���Ҳ�������
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
//�ڶ�����ֵ��Ӧ����max1
									 
              if(max_right==0)max1=max_left;
              else max1=max_right;	
	 
//��������XL
							XL=abs(60*160/(max-max1));
            // printf("%d\r\n",XL);								
							for (j = 299; j > -1 ; j--)   //����
							    Z[j+1] = Z[j];
												
}
*/

