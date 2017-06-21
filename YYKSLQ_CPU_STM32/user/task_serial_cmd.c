/**
  ******************************************************************************
  * @file   	task_spi_message.c
  * @author  	sam.wu
  * @version 	V1.0.0.0
  * @date   	2015.11.05
  * @brief
  * @Changelog :
  *    [1].Date   : 2016_8-26
	*        Author : sam.wu
	*        brief  : 尝试分离数据，降低代码的耦合度
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"
#include "cJSON.h"
#include "task_spi_message.h"
#include "task_find_card.h"
#include "task_show_message.h"
#include "yyk_protocol.h"

/* Private variables ---------------------------------------------------------*/
extern nrf_communication_t nrf_data;
extern uint16_t list_tcb_table[UID_LIST_TABLE_SUM][WHITE_TABLE_LEN];
extern uint8_t show_log_flag;

/* 暂存题目信息，以备重发使用 */
uint8_t json_read_index  = 0;
uint8_t current_protocol = 0;
uint8_t show_log_flag    = 0;
uint8_t si24r2e_auto_burn_flag = 0;

extern wl_typedef       wl;
extern revicer_typedef  revicer;
extern task_tcb_typedef card_task;
extern u8 txbuf[52];
/* Private functions ---------------------------------------------------------*/

const static serial_cmd_typedef cmd_list[] = {
{"bind_start",         sizeof("bind_start"),              serial_cmd_find_card},
{"bind_stop",          sizeof("bind_stop"),               serial_cmd_find_card},
{"get_device_info",    sizeof("get_device_info"),         serial_cmd_get_device_info},
{"si24r2e_auto_burn",  sizeof("si24r2e_auto_burn"),       serial_cmd_si24r2e_rd_wr_nvm},
{"si24r2e_show_log",   sizeof("si24r2e_show_log"),        serial_cmd_si24r2e_show_log},
{"NO_USE",             sizeof("NO_USE"),                  NULL                     }
};

static void serial_cmd_process(void);
void exchange_json_format( char *out, char old_format, char new_format);

/******************************************************************************
  Function:App_seirial_cmd_process
  Description:
		串口进程处理函数
  Input :None
  Return:None
  Others:None
******************************************************************************/
void serial_cmd_process(void)
{
	if( rjson_count > 0 )
	{
		char header[30];
		char *pdata = (char *)uart_irq_revice_massage[json_read_index];
		/* 增加对'的支持 */
		exchange_json_format( pdata, '\'', '\"' );
		memcpy(header,pdata+8,14);
		
		{
			cJSON *json;
			json = cJSON_Parse((char *)uart_irq_revice_massage[json_read_index]);
			if (!json)
			{
				  b_print("{\r\n");
	        b_print("  \"fun\": \"Error\",\r\n");
					b_print("  \"description\": \"json syntax error!\"\r\n");
					b_print("}\r\n");
			}
			else
			{
				uint8_t i = 0, is_know_cmd = 0;
				char *p_cmd_str = cJSON_GetObjectItem(json, "fun")->valuestring;

				while(cmd_list[i].cmd_fun != NULL)
				{
					if(strncmp(p_cmd_str, cmd_list[i].cmd_str,
						 cmd_list[i].cmd_len)== 0)
					{
						cmd_list[i].cmd_fun(json);
						is_know_cmd = 1;
					}
					i++;
				}

				if(is_know_cmd == 0)
				{
					b_print("{\r\n");
	        b_print("  \"fun\": \"Error\",\r\n");
					b_print("  \"description\": \"unknow cmd!\"\r\n");
					b_print("}\r\n");
				}
			}
			cJSON_Delete(json);
		}

		rjson_count--;
		memset( pdata, 0, JSON_BUFFER_LEN );
		json_read_index = (json_read_index + 1) % JSON_ITEM_MAX;
	}
}

void parse_str_to_time( char *str )
{
	char str1[10];
	/*system_rtc_timer:year*/
	memset(str1,0,10);
	memcpy(str1,str,4);
	system_rtc_timer.year = atoi( str1 );
	/*system_rtc_timer:mon*/
	memset(str1,0,10);
	memcpy(str1,str+5,2);
	system_rtc_timer.mon = atoi( str1 );
	/*system_rtc_timer:date*/
	memset(str1,0,10);
	memcpy(str1,str+8,2);
	system_rtc_timer.date = atoi( str1 );
	/*system_rtc_timer:hour*/
	memset(str1,0,10);
	memcpy(str1,str+11,2);
	system_rtc_timer.hour = atoi( str1 );
	/*system_rtc_timer:min*/
	memset(str1,0,10);
	memcpy(str1,str+14,2);
	system_rtc_timer.min = atoi( str1 );
	/*system_rtc_timer:sec*/
	memset(str1,0,10);
	memcpy(str1,str+17,2);
	system_rtc_timer.sec = atoi( str1 );
//printf("Parse:num  = %d type = %d \r\n",num,type);
//printf("Parse:year = %d \r\n",system_rtc_timer.year);
//printf("Parse:mon  = %d \r\n",system_rtc_timer.mon);
//printf("Parse:mon  = %d \r\n",system_rtc_timer.date);	
//printf("Parse:hour = %d \r\n",system_rtc_timer.hour);
//printf("Parse:min  = %d \r\n",system_rtc_timer.min);	
//printf("Parse:sec  = %d \r\n",system_rtc_timer.sec);
}

