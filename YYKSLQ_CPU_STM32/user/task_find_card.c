/**
  ******************************************************************************
  * @file   	app_card_process.c
  * @author  	sam.wu
  * @version 	V1.0.0.0
  * @date   	2015.11.05
  * @brief
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "task_serial_cmd.h"
#include "task_find_card.h"
#include "task_show_message.h"
#include "task_spi_message.h"
#include "yyk_protocol.h"

//#define SHOW_CARD_PROCESS_TIME
extern uint8_t g_cSNR[10];	
extern wl_typedef        wl;
extern revicer_typedef   revicer;
extern rf_config_typedef clicker_set;
extern uint8_t           current_protocol;
task_tcb_typedef card_task;

#ifdef SHOW_CARD_PROCESS_TIME
extern __IO uint32_t     PowerOnTime;
uint32_t StartTime,EndTime;
#endif

static uint8_t card_process_status = 0;
/* 返回卡类型 */
uint8_t g_cardType[40];	
uint8_t respon[BUF_LEN + 20];	
static rf_id_typedf rID,wID;

uint16_t NDEF_Len               = 0;
uint8_t g_uid_len               = 0;
static uint8_t wtrte_flash_ok   = 0;
static uint8_t card_message_err = 0;
static uint8_t find_card_ok     = 0;

/******************************************************************************
  Function:rf_set_card_status
  Description:
		修改systick的状态
  Input :
		rf_status: systick的新状态
  Output:
  Return:
  Others:None
******************************************************************************/
void rf_set_card_status(uint8_t new_status)
{
	card_process_status = new_status;
	//printf("rf_set_card_status  = %d\r\n",card_process_status);
}

/******************************************************************************
  Function:rf_get_card_status
  Description:
		获取systick的状态
  Input :
  Output:systick的新状态
  Return:
  Others:None
******************************************************************************/
uint8_t rf_get_card_status(void)
{
	return card_process_status ;
}

/******************************************************************************
  Function:App_card_process
  Description:
		App MI Card 轮询处理函数
  Input :
  Return:
  Others:None
******************************************************************************/
void App_card_process(void)
{
	/* 获取当前状态 */
	uint8_t card_current_status = 0;
	uint8_t spi_current_status  = 0;
	
	card_current_status = rf_get_card_status();

	spi_current_status  = get_spi_rf_rev_status();

	if(spi_current_status != 0)
		return;
	
	if( card_current_status == 1 )
	{
		uint8_t status = 0;
		#ifdef SHOW_CARD_PROCESS_TIME
		StartTime = PowerOnTime;
		#endif

		PcdAntennaOn();
		memset(g_cardType, 0, 40);
		/* reqA指令 :请求A卡，返回卡类型，不同类型卡对应不同的UID长度 */
		status = PcdRequest(PICC_REQIDL,g_cardType);
		if( status == MI_OK )
		{
			if(find_card_ok == 1)
			{
				sw_clear_timer(&card_second_find_timer);
				return;
			}
			if(find_card_ok == 2)
			{
				find_card_ok = 0;
			}
		
			if( (g_cardType[0] & 0x40) == 0x40)
			{	
				g_uid_len = 8;	
			}
			else
			{	
				g_uid_len = 4;
			}
		}
		else
		{
			return;
		}
		/* 防碰撞1 */
		status = PcdAnticoll(PICC_ANTICOLL1, g_cSNR);
		if( status != MI_OK )
		{
			return;
		}

		/* 选卡1 */
		memset(respon, 0, 10);
		status = PcdSelect1(g_cSNR, respon);
		if( status == MI_OK )
		{
			if((g_uid_len == 8) && ((respon[0] & 0x04) == 0x04))
			{
				DEBUG_CARD_DEBUG_LOG("PcdSelect1 status = %d\r\n",status);
				//MRC500_DEBUG_START("PICC_ANTICOLL2 \r\n");
				status = PcdAnticoll(PICC_ANTICOLL2, &g_cSNR[4]);
				//MRC500_DEBUG_END();
				if( status != MI_OK)
				{
					return;
				}
				status = PcdSelect2(&g_cSNR[4], respon);
				if( status == MI_OK)
				{
					card_message_err = 1;
					wtrte_flash_ok = 1;
					rf_set_card_status(3);
					DEBUG_CARD_DEBUG_LOG("PcdSelect2 status = %d\r\n",status);
				}
				else
				{
					return;
				}
			}
			else
			{
				card_message_err = 1;
				wtrte_flash_ok = 1;
				rf_set_card_status(3);
			}
		}
		else
		{
			return;
		}
		#ifdef SHOW_CARD_PROCESS_TIME
		EndTime = PowerOnTime - StartTime;
		printf("UseTime:PcdSelect2 = %d \r\n",EndTime);
		#endif
	}

	if( card_current_status == 3 )
	{
		if(wtrte_flash_ok == 1)
		{
			if( card_message_err != 2 )
			{
				#ifdef OPEN_SILENT_MODE
				ledOn(LBLUE);
				#else
				BEEP_EN();
				#endif
				Deselect();
				PcdHalt();
				PcdAntennaOff();
			}
		}

		if(card_message_err != 0)
		{
			if(( wtrte_flash_ok == 1 ) && (wl.weite_std_id_status == OFF))
			{
				#ifndef OPEN_CARD_DATA_SHOW 
				
				DEBUG_CARD_DATA_LOG("NDEF_DataRead and NDEF_DataWrite Clear!\r\n");
				#endif
			}
		}
		rf_set_card_status(4);
	}

	if( card_current_status == 5 )
	{

		uint8_t *rpdata = (uint8_t *)&rID;
		uint8_t *wpdata = (uint8_t *)&wID;
		#ifdef OPEN_SILENT_MODE
		ledOff(LBLUE);
		#else
		BEEP_DISEN();
		set_spi_rf_rev_status(1);
		#endif
		rf_set_card_status(1);
		memset(rpdata,0x00,sizeof(rf_id_typedf));
		memset(wpdata,0x00,sizeof(rf_id_typedf));
		if( wl.weite_std_id_status == ON )
		{
			memset(wID.stdid,0x00,10);
			wl.weite_std_id_status = OFF;
			wl.match_status = OFF;
			rf_set_card_status(0);
		}
		find_card_ok = 1;
		#ifdef SHOW_CARD_PROCESS_TIME
		EndTime = PowerOnTime - StartTime;
		printf("UseTime:SecondFindStart = %d \r\n",EndTime);
		#endif
	}
}

/******************************************************************************
  Function:systick_timer_init
  Description:
  Input :
  Return:
  Others:None
******************************************************************************/
void card_timer_init( void )
{
	sw_create_timer(&card_buzzer_timer    , 150, 4, 5,&(card_process_status), NULL);
	sw_create_timer(&card_second_find_timer,200, 1, 2,&(find_card_ok), NULL);
}
