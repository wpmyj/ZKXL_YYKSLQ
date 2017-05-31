/**
  ******************************************************************************
  * @file   	yyk_protocol.c
  * @author  	Samwu
  * @version 	V1.0.0.0
  * @date   	2017.05.27
  * @brief   	si24r2e 驱动程序
  ******************************************************************************
  */

#include "main.h"
#include "yyk_protocol.h"
#include "si24r2e.h"

extern uint8_t current_protocol;

/*---------------------------- 协议管理结构体 ------------------------*/
static yyk_pro_tyedef zkxl_yyk_pro = 
{
	"中科讯联",
	{
		81,                                                     // tx_ch
		1000,                                                   // speed
		0,                                                      // tx_power
		5,                                                      // addr_len
		{ 0x01, 0x02, 0x03, 0x04, 0x05 },                       // address
		8,                                                      // data_len
		{ 0x00, 0x00, 0x00, 0x66, 0x55, 0x44, 0x00, 0x07 },     // data
		1100                                                    // send_delay
	},
	zkxl_yyk_protocol_update_uid,
	zkxl_yyk_protocol_check_rssi
};

static yyk_pro_tyedef jxyd_yyk_pro = 
{
	"江西移动",
	{
		18,                                                     // tx_ch
		250,                                                    // speed
		0,                                                      // tx_power
		4,                                                      // addr_len
		{ 0x33, 0x20, 0x0d, 0x54 },                             // address
		10,                                                     // data_len
		{ 0x00, 0x00, 0x00, 0x66, 0x55, 0x44, 0x00, 0x07 },     // data
		1100                                                    // send_delay
	}
};

static yyk_pro_tyedef cqyd_yyk_pro = 
{
	"重庆移动",
	{
		18,                                                     // tx_ch
		250,                                                    // speed
		0,                                                      // tx_power
		4,                                                      // addr_len
		{ 0x34, 0x12, 0xbb, 0xAA },                             // address
		8,                                                      // data_len
		{ 0x00, 0x00, 0x00, 0x66, 0x55, 0x44, 0x00, 0x07 },     // data
		1100                                                    // send_delay
	}
};


yyk_pro_tyedef *yyk_pro_list[YYK_PROTOCOL_MUM] = 
{
	(yyk_pro_tyedef *)&zkxl_yyk_pro,
	(yyk_pro_tyedef *)&jxyd_yyk_pro,
	(yyk_pro_tyedef *)&cqyd_yyk_pro,
	NULL,
};

/*---------------------------- 协议共有函数定义 ------------------------*/
int16_t yyk_protocol_update_rf_setting( yyk_pro_tyedef *pprotocol )
{
	/* 打印协议名 */
	//printf("PROTOCOL NAME: %s",pprotocol->name);
	/* 同步信道 */
	if(pprotocol->conf.tx_ch < 127)
		txbuf[NVM_RF_CHA] = (txbuf[NVM_RF_CHA] & 0x80) | pprotocol->conf.tx_ch; 
	else
		return -1;

	/* 同发送速率 */
	if(pprotocol->conf.tx_speed <= 2000)
	{
		txbuf[NVM_RF_SETUP] = (txbuf[NVM_RF_SETUP] & 0xC7);
		switch( pprotocol->conf.tx_speed )
		{
			case 1000: break;
			case 2000: txbuf[NVM_RF_SETUP] |= 0x08; break;
			case 250 : txbuf[NVM_RF_SETUP] |= 0x20; break;
			default: break;
		}
	}
	else
		return -2;

	/* 同步发送速率 */
	if(pprotocol->conf.tx_speed <= 2000)
	{
		txbuf[NVM_RF_SETUP] = (txbuf[NVM_RF_SETUP] & 0xC7);
		switch( pprotocol->conf.tx_speed )
		{
			case 1000: break;
			case 2000: txbuf[NVM_RF_SETUP] |= 0x08; break;
			case 250 : txbuf[NVM_RF_SETUP] |= 0x20; break;
			default: break;
		}
	}
	else
		return -2;

	/* 同步发送功率 */
	if((pprotocol->conf.tx_power <= 7) && (pprotocol->conf.tx_power >= -12))
	{
		txbuf[NVM_RF_SETUP] = (txbuf[NVM_RF_SETUP] & 0xF8);
		switch( pprotocol->conf.tx_power )
		{
			case -12: break;
			case -6 : txbuf[NVM_RF_SETUP] |= 0x01; break;
			case -4 : txbuf[NVM_RF_SETUP] |= 0x02; break;
			case  0 : txbuf[NVM_RF_SETUP] |= 0x03; break;
			case  1 : txbuf[NVM_RF_SETUP] |= 0x04; break;
			case  3 : txbuf[NVM_RF_SETUP] |= 0x05; break;
			case  4 : txbuf[NVM_RF_SETUP] |= 0x06; break;
			case  7 : txbuf[NVM_RF_SETUP] |= 0x07; break;
			default: break;
		}
	}
	else
		return -3;
	
	/* 同步地址信息 */
	if(pprotocol->conf.addr_len <= 5)
	{
		uint8_t no_addr = 0;
		txbuf[NVM_FEATURE] = (txbuf[NVM_FEATURE] & 0xFC);
		switch( pprotocol->conf.addr_len )
		{
			case  3 : txbuf[NVM_FEATURE] |= 0x01; break;
			case  4 : txbuf[NVM_FEATURE] |= 0x02; break;
			case  5 : txbuf[NVM_FEATURE] |= 0x03; break;
			default : no_addr = 1;                break;
		}
		if(no_addr == 0)
			memcpy(txbuf+NVM_TX_ADDR_0,pprotocol->conf.addr,
		         pprotocol->conf.addr_len);
	}
	else
		return -4;

	/* 同步发送数据 */
	if(pprotocol->conf.data_len <= 32)
	{
		txbuf[NVM_FIFO_LEN]  = (txbuf[NVM_FIFO_LEN] & 0xC0);
		txbuf[NVM_FIFO_LEN] |= pprotocol->conf.data_len;
		memcpy(txdata,pprotocol->conf.data,
		         pprotocol->conf.data_len);
	}
	else
		return -5;

	return 0;
}

