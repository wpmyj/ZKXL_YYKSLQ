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

/* These are set to zero as Shockburst packets don't have corresponding fields. */
#define PACKET_S1_FIELD_SIZE        (1UL)  /**< Packet S1 field size in bits. */
#define PACKET_S0_FIELD_SIZE        (1UL)  /**< Packet S0 field size in bits. */
#define PACKET_LENGTH_FIELD_SIZE    (0UL)  /**< Packet length field size in bits. */

#define PACKET_BASE_ADDRESS_LENGTH  (4UL)  //!< Packet base address length field size in bytes
#define PACKET_STATIC_LENGTH        (8UL)  //!< Packet static length in bytes
#define PACKET_PAYLOAD_MAXSIZE      (32UL) //!< Packet payload maximum size in bytes

//static uint32_t                      packet[PACKET_PAYLOAD_MAXSIZE/4];              /**< Packet to transmit. */
typedef struct
{
	uint8_t data[32];
	uint8_t rssi;
	uint8_t pid;
	uint8_t len;
}m_rf_data_typedef;

m_rf_data_typedef packet;

static uint32_t swap_bits(uint32_t inp)
{
	uint32_t i;
	uint32_t retval = 0;
	inp = (inp & 0x000000FFUL);
	for (i = 0; i < 8; i++)
	{
		retval |= ((inp >> i) & 0x01) << (7 - i);     
	}
	return retval;    
}

static uint32_t bytewise_bitswap(uint32_t inp)
{
	return (swap_bits(inp >> 24) << 24)
			 | (swap_bits(inp >> 16) << 16)
			 | (swap_bits(inp >> 8) << 8)
			 | (swap_bits(inp));
}

void radio_configure()
{
	// Radio config
	NRF_RADIO->TXPOWER   = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);
	NRF_RADIO->FREQUENCY = 0x51;  // Frequency bin 0, 2400 MHz
	NRF_RADIO->MODE      = (RADIO_MODE_MODE_Nrf_1Mbit << RADIO_MODE_MODE_Pos);

	// Radio address config
	NRF_RADIO->PREFIX0 = 
			((uint32_t)swap_bits(0xC5) << 24) // Prefix byte of address 3 converted to nRF24L series format
		| ((uint32_t)swap_bits(0xC4) << 16) // Prefix byte of address 2 converted to nRF24L series format
		| ((uint32_t)swap_bits(0xC3) << 8)  // Prefix byte of address 1 converted to nRF24L series format
		| ((uint32_t)swap_bits(0x01) << 0); // Prefix byte of address 0 converted to nRF24L series format

	NRF_RADIO->PREFIX1 = 
			((uint32_t)swap_bits(0xC8) << 24) // Prefix byte of address 7 converted to nRF24L series format
		| ((uint32_t)swap_bits(0xC7) << 16) // Prefix byte of address 6 converted to nRF24L series format
		| ((uint32_t)swap_bits(0xC6) << 0); // Prefix byte of address 4 converted to nRF24L series format

	NRF_RADIO->BASE0 = bytewise_bitswap(0x02030405UL);  // Base address for prefix 0 converted to nRF24L series format
	NRF_RADIO->BASE1 = bytewise_bitswap(0xC2C2C2C2UL);  // Base address for prefix 1-7 converted to nRF24L series format

  NRF_RADIO->TXADDRESS   = 0x00UL;  // Set device address 0 to use when transmitting
	NRF_RADIO->RXADDRESSES = 0x1UL;  // Enable device address 0 to use to select which addresses to receive

	// Packet configuration
	NRF_RADIO->PCNF0 = (PACKET_S1_FIELD_SIZE     << RADIO_PCNF0_S1LEN_Pos) |
										 (PACKET_S0_FIELD_SIZE     << RADIO_PCNF0_S0LEN_Pos) |
										 (PACKET_LENGTH_FIELD_SIZE << RADIO_PCNF0_LFLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

	// Packet configuration
	NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
										 (RADIO_PCNF1_ENDIAN_Big       << RADIO_PCNF1_ENDIAN_Pos)  |
										 (PACKET_BASE_ADDRESS_LENGTH   << RADIO_PCNF1_BALEN_Pos)   |
										 (PACKET_STATIC_LENGTH         << RADIO_PCNF1_STATLEN_Pos) |
										 (PACKET_PAYLOAD_MAXSIZE       << RADIO_PCNF1_MAXLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

	// CRC Config
	NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // Number of checksum bits
	if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos))
	{
		NRF_RADIO->CRCINIT = 0xFFFFUL;   // Initial value      
		NRF_RADIO->CRCPOLY = 0x11021UL;  // CRC poly: x^16+x^12^x^5+1
	}
	else if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_One << RADIO_CRCCNF_LEN_Pos))
	{
		NRF_RADIO->CRCINIT = 0xFFUL;   // Initial value
		NRF_RADIO->CRCPOLY = 0x107UL;  // CRC poly: x^8+x^2^x^1+1
	}
	NRF_RADIO->TASKS_RSSISTART = 1U;       // 开始检测RSSI
}

