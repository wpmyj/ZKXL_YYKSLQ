/**
  ******************************************************************************
  * @file   	hal_nrf.c
  * @author  	Tian erjun
  * @version 	V1.0.0.0
  * @date   	2015.11.05
  * @brief   	hal function for nrf moulde
  ******************************************************************************
  */

#include "main.h"
#include "nrf.h"
#include "app_send_data_process.h"

#ifdef NRF_DEBUG
#define nrf_debug  printf
#else
#define nrf_debug(...)
#endif

extern nrf_communication_t nrf_data;
extern uint16_t            list_tcb_table[UID_LIST_TABLE_SUM][WHITE_TABLE_LEN];
extern wl_typedef          wl;
extern revicer_typedef     revicer;

u8 txbuf[52] ={0x74,0x00,0xc0,0x8d,0x05,0xdc,0x02,0x07,0xe7,0xe7,
							 0xe7,0xe7,0xe7,0xef,0x0b,0x01,0x07,0x33,0x34,0xa9};
 
u8 txbuf1[52]={0xFD,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
							 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

#ifdef ZL_RP551_MAIN_F
void nrf1_spi_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);  
	
	/* Configure SPI_MISO Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF1_SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(NRF1_SPI_MISO_PORT, &GPIO_InitStructure);

	/* Configure SPI_MOSI Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF1_SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(NRF1_SPI_MOSI_PORT, &GPIO_InitStructure);

	/* Configure SPI_SCK Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF1_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(NRF1_SPI_SCK_PORT, &GPIO_InitStructure);

	/* Configure SPI_CSN Pin */								//CSN 配置
	GPIO_InitStructure.GPIO_Pin   = NRF1_SPI_CSN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(NRF1_SPI_CSN_PORT, &GPIO_InitStructure);

	/* Configure SPI_CE Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF1_SPI_CE_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(NRF1_SPI_CE_PORT, &GPIO_InitStructure);

	/* Configure SPI_IRQ Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF1_SPI_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(NRF1_SPI_IRQ_PORT, &GPIO_InitStructure);

	/* NRF1_SPI相关参数配置 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	/* 空闲为低 */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	/* 第一个电平读取信号  模式0 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	/* 不超过2M */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStructure);

  /* Connect EXTI5 Line to PC.05 pin */
  GPIO_EXTILineConfig(NRF1_RFIRQ_PortSource, GPIO_PinSource5);
	
	EXTI_InitStructure.EXTI_Line    = NRF1_EXTI_LINE_RFIRQ;
	EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* NRF1_SPI中断配置 */
	NVIC_PriorityGroupConfig(SYSTEM_MVIC_GROUP_SET);
	NVIC_InitStructure.NVIC_IRQChannel = NRF1_RFIRQ_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NRF1_PREEMPTION_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NRF_SUB_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	SPI_Cmd(SPI1, ENABLE);
	NRF1_CSN_HIGH();		
}

