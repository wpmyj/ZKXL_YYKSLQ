/**
  ******************************************************************************
  * @file   	si24r2e.c
  * @author  	Samwu
  * @version 	V1.0.0.0
  * @date   	2017.05.27
  * @brief   	si24r2e ��������
  ******************************************************************************
  */

#ifndef	__SI24R2E_H_
#define	__SI24R2E_H_

#include <stdint.h>
#include "stm32f10x_spi.h"
#include "board.h"

// SI24R2E NVM Register Map
#define NVM_CONFIG      0x00
#define NVM_RF_CHA      0x01
#define NVM_FIFO_LEN    0x07
#define NVM_TX_ADDR_0   0x08
#define NVM_FEATURE     0x0D
#define NVM_RF_SETUP    0x0E
#define NVM_TX_DATA_0   0x14


extern u8 txbuf[52];
extern u8 txdata[10];

void si24r2e_read_nvm( void );
void si24r2e_write_nvm( void );

#endif //__NRF_H_