/*---------------------------- 协议私有函数定义 ------------------------*/
int16_t zkxl_yyk_protocol_update_uid( void *pprotocol, uint8_t *data )
{
	char str[20];
	uint8_t pwdata[5],rdata_index = 0;
	uint8_t prdata[10];
	uint8_t write_flag = 0;
	uint8_t re_write_count = 0;
	yyk_pro_tyedef *ppro = (yyk_pro_tyedef *)pprotocol;

	sprintf(str, "%010u" , *(uint32_t *)(data));
	
	memset(pwdata,0,5);
	while( rdata_index < strlen( str ) )
	{
		pwdata[rdata_index/2] = (((str[rdata_index  ]-'0') << 4) & 0xF0) | 
							 ((str[rdata_index+1]-'0') & 0x0F);
		rdata_index = rdata_index + 2;
	}

	/* 同步UID */
	if(ppro->conf.data_len <= 32)
	{
		memcpy( txdata + 3, pwdata, 5 );
		memcpy( ppro->conf.data + 3, pwdata, 5 );
	}

	/* 检测UID，决定是否重新写入 */
	do
	{
		rdata_index = 0;
		write_flag  = 0;

		if( si24r2e_read_nvm( prdata ) == 127)
			return -2;

		for(rdata_index = 0; rdata_index<ppro->conf.data_len; rdata_index++ )
		{
		//printf("rdata:%02x  wdata:%02x\r\n",prdata[rdata_index],ppro->conf.data[rdata_index]);
			if( prdata[rdata_index] != ppro->conf.data[rdata_index] )
			{
				write_flag = 1;
			}
		}

		if( write_flag == 1 )
		{
			si24r2e_write_nvm(ppro->conf.data);
			
			re_write_count++;
			if( re_write_count >= 6 )
			{
				write_flag = 0;
				return -1;
			}
		}
	}while(write_flag == 1 );
	return 0;
}

int16_t zkxl_yyk_protocol_check_rssi( void *pprotocol, uint8_t *data )
{
	uint8_t i,check_flg = 0;
	
	for(i = 0; i<yyk_pro_list[current_protocol]->conf.data_len; i++ )
	{
		if( data[i+1] != yyk_pro_list[current_protocol]->conf.data[i] )
		{
			check_flg = 1;
		}
	}

	if( check_flg == 0 )
	{
		b_print("{\r\n");
		b_print("  \"fun\": \"rssi_check\",\r\n");
		b_print("  \"pro_name\": \"%s\",\r\n",yyk_pro_list[current_protocol]->name);
		b_print("  \"card_id\": \"%02x%02x%02x%02x%02x\",\r\n",data[4],
		        data[5],data[6],data[7],data[8]);
		b_print("  \"check_rssi\": \"-%d\",\r\n",data[0]);
		b_print("  \"result\": \"%d\"\r\n",check_flg);
		b_print("}\r\n");
	}
	else
	{
		return -1;
	}
		return 0;
}
