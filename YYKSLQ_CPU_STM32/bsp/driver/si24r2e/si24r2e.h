/**
  ******************************************************************************
  * @file   	si24r2e.c
  * @author  	Samwu
  * @version 	V1.0.0.0
  * @date   	2017.05.27
  * @brief   	si24r2e Çý¶¯³ÌÐò
  ******************************************************************************
  */

#ifndef	__SI24R2E_H_
#define	__SI24R2E_H_

#include <stdint.h>
#include "stm32f10x_spi.h"
#include "board.h"

//#define ENABLE_SI24R2E_DEBUG_LOG

#ifdef 	ENABLE_SI24R2E_DEBUG_LOG
#define DEBUG_SI24R2E_LOG							     DEBUG_LOG
#else
#define DEBUG_SI24R2E_LOG(...)
#endif

// SI24R2E NVM Register Map
#define NVM_CONFIG      0x00
#define NVM_RF_CHA      0x01
#define NVM_RF_TH       0x04
#define NVM_RF_TL       0x05
#define NVM_FIFO_LEN    0x07
#define NVM_TX_ADDR_0   0x08
#define NVM_FEATURE     0x0D
#define NVM_RF_SETUP    0x0E
#define NVM_REP_PSE     0x0F
#define NVM_TEST        0x10
#define NVM_LV_REP      0x11
#define NVM_TRIM_CLOCK  0x13
#define NVM_TX_DATA_0   0x14


extern u8 txbuf[52];
extern u8 txdata[10];

uint8_t si24r2e_read_nvm( uint8_t *pbuf );
void si24r2e_write_nvm( uint8_t *pwbuf );
void sync_power_check(void);
uint8_t get_power_status(void);
#endif //__NRF_H_
