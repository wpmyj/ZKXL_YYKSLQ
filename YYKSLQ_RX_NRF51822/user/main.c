#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "app_error.h"
#include "nrf_delay.h"
#include "simple_uart.h"
#include "my_uart.h"
#include "my_misc_fun.h"
#include "my_radio.h"
#include "my_timer.h"
#include "ringbuf2.h"
#include "nrf_esb.h"
#include "nrf_error.h"
#include "sdk_common.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"

nrf_esb_payload_t rx_payload;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
	switch (p_event->evt_id)
	{
		case NRF_ESB_EVENT_TX_SUCCESS : break;
		case NRF_ESB_EVENT_TX_FAILED  : break;
		case NRF_ESB_EVENT_RX_RECEIVED:
			if (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
			{
				if(spi_busy_status == SPI_IDLE)
				{
					/* SPI传输层协议封装 */
					tx_data_len 			  = rx_payload.length + 6;	//SPI传输数据总长度
					m_tx_buf[0] 			  = 0x86;										//包头
					m_tx_buf[1]				  = 0x10;										//Type 
					m_tx_buf[2] 			  = 0x7F & rx_payload.rssi;	//接收到的2.4G信号强度值
					m_tx_buf[3] 			  = rx_payload.length;		
					memcpy(m_tx_buf + 4, rx_payload.data, rx_payload.length);
					m_tx_buf[tx_data_len - 2] = XOR_Cal(m_tx_buf+1, rx_payload.length + 3);	//异或校验
					m_tx_buf[tx_data_len - 1] = 0x76;						  //包尾
					
					/* 产生低电平脉冲，通知stm32中断读取SPI数据 */
					nrf_gpio_pin_clear(SPIS_IRQ_PIN);	
					nrf_delay_us(1);
					nrf_gpio_pin_set(SPIS_IRQ_PIN);
					
					//SPI传输超时定时器
					spi_busy_status = SPI_BUSY;	
					spi_overtime_timer_start();
				}
			}
			break;
	}
}

uint32_t esb_init( void )
{
	uint32_t err_code;
	uint8_t base_addr_0[4] = {0x02, 0x03, 0x04, 0x05};
	uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2};
	uint8_t addr_prefix[8] = {0x01, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };
	nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
	nrf_esb_config.payload_length           = 8;
	nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB;
	nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_1MBPS;
	nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
	nrf_esb_config.event_handler            = nrf_esb_event_handler;
	nrf_esb_config.selective_auto_ack       = false;

	err_code = nrf_esb_init(&nrf_esb_config);
	VERIFY_SUCCESS(err_code);

	err_code = nrf_esb_set_base_address_0(base_addr_0);
	VERIFY_SUCCESS(err_code);

	err_code = nrf_esb_set_base_address_1(base_addr_1);
	VERIFY_SUCCESS(err_code);

	err_code = nrf_esb_set_prefixes(addr_prefix, 1);
	VERIFY_SUCCESS(err_code);

	err_code = nrf_esb_set_rf_channel(81);
	VERIFY_SUCCESS(err_code);
			
	return err_code;
}

int main (void)
{
	clocks_start();
	debug_uart_init();
	timers_init();
	spi_gpio_init();
	my_spi_slave_init();
	
	esb_init();
	nrf_esb_start_rx();
	/* RTC校准定时器 */
	temp_timeout_start();

	while (true)
	{
		__WFE();
	}
}



