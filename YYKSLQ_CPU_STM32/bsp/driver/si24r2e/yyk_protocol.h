/**
  ******************************************************************************
  * @file   	yyk_protocol.h
  * @author  	Samwu
  * @version 	V1.0.0.0
  * @date   	2017.05.27
  * @brief   	si24r2e Çý¶¯³ÌÐò
  ******************************************************************************
  */

#ifndef	__YYK_PROTOCOL_H_
#define	__YYK_PROTOCOL_H_

#include <stdint.h>
#include "stm32f10x_spi.h"
#include "board.h"
#include "si24r2e.h"

typedef struct
{
	uint8_t             tx_ch;
	uint16_t            tx_speed;
	int8_t              tx_power;
	uint8_t             addr_len;
	uint8_t             addr[5];
	uint8_t             data_len;
	uint8_t             data[10];
	uint16_t            send_delay;
}yyk_rf_conf_typefef; 

typedef struct
{
	char                *name;
	yyk_rf_conf_typefef conf;
	int16_t             (*update_data)( void *p_pro, uint8_t *p_uid_data );
	int16_t             (*check_rssi) ( void *p_pro, uint8_t *p_rdata );
}yyk_pro_tyedef;

#define YYK_PROTOCOL_MUM                15
#define YYK_ZKXL_PROTOCOL               0

extern yyk_pro_tyedef *yyk_pro_list[YYK_PROTOCOL_MUM];

int16_t yyk_protocol_update_rf_setting( yyk_pro_tyedef *pprotocol );
int16_t zkxl_yyk_protocol_update_uid( void *pprotocol, uint8_t *data );
int16_t zkxl_yyk_protocol_check_rssi( void *pprotocol, uint8_t *data );
#endif //__NRF_H_
