#include "my_radio.h"

//#define RADIO_DEBUG

#ifdef RADIO_DEBUG
#define radio_debug  printf   
#else  
#define radio_debug(...)                    
#endif 

nrf_esb_payload_t        tx_payload;
nrf_esb_payload_t        rx_payload;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
//            radio_debug("TX SUCCESS EVENT\r\n");
            break;
        case NRF_ESB_EVENT_TX_FAILED:
//            radio_debug("TX FAILED EVENT\r\n");
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
//            radio_debug("RX RECEIVED EVENT\r\n");
//            if(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
//            {
//				nrf_esb_flush_rx();
				
//				radio_debug("nrf:%02x %02x \r\n",rx_payload.data[5],rx_payload.data[6]);
//				radio_debug("nrf:");
//				for(uint8_t i = 0;i < 9;i++)
//				{
//					radio_debug("%02X ",rx_payload.data[i]);
//				}radio_debug("end \r\n");
				
				/* 包头、包尾、异或校验正确则存在buff里 等待处理 */
//				if(rx_payload.data[0]				    == NRF_DATA_HEAD &&
//				   rx_payload.data[rx_payload.length-1] == NRF_DATA_END  &&
//				   rx_payload.data[rx_payload.length-2] == XOR_Cal(rx_payload.data+1,rx_payload.length-3))
//				{
//					if(get_ringbuf_status() != BUFF_FULL)
//					{
//						ringbuf_write_data(rx_payload.data,rx_payload.length);
//					}
//					else
//					{
////						radio_debug("RING_BUF_FULL \r\n");
//					}
//				}
//				else
//				{
////					radio_debug("2.4G rx data check err  \r\n");
//				}

//			}
            break;
    }
}

void nrf_rx_data_handler(void)
{
	uint8_t i;
	
	 if(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
	 {

		 printf("[[[");
		 
		 
//		rf_int_status |= 0x40;							
//		m_tx_buf[0] = UESB_READ_RF_INT_STATUS;
//		m_tx_buf[1] = 0x02 + rx_payload.length + 1;		// + rssi
//		m_tx_buf[2] = rf_int_status;
//		m_tx_buf[3] = rx_payload.length + 1;			// + rssi	
//		memcpy(&m_tx_buf[4], rx_payload.data, rx_payload.length);	//2.4G接收到的数据copy到SPI发送反冲区内
//		m_tx_buf[rx_payload.length + 4] = rx_payload.rssi;
//		m_tx_buf[rx_payload.length + 5] = XOR_Cal(m_tx_buf, rx_payload.length + 5);
//		tx_data_len = rx_payload.length + 6;	
//		flag_m_tx_buf_not_empty = 1;
//		
//		nrf_gpio_pin_clear(SPIS_IRQ_PIN);		//接收2.4G数据后产生低电平脉冲，通知stm32中断读取SPI数据
//		nrf_delay_us(1);
//		nrf_gpio_pin_set(SPIS_IRQ_PIN);
//		spi_busy_status = SPI_BUSY;				
	}
}

uint32_t my_tx_esb_init(void)
{
    uint32_t err_code;
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

    nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
    nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.retransmit_delay         = 600;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_1MBPS;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
    nrf_esb_config.mode                     = NRF_ESB_MODE_PTX;
	nrf_esb_config.selective_auto_ack       = true;	//lj 理解，等于true时，每次发送的时候可选择回不回复ACK
	nrf_esb_config.payload_length           = 250;
	
	err_code = nrf_esb_set_rf_channel(12);		//注意：答题器发送频点61接收频点21，接收器相反
	VERIFY_SUCCESS(err_code);
	
    err_code = nrf_esb_init(&nrf_esb_config);
    VERIFY_SUCCESS(err_code);

    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);
	
    err_code = nrf_esb_set_prefixes(addr_prefix, 8);
    VERIFY_SUCCESS(err_code);

    return err_code;
}

uint32_t my_rx_esb_init(void)
{
	uint32_t err_code;
	uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
	uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

	nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
	nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
	nrf_esb_config.retransmit_delay         = 600;
	nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_1MBPS;
	nrf_esb_config.event_handler            = nrf_esb_event_handler;
	nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
	nrf_esb_config.selective_auto_ack       = true;	
	nrf_esb_config.payload_length           = 250;

	err_code = nrf_esb_set_rf_channel(45);		
	VERIFY_SUCCESS(err_code);
	
	err_code = nrf_esb_init(&nrf_esb_config);
	VERIFY_SUCCESS(err_code);

	err_code = nrf_esb_set_base_address_0(base_addr_0);
	VERIFY_SUCCESS(err_code);

	err_code = nrf_esb_set_prefixes(addr_prefix, 8);
	VERIFY_SUCCESS(err_code);

	return err_code;
}















