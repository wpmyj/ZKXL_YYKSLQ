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
extern uint8_t g_uid_len;
extern uint8_t show_log_flag;
static uint8_t rf_rev_status = 0, flash_count = 0;
static int16_t rf_rev_result = 0;
static uint8_t spi_message[255];
static uint8_t beep_open_flag = 0;
	
void set_spi_rf_rev_status( uint8_t new_status )
{
	rf_rev_status = new_status;
}

uint8_t get_spi_rf_rev_status( void )
{
	return rf_rev_status;
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
		char str[20];
		int8_t update_result = 0;
		update_result = yyk_pro_list[current_protocol]->update_data(
						 yyk_pro_list[current_protocol],(g_uid_len == 8)?(g_cSNR+4):g_cSNR);
		b_print("{\r\n");
		b_print("  \"fun\": \"card_setting\",\r\n");
		memset(str,0,20);
		if(g_uid_len == 8)
			sprintf(str, "%010u" , *(uint32_t *)(g_cSNR+4));
		else
			sprintf(str, "%010u" , *(uint32_t *)(g_cSNR));
		b_print("  \"card_id\": \"%s\",\r\n",str);
		b_print("  \"pro_name\": \"%s\",\r\n",yyk_pro_list[current_protocol]->name);
		b_print("  \"result\": \"%d\"\r\n",update_result);
		b_print("}\r\n");
		if( update_result == 0 )
		{
			clear_buffer(SPI_RBUF);
			set_spi_rf_rev_status(2);
			BEEP_EN() ;
			beep_open_flag = 1;
			sw_clear_timer(&rf_3000_timer);
		}
		else
		{
			rf_rev_result = -1;
			set_spi_rf_rev_status(3);
			BEEP_EN() ;
			beep_open_flag = 2;
		}
	}

	if( rf_rev_status == 2 )
	{
		int8_t result = 0;
		if( buffer_get_buffer_status(SPI_RBUF) != BUF_EMPTY )
		{
			memset(spi_message,0,255);
			if( spi_rd_buffer( SPI_RBUF, spi_message ) > 0 )
			{
				if(show_log_flag >= 1)
					b_print("{\"fun\":\"debug\",\"rssi\":\"-%d\",\"uid\":\"%02x%02x%02x%02x%02x\"}\r\n",
				spi_message[0],spi_message[4],spi_message[5],spi_message[6],spi_message[7],spi_message[8]);
			}
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
	}

	if( rf_rev_status == 3 )
	{
		switch( rf_rev_result )
		{
			case  0: ledOn(LRED);      set_spi_rf_rev_status(4);break;
			case -1: ledOn(LBLUE);     set_spi_rf_rev_status(4);break;
			case -2: ledToggle(LBLUE); set_spi_rf_rev_status(4);break;
			default: break;
		}
		return;
	}

	if( rf_rev_status == 4 )
	{ 
		flash_count++;
		if( flash_count <= 2 )
		{
			set_spi_rf_rev_status(5);
		}
		else
		{
			uint8_t power_status = get_power_status();
			if(rf_rev_result != -1)
				yyk_pro_list[current_protocol]->check_rssi_print(
					yyk_pro_list[current_protocol],spi_message,rf_rev_result);
			set_spi_rf_rev_status(0);
			rf_rev_result = 0;
			flash_count   = 0;
			ledOff(LRED);
			ledOff(LBLUE);
			if(power_status == 0)
			{
				set_spi_rf_rev_status(0);
			}
			rf_set_card_status(1);
		}
		return;
	}

	if( rf_rev_status == 6 )
		set_spi_rf_rev_status(3);
}

void beep_callback(void)
{
	BEEP_DISEN();
}

void spi_rev_timer_init(void)
{
	sw_create_timer(&rf_3000_timer , 3000, 2, 3,&(rf_rev_status), NULL);
	sw_create_timer(&rf_300_timer ,  100,  5, 6,&(rf_rev_status), NULL);
	sw_create_timer(&beep_ok_timer , 100,  1, 0,&(beep_open_flag),beep_callback);
	sw_create_timer(&beep_fail_timer,600,  2, 0,&(beep_open_flag),beep_callback);
}