void nrf2_spi_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);      
	
	/* Configure SPI_MISO Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF2_SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(NRF2_SPI_MISO_PORT, &GPIO_InitStructure);

	/* Configure SPI_MOSI Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF2_SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(NRF2_SPI_MOSI_PORT, &GPIO_InitStructure);

	/* Configure SPI_SCK Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF2_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(NRF2_SPI_SCK_PORT, &GPIO_InitStructure);

	/* Configure SPI_CSN Pin */								//CSN 配置
	GPIO_InitStructure.GPIO_Pin   = NRF2_SPI_CSN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(NRF2_SPI_CSN_PORT, &GPIO_InitStructure);

	/* Configure SPI_CE Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF2_SPI_CE_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(NRF2_SPI_CE_PORT, &GPIO_InitStructure);

	/* Configure SPI_IRQ Pin */
	GPIO_InitStructure.GPIO_Pin   = NRF2_SPI_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(NRF2_SPI_IRQ_PORT, &GPIO_InitStructure);

	/* NRF2_SPI相关参数配置 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	/* 空闲为低 */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	/* 第一个电平读取信号  模式0 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	/* 不超过2M */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI2, &SPI_InitStructure);

	SPI_Cmd(SPI2, ENABLE);
	NRF2_CSN_HIGH();		
}
#endif
/******************************************************************************
  Function:my_nrf_transmit_start
  Description:
  Input:
	    data_buff：	   要发送的数组
			data_buff_len：要发送的数组长度
			nrf_data_type：发送数据类型，有效数据:NRF_DATA_IS_USEFUL
			ACK		:        NRF_DATA_IS_ACK
  Output:
  Return:
  Others:注意：通信方式限制，若向同一UID答题器下发数据，时间要间隔3S以上
******************************************************************************/
void nrf51822_spi_init(void)
{
#ifdef ZL_RP551_MAIN_E
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	/* Configure SPI_MISO Pin */
	GPIO_InitStructure.GPIO_Pin   = SPI_MISO_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(SPI_MISO_PORT, &GPIO_InitStructure);

	/* Configure SPI_MOSI Pin */
	GPIO_InitStructure.GPIO_Pin   = SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(SPI_MOSI_PORT, &GPIO_InitStructure);

	/* Configure SPI_SCK Pin */
	GPIO_InitStructure.GPIO_Pin   = SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_Init(SPI_SCK_PORT, &GPIO_InitStructure);

	/* Configure SPI_CSN Pin */								//CSN_1 配置
	GPIO_InitStructure.GPIO_Pin   = SPI_CSN_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_CSN_PORT, &GPIO_InitStructure);

	/* Configure SPI_CSN Pin */								//CSN_2 配置
	GPIO_InitStructure.GPIO_Pin   = SPI_CSN_PIN_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_CSN_PORT_2, &GPIO_InitStructure);

	/* Configure SPI_CE Pin */
	GPIO_InitStructure.GPIO_Pin   = SPI_CE_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(SPI_CE_PORT, &GPIO_InitStructure);

	/* Configure SPI_IRQ Pin */
	GPIO_InitStructure.GPIO_Pin   = SPI_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SPI_IRQ_PORT, &GPIO_InitStructure);

	/* SPI相关参数配置 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;			//空闲为低
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;		//第一个电平读取信号  模式0
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;		//不超过2M
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStructure);

	/* SPI中断配置 */
	NVIC_PriorityGroupConfig(SYSTEM_MVIC_GROUP_SET);
	NVIC_InitStructure.NVIC_IRQChannel = RFIRQ_EXTI_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NRF1_PREEMPTION_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NRF_SUB_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	GPIO_EXTILineConfig(RFIRQ_PortSource, RFIRQ_PinSource);
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_Line = NRF1_EXTI_LINE_RFIRQ;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	SPI_Cmd(SPI1, ENABLE);
	SPI_CSN_HIGH();	
	SPI_CSN_HIGH_2();	

	GPIOInit_SE2431L();
#endif

#ifdef ZL_RP551_MAIN_F
	nrf1_spi_init();
	nrf2_spi_init();
#endif
}



/*
 * 函数名：SPI_NRF_RW
 * 描述  ：用于向NRF读/写一字节数据
 * 输入  ：写入的数据
 * 输出  ：读取得的数据
 * 调用  ：内部调用
 */
u8 SPI_NRF_RW(u8 dat)
{  	
	/* 当 SPI发送缓冲器非空时等待 */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
	/* 通过 SPI2发送一字节数据 */
  SPI_I2S_SendData(SPI2, dat);
	/* 当SPI接收缓冲器为空时等待 */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI2);
}

/*
 * 函数名：SPI_NRF_WriteReg
 * 描述  ：用于向NRF特定的寄存器写入数据
 * 输入  ：reg:NRF的命令+寄存器地址。
 		   dat:将要向寄存器写入的数据
 * 输出  ：NRF的status寄存器的状态
 * 调用  ：内部调用
 */
u8 SPI_NRF_WriteReg(u8 reg,u8 dat)
{
 	u8 status;
	/*置低CSN，使能SPI传输*/
 	NRF2_CSN_LOW();		
	/*发送命令及寄存器号 */
	status = SPI_NRF_RW(reg); 
	/*向寄存器写入数据*/
	SPI_NRF_RW(dat);       
	/*CSN拉高，完成*/	   
	NRF2_CSN_HIGH();	
	/*返回状态寄存器的值*/
	return(status);
}


/*
 * 函数名：SPI_NRF_ReadReg
 * 描述  ：用于从NRF特定的寄存器读出数据
 * 输入  ：reg:NRF的命令+寄存器地址。
 * 输出  ：寄存器中的数据
 * 调用  ：内部调用
 */