uint32_t read_packet()
{
	uint32_t len = 0;

  /* 开始设置接收 */ 
	NRF_RADIO->EVENTS_READY = 0U;
	NRF_RADIO->TASKS_RXEN   = 1U;          // 启动接收
	while(NRF_RADIO->EVENTS_READY == 0U);  // 等待收发模式转换完成
	NRF_RADIO->EVENTS_END  = 0U;           // 传输完成
	NRF_RADIO->TASKS_START = 1U;           // 开始传输

  /* 等待接收完成 */ 
	while (NRF_RADIO->EVENTS_END == 0U);

  /* 判断CRC检验，提取接收数据 */ 
	if (NRF_RADIO->CRCSTATUS == 1U)
	{
		len = packet.data[0];
		packet.rssi = NRF_RADIO->RSSISAMPLE;
		packet.len  = (packet.data[0] & 0xFC) >> 2;
		packet.pid  = (packet.data[0] & 0x03);
	}

  /* 关闭接收 */
	NRF_RADIO->EVENTS_DISABLED = 0U;
	NRF_RADIO->TASKS_DISABLE = 1U;	         
	while (NRF_RADIO->EVENTS_DISABLED == 0U);

	return len;
}

int main (void)
{
	clocks_start();
	debug_uart_init();
	timers_init();
	spi_gpio_init();
	my_spi_slave_init();

	NRF_RADIO->PACKETPTR = (uint32_t)&packet;
	radio_configure();

	/* RTC校准定时器 */
	temp_timeout_start();

	while (true)
	{
		if(spi_busy_status == SPI_IDLE)
		{
			if( read_packet() > 0 )
			{
				/* SPI传输层协议封装 */
				tx_data_len 			  = packet.len + 6;						//SPI传输数据总长度
				m_tx_buf[0] 			  = 0x86;										//包头
				m_tx_buf[1]				  = 0x10;										//Type 
				m_tx_buf[2] 			  = 0x7F & packet.rssi;						//接收到的2.4G信号强度值
				m_tx_buf[3] 			  = packet.len;		
				memcpy(m_tx_buf + 4, packet.data+2, packet.len);
				m_tx_buf[tx_data_len - 2] = XOR_Cal(m_tx_buf+1, packet.len + 3);	//异或校验
				m_tx_buf[tx_data_len - 1] = 0x76;										//包尾
				
				/* 产生低电平脉冲，通知stm32中断读取SPI数据 */
				nrf_gpio_pin_clear(SPIS_IRQ_PIN);	
				nrf_delay_us(1);
				nrf_gpio_pin_set(SPIS_IRQ_PIN);
				
				//SPI传输超时定时器
				spi_busy_status = SPI_BUSY;	
				spi_overtime_timer_start();
			}
		}
	}
}



