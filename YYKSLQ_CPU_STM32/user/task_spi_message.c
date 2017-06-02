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
static uint8_t rf_rev_status = 0, flash_count = 0;
static int16_t rf_rev_result = 0;
static uint8_t  spi_message[255];
	
void set_spi_rf_rev_status( uint8_t new_status )
{
	rf_rev_status = new_status;
}

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

	/* 设置接收等待超时时间 */
	if( rf_rev_status == 1 )
	{
		set_spi_rf_rev_status(2);
		sw_clear_timer(&rf_3000_timer);
	}

	if( rf_rev_status == 2 )
	{
		int8_t result = 0;
		uint8_t pack_len = 0;

		if(buffer_get_buffer_status(SPI_RBUF) == BUF_EMPTY)
			return;

		memset(spi_message,0,255);
		pack_len = spi_rd_buffer( SPI_RBUF, spi_message );

		if( pack_len > 0 )
		{
			printf("{\"fun\":\"debug\",\"rssi\":\"-%d\",\"uid\":\"%02x%02x%02x%02x%02x\"}",spi_message[0],
			spi_message[4],spi_message[5],spi_message[6],spi_message[7],spi_message[8]);
			result = yyk_pro_list[current_protocol]->check_rssi(yyk_pro_list[current_protocol],spi_message);
			if( result == 0 )
			{
				set_spi_rf_rev_status(3);
				rf_rev_result = 0;
			}
			else
			{
				rf_rev_result = -1;
			}
		}
		return;
	}

	if( rf_rev_status == 3 )
	{
		switch( rf_rev_result )
		{
			case  0: ledOn(LRED)     ;set_spi_rf_rev_status(4);break;
			case -1: ledOn(LBLUE)    ;set_spi_rf_rev_status(4);break;
			case -2: ledToggle(LBLUE);set_spi_rf_rev_status(4);break;
			default: break;
		}
		return;
	}

	if( rf_rev_status == 4 )
	{
		flash_count++;
		if( flash_count <= 10 )
		{
			set_spi_rf_rev_status(5);
		}
		else
		{
			yyk_pro_list[current_protocol]->check_rssi_print(
				yyk_pro_list[current_protocol],spi_message,rf_rev_result);
			set_spi_rf_rev_status(0);
			rf_rev_result = 0;
			flash_count   = 0;
			ledOff(LRED);
			ledOff(LBLUE);
		}
		return;
	}

	if( rf_rev_status == 6 )
		set_spi_rf_rev_status(3);
}

void spi_rev_timer_init(void)
{
	sw_create_timer(&rf_3000_timer , 3000, 2, 3,&(rf_rev_status), NULL);
	sw_create_timer(&rf_300_timer ,  100,  5, 6,&(rf_rev_status), NULL);
}
