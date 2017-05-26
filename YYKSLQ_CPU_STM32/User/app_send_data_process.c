/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "cJSON.h"
#include "app_send_data_process.h"
#include "app_show_message_process.h"

/******************************************************************************
  Function:App_clickers_send_data_process
  Description:
		答题器发送处理逻辑函数
  Input :
  Return:
  Others:None
******************************************************************************/
void App_clickers_send_data_process( void )
{
	uint8_t pack_len = 0,i;
	uint8_t  spi_message[255];
	if(buffer_get_buffer_status(SPI_RBUF) == BUF_EMPTY)
		return;

	memset(spi_message,0,255);
	pack_len = spi_rd_buffer( SPI_RBUF, spi_message );
	printf("rssi = -%03d ",spi_message[0]);
	printf("data_buf: ");
	for(i=1;i<pack_len;i++)
		printf(" %02x",spi_message[i]);
	printf("\r\n");
	
}

