/**
  ******************************************************************************
  * @file   	ringbuffer.c
  * @author  	sam.wu
  * @version 	V1.0.0.0
  * @date   	2016.8.26
  * @brief   	ringbuffer
  ******************************************************************************
  */
#include "ringbuffer.h"
#include "string.h"

/* Private variables ---------------------------------------------------------*/
static uint8_t  spi_rbuf[SPI_RBUF_SIZE];
static uint8_t  ptint_buf[PRINT_RBUF_SIZE];
const uint32_t  buf_size[BUF_NUM]        = { PRINT_RBUF_SIZE, SPI_RBUF_SIZE };
static uint8_t  *pbuf[BUF_NUM]           = { ptint_buf, spi_rbuf };
static volatile uint16_t top[BUF_NUM]    = { 0, 0 };
static volatile uint16_t bottom[BUF_NUM] = { 0, 0 };
static volatile int32_t  Size[BUF_NUM]   = { 0, 0 };
static volatile uint8_t  status[BUF_NUM] = { BUF_EMPTY, BUF_EMPTY };

/* Private functions ---------------------------------------------------------*/
static void    update_read_status( uint8_t sel) ;
static void    update_write_status( uint8_t sel) ;
static void    update_top( uint8_t sel, uint16_t Len );
static void    update_bottom( uint8_t sel, uint16_t Len );
static uint8_t get(uint8_t sel, uint16_t index);
static void    set(uint8_t sel, uint16_t index,uint8_t data);

/******************************************************************************
  Function:buffer_get_buffer_status
  Description:
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
uint8_t buffer_get_buffer_status( uint8_t sel )
{
	return status[sel];
}

/******************************************************************************
  Function:get
  Description:
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
uint8_t get( uint8_t sel, uint16_t index )
{
	uint8_t data = pbuf[sel][index % buf_size[sel]];
	pbuf[sel][index % buf_size[sel]] = 0;
	return  data;
}

/******************************************************************************
  Function:set
  Description:
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
void set( uint8_t sel, uint16_t index, uint8_t data)
{
	pbuf[sel][index % buf_size[sel]] = data;
}

/******************************************************************************
  Function:update_read_status
  Description:
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
static void update_read_status( uint8_t sel)
{
	uint8_t bufferstatus = buffer_get_buffer_status(sel);

	switch(bufferstatus)
	{
		case BUF_EMPTY:
			break;
		case BUF_USEING:
			{
				if(Size[sel] > 0)
					status[sel] = BUF_USEING;
				else if(Size[sel] == 0)
					status[sel] = BUF_EMPTY;
				else
				{
					status[sel] = BUF_EMPTY;
					top[sel]    = 0;
					bottom[sel] = 0;
				}
			}
			break;
		case BUF_FULL:
			status[sel] = BUF_USEING;
			break;

		default:
			break;
	}
}

/******************************************************************************
  Function:update_write_status
  Description:
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
static void update_write_status( uint8_t sel)
{
	uint8_t bufferstatus = buffer_get_buffer_status(sel);

	switch(bufferstatus)
	{
		case BUF_EMPTY:
			status[sel] = BUF_USEING;
			break;

		case BUF_USEING:
			{
				if( Size[sel] > buf_size[sel]-PACKETSIZE )
					status[sel] = BUF_FULL;
				else
					status[sel] = BUF_USEING;
			}
			break;

		default:
			break;
	}
}
/******************************************************************************
  Function:update_top
  Description:
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
static void update_top( uint8_t sel, uint16_t Len )
{
	CLOSEIRQ();
	Size[sel] += Len;
	top[sel] = (top[sel] + Len) % buf_size[sel];
	OPENIRQ();
}

/******************************************************************************
  Function:update_bottom
  Description:
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
static void update_bottom( uint8_t sel, uint16_t Len )
{
	CLOSEIRQ();
	Size[sel] -= Len;
	bottom[sel] = (bottom[sel] + Len) % buf_size[sel];
	OPENIRQ();
}

/******************************************************************************
  Function:App_rf_check_process
  Description:
		spi RF 消息缓存处理函数
  Input :
  Return:
  Others:None
******************************************************************************/
void spi_wr_buffer( uint8_t sel, uint8_t *pbuf, uint16_t len )
{
	uint8_t  *pdata = pbuf;
	uint16_t i;
	uint8_t  pack_len = len % 256;

  set(sel,top[sel]+0,0x5C);
	set(sel,top[sel]+1,pack_len);
	for(i=1;i<len+1;i++)
	{
		set(sel,top[sel]+1+i,*pdata);
		//printf(" %02x",*pdata);
		pdata++;
	}
	set(sel,top[sel]+len+2,0xCA);

	update_top( sel, len+3);
	update_write_status(sel);
}

/******************************************************************************
  Function:App_rf_check_process
  Description:
		spi RF 消息读取处理函数
  Input :
  Return:
  Others:None
******************************************************************************/
uint16_t spi_rd_buffer( uint8_t sel, uint8_t *rbuf)
{
	uint16_t i;
	uint8_t skip = 0;
	uint8_t header = 0;
	uint16_t DataLen = 0;
	uint8_t *pdata;

	pdata = rbuf;
	
	while( header != 0x5C )
	{
		header = get(sel,bottom[sel]+skip);
		skip++;
	}

	DataLen = get(sel,bottom[sel]+skip);
	for(i=0;i<DataLen;i++)
	{
		*pdata = get(sel,bottom[sel]+i+skip+1);
		//printf(" %02x",*pdata);
		pdata++;
	}

	update_bottom(sel, DataLen + skip + 2);
	update_read_status(sel);

	return DataLen;
}

void clear_buffer( uint8_t sel )
{
	bottom[sel] = 0;
	top[sel]    = 0;
	Size[sel]   = 0;
	status[sel] = BUF_EMPTY;
}

/******************************************************************************
  Function:ringbuffer_get_usage_rate
  Description:
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
uint8_t ringbuffer_get_usage_rate(uint8_t sel)
{
	return (Size[sel]*100/buf_size[sel]);
}

/******************************************************************************
  Function:print_write_data_to_buffer
  Description:
		存储打印数据到缓存
  Input :
  Return:
  Others:None
******************************************************************************/
void print_write_data_to_buffer( char *str, uint8_t len )
{
	char *pdata;
  uint8_t	i;

	pdata = str;

	for(i=0;i<len;i++)
	{
		set(PRINT_BUF,top[PRINT_BUF]+i,*pdata);
		pdata++;
	}

	update_top( PRINT_BUF, len);
	update_write_status(PRINT_BUF);
}

/******************************************************************************
  Function:print_write_data_to_buffer
  Description:
		从缓存中读取打印数据
  Input :
  Return:
  Others:None
******************************************************************************/
void print_read_data_to_buffer( uint8_t *str ,uint8_t size)
{
	uint8_t Len, *pdata, i;

	Len = Size[PRINT_BUF] > size ? size : Size[PRINT_BUF];
	pdata = str;

	for(i=0;i<Len;i++)
	{
		*pdata = get(PRINT_BUF,bottom[PRINT_BUF]+i);
		pdata++;
	}

	update_bottom(PRINT_BUF, Len);
	update_read_status(PRINT_BUF);
}

/**************************************END OF FILE****************************/
