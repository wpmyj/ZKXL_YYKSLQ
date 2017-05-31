/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "cJSON.h"
#include "task_spi_message.h"
#include "task_find_card.h"
#include "yyk_protocol.h"

extern uint8_t current_protocol;
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
	int8_t result = 0;
	uint8_t pack_len = 0;
	uint8_t  spi_message[255];
	if(buffer_get_buffer_status(SPI_RBUF) == BUF_EMPTY)
		return;

	memset(spi_message,0,255);
	pack_len = spi_rd_buffer( SPI_RBUF, spi_message );

  if(pack_len > 0)
  {
		result = yyk_pro_list[current_protocol]->check_rssi(yyk_pro_list[current_protocol],spi_message);
		if( result == 0 )
			ledOn(LRED);
		else
			ledOff(LRED);
	}
}