u8 SPI_NRF_ReadReg(u8 reg)
{
 	u8 reg_val;
	/*置低CSN，使能SPI传输*/
 	NRF2_CSN_LOW();		
	/*发送寄存器号*/
	SPI_NRF_RW(reg); 
	/*读取寄存器的值 */
	reg_val = SPI_NRF_RW(0x00);        
	/*CSN拉高，完成*/
	NRF2_CSN_HIGH();		
	return reg_val;
}	

/******************************************************************************
  Function:nrf_transmit_start
  Description:
  Input:	data_buff：	   要发送的数组
			data_buff_len：要发送的数组长度
			nrf_data_type：发送数据类型，有效数据:NRF_DATA_IS_USEFUL
										ACK		:NRF_DATA_IS_ACK
  Output:
  Return:
  Others:注意：通信方式限制，若向同一UID答题器下发数据，时间要间隔3S以上
******************************************************************************/
void nrf_transmit_start( nrf_transmit_parameter_t *t_conf)
{

}

 int spi_wr_data(char addr, char data)
{		
	int reg_val;							
	char reg_val2;									
	NRF2_CSN_LOW();			
	reg_val=SPI_NRF_RW(addr); 
	reg_val2=SPI_NRF_RW(data); 
	NRF2_CSN_HIGH();
	reg_val=(reg_val<<8)+reg_val2;			
	return reg_val; 
}	

void delay( int m)	
{ 
	int i,j,k;
	k=2*m;
	for(i=0;i<k;i++)
	{
		for(j=0;j<k;j++);
	}
}

void si24r2e_read_nvm( void )
{
	int i;
	int tep;
	NRF2_CSN_HIGH();	
	for(i=0;i<10000;i++);

	/* 获取状态 */
	tep = SPI_NRF_ReadReg(0x07);
	printf("status: %x \r\n",tep);
	delay(25);	

	/* 解锁 */
	delay(25);
	tep = spi_wr_data(0x5C,0xA7); // 禁止发送
	delay(25);
  tep = spi_wr_data(0x2C,0x00);
	delay(25);
  tep = spi_wr_data(0x5E,0xA9);
	delay(25);
  tep = spi_wr_data(0x5A,0xA5); // 开始读逻辑
	delay(65);
	
	/* 读取数据 */
	printf("NVM_DATA:\r\n");
	for(i=0;i<64;i++)
	{
		SPI_NRF_ReadReg(0xC0+i);
		tep = SPI_NRF_ReadReg(0x15); 
		//printf("addr :%02d data:%02x \r\n",i,tep);
		switch(i)
		{
			case 0x0E:  /* 发送功率和速率 */
			{
				int8_t tx_power = 0;
				uint16_t speed = 0;
				switch(tep & 0x07)
				{
					case 0: tx_power = -12; break;
					case 1: tx_power = -6;  break;
					case 2: tx_power = -4;  break;
					case 3: tx_power = 0;   break;
					case 4: tx_power = 1;   break;
					case 5: tx_power = 3;   break;
					case 6: tx_power = 4;   break;
					case 7: tx_power = 7;   break;
					default:                break;
				}
				printf("发送功率：%d dBm\r\n",tx_power);
				switch((tep & 0x28)>>3)
				{
					case 0: speed = 1000; break;
					case 1: speed = 2000; break;
					case 4: speed = 250;  break;
					default:              break;
				}
				printf("发送速率：%d kbps \r\n",speed);
			}
			break;

			case 0x01:
				printf( "信道：%d\r\n",tep & 0x7F );
			break;

			case 0x2:
				printf( "兼容模式：%s\r\n",(tep & 0x7F)?"普通模式":"兼容模式");
			break;

			case 0x07:
				printf( "数据长度：%d\r\n",tep & 0x3F );
			break;

			case 0x00:
				printf( "CRC状态：%s\r\n",(tep & 0x40) ? "开启":"关闭");
				printf( "CRC长度：%d\r\n",(tep & 0x20) ? 2:1);
			break;

			case 0x0D:
				printf( "动态负载：%s\r\n",(tep & 0x04) ? "开启":"关闭");
				printf( "地址长度：%d\r\n",(tep & 0x03) ? (tep & 0x03)+2:0);
			break;

			case 0x08: printf( "TX_ADDR：%02X" ,tep);	break;
			case 0x09: printf( "%02X",tep);			      break;
			case 0x0A: printf( "%02X",tep);						break;
			case 0x0B: printf( "%02X",tep);						break;
			case 0x0C: printf( "%02X\r\n",tep);						break;

			case 0x14: printf( "TX_DATA：%02X" ,tep);	break;
			case 0x15:
			case 0x16:
			case 0x17:
			case 0x18:
			case 0x19:
			case 0x1A: printf( " %02X" ,tep);	break;

			default:
				break;
		}
	}
	delay(25);

	/* 获取编程次数 */
	tep = spi_wr_data(0x16,0x00);
	printf("\r\nProgram Count: %x \r\n",tep); 

	tep = spi_wr_data(0x5A,0xA5);
	delay(25);
	tep = spi_wr_data(0x2C,0x00);
	delay(25);
	tep = spi_wr_data(0x5E,0xA9);
	delay(25);
	tep = spi_wr_data(0x63,0x00);
	delay(25);
	delay(1000);
}