void exchange_json_format( char *out, char old_format, char new_format)
{
	char *pdata = out;

	while(*pdata != '\0')
	{
		if(*pdata == old_format)
		{
			*pdata = new_format;
		}
		pdata++;
	}
}
/******************************************************************************
  Function:App_seirial_cmd_process
  Description:
		串口进程处理函数
  Input :None
  Return:None
  Others:None
******************************************************************************/
void App_seirial_cmd_process(void)
{
	/* serial cmd process */
	serial_cmd_process();
}

void serial_cmd_find_card(const cJSON *object)
{
	uint8_t card_status = rf_get_card_status();
	char    *p_cmd_str  = cJSON_GetObjectItem(object, "fun")->valuestring;

	b_print("{\r\n");

	if(strncmp(p_cmd_str, "bind_start", sizeof("bind_start"))== 0 )
	{
		b_print("  \"fun\": \"bind_start\",\r\n");

	}

	if(strncmp(p_cmd_str, "bind_stop", sizeof("bind_stop"))== 0 )
	{
		b_print("  \"fun\": \"bind_stop\",\r\n");
	}

	b_print("  \"result\": \"0\"\r\n");
	b_print("}\r\n");
}

void serial_cmd_si24r2e_show_log(const cJSON *object)
{
	uint8_t card_status = rf_get_card_status();
	char    *p_cmd_str  = cJSON_GetObjectItem(object, "setting")->valuestring;
	
	show_log_flag = atoi(p_cmd_str);
	
	b_print("{\r\n");
	b_print("  \"fun\": \"si24r2e_show_log\",\r\n");
	b_print("  \"result\": \"0\"\r\n");
	b_print("}\r\n");
}

void serial_cmd_get_device_info(const cJSON *object)
{
	b_print("{\r\n");
	b_print("  \"fun\": \"get_device_info\",\r\n");
	b_print("  \"device_id\": \"%010u\",\r\n",*(uint32_t *)(revicer.uid));
	b_print("  \"software_version\": \"v0.1.0\",\r\n");
	b_print("  \"hardware_version\": \"ZL-RP551-MAIN-F\",\r\n");
	b_print("  \"company\": \"zkxltech\"\r\n");
	b_print(" }\r\n");
}

void serial_cmd_si24r2e_rd_wr_nvm(const cJSON *object)
{
	int8_t result = 0;
	uint8_t pro_index = 0;
	char    *p_cmd_str  = cJSON_GetObjectItem(object, "setting")->valuestring;
	uint8_t rd_wr = atoi(p_cmd_str);
	
	if(rd_wr == 0)
	{
		wl.match_status = OFF;
		BEEP_DISEN();
		ledOff(LBLUE);
		ledOff(LRED);
		rf_set_card_status(0);
		PcdHalt();
		PcdAntennaOff();
		si24r2e_auto_burn_flag = 0;
	}
	else
	{
		char *p_time_str; 
		p_cmd_str = cJSON_GetObjectItem(object, "pro_index")->valuestring;
		pro_index = atoi(p_cmd_str);
		if( pro_index <= 13 )
		{ 
			yyk_protocol_update_rf_setting(yyk_pro_list[pro_index]);
			p_time_str = cJSON_GetObjectItem(object, "time")->valuestring;
			parse_str_to_time(p_time_str);
			wl.match_status = ON;
			rf_set_card_status(1);
			result = 0;
		}
		else
		{
			yyk_protocol_update_rf_setting(yyk_pro_list[YYK_ZKXL_PROTOCOL]);
			pro_index = 0;
			result = -1;
		}
		si24r2e_auto_burn_flag = 1;
		current_protocol = pro_index;
	}

	b_print("{\r\n");
	b_print("  \"fun\": \"si24r2e_auto_burn\",\r\n");
	b_print("  \"pro_name\": \"%s\",\r\n",yyk_pro_list[pro_index]->name);
	b_print("  \"setting\": \"%s\",\r\n",rd_wr?"start":"stop");
	b_print("  \"debug\": \"%d\",\r\n",show_log_flag);
	b_print("  \"result\": \"%d\"\r\n",result);
	b_print("}\r\n");
}
/**************************************END OF FILE****************************/