int rd_data_before_wr1()
{
	int tep,tep11;
	int i;
	printf("\r\n rd_nvm data ..............:  \r\n"); 
	NRF2_CSN_HIGH();	
	for(i=0;i<10000;i++);
	

	tep=SPI_NRF_ReadReg(0x07); 
	printf("\r\n status: %x \r\n",tep); 
	while(!(0x40&tep))
	{
		tep=SPI_NRF_ReadReg(0x07); 
		printf("\r\n status: %x \r\n",tep); 
	} 
	delay(10);
	tep=spi_wr_data(0x5C,0xA7);
	delay(10);
	tep=spi_wr_data(0x2C,0x00);
	delay(10);
	tep=spi_wr_data(0x5E,0xA9);
	delay(10);
	tep=spi_wr_data(0x5A,0xA5);
	delay(10);

	SPI_NRF_ReadReg(0xC0+0x00);
	tep11=SPI_NRF_ReadReg(0x15); 

	delay(10);
	tep=spi_wr_data(0x16,0x00);
	delay(10);
	tep=spi_wr_data(0x5A,0xA5);
	delay(10);
	tep=spi_wr_data(0x2C,0x00);
	delay(10);
	tep=spi_wr_data(0x5E,0xA9);
	delay(10);
	tep=spi_wr_data(0x63,0x00);
	delay(100);
	return tep11;
}

void pre_wr_data()
{
	int tep;
	tep=spi_wr_data(0x5A,0xA5);
	delay(25);
	tep=spi_wr_data(0x5b,0xA6);
	delay(25);
	tep=spi_wr_data(0x80,0xFB);
	delay(65);
	tep=spi_wr_data(0x5B,0xA6);
	delay(25);
	tep=spi_wr_data(0x5A,0xA5);
	delay(25);
	tep=spi_wr_data(0x63,0x00);
	delay(650);
}

void wr_data(char mode,char data_in)
{
	int tep;
	int i;
	
	NRF2_CSN_HIGH();		
	delay(25);

	tep=SPI_NRF_ReadReg(0x07); 
//printf("\r\n status: %x \r\n",tep); 
	while(!(0x40&tep))
	{
		 tep=SPI_NRF_ReadReg(0x07); 
		//printf("\r\n status: %x \r\n",tep); 
	}
  delay(25);
	tep=spi_wr_data(0x5C,0xA7);
	delay(25);
	tep=spi_wr_data(0x5A,0xA5);
	delay(25);
	tep=spi_wr_data(0x5B,0xA6);
	delay(25);
	 
	for(i=0;i<64;i++)
	{
		if(i>19) 
		{
			txbuf[i]  = data_in;
			txbuf1[i] = data_in;
		}	
		if(mode)
		{
		 SPI_NRF_WriteReg((0x80 + (i & 0x3F)),txbuf1[i]);
		}
		else SPI_NRF_WriteReg((0x80 + (i & 0x3F)),txbuf[i]);
		delay(65);		
	}
	tep=spi_wr_data(0x5B,0xA6);
	delay(25);
	tep=spi_wr_data(0x5A,0xA5);
	delay(25);
	tep=spi_wr_data(0x63,0x00);
	delay(25);
	printf("wr_nvm data done:  \r\n"); 
}

void si24r2e_write_nvm( void )
{
	int un_lock=0x00;
	int i;

	NRF2_CSN_HIGH();	
	for(i=0;i<10000;i++);
	
	un_lock = rd_data_before_wr1();

	if(un_lock)
	{
		printf("un_lock \r\n"); 
		pre_wr_data();
	}
	else 	
		printf("lock \r\n"); 
	wr_data(0,0x77);
}

/**************************************END OF FILE****************************/