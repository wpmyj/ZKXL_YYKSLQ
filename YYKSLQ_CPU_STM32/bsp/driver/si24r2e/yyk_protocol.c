/**
  ******************************************************************************
  * @file   	yyk_protocol.c
  * @author  	Samwu
  * @version 	V1.0.0.0
  * @date   	2017.05.27
  * @brief   	si24r2e ��������
  ******************************************************************************
  */

#include "main.h"
#include "yyk_protocol.h"
#include "si24r2e.h"

extern uint8_t current_protocol;
extern uint8_t show_log_flag;

/*---------------------------- Э�����ṹ�� ------------------------*/
static yyk_pro_tyedef zkxl_yyk_pro = 
{
	"ZKXL",
	{
		81,                                                     // tx_ch
		1000,                                                   // speed
		0,                                                      // tx_power
		5,                                                      // addr_len
		{ 0x01, 0x02, 0x03, 0x04, 0x05 },                       // address
		8,                                                      // data_len
		{ 0x00, 0x00, 0x00, 0x66, 0x55, 0x44, 0x00, 0x07 },     // data
		1100,                                                   // send_delay
		3101                                                    // version
	},
	zkxl_yyk_protocol_update_uid,
	zkxl_yyk_protocol_check_rssi,
	zkxl_yyk_protocol_check_rssi_print,
};

yyk_pro_tyedef *yyk_pro_list[YYK_PROTOCOL_MUM] = 
{
	(yyk_pro_tyedef *)&zkxl_yyk_pro,
	NULL,
};

/*---------------------------- Э�鹲�к������� ------------------------*/
int16_t yyk_protocol_update_rf_setting( yyk_pro_tyedef *pprotocol )
{
	/* ��ӡЭ���� */
	//printf("PROTOCOL NAME: %s",pprotocol->name);
	/* ͬ���ŵ� */
	if(pprotocol->conf.tx_ch < 127)
		txbuf[NVM_RF_CHA] = (txbuf[NVM_RF_CHA] & 0x80) | pprotocol->conf.tx_ch; 
	else
		return -1;

	/* ͬ�������� */
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

	/* ͬ���������� */
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

	/* ͬ�����͹��� */
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
	
	/* ͬ����ַ��Ϣ */
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

	/* ͬ���������� */
	if(pprotocol->conf.data_len <= 32)
	{
		txbuf[NVM_FIFO_LEN]  = (txbuf[NVM_FIFO_LEN] & 0xC0);
		txbuf[NVM_FIFO_LEN] |= pprotocol->conf.data_len;
		memcpy(txdata,pprotocol->conf.data,
		         pprotocol->conf.data_len);
	}
	else
		return -5;

	/* ͬ�����ͼ�� */
	if(pprotocol->conf.send_delay >= 270)
	{
		txbuf[NVM_FIFO_LEN] = (txbuf[NVM_FIFO_LEN] & 0x3F);
		txbuf[NVM_RF_TH]    = (((pprotocol->conf.send_delay-270) * 3)&0xFF00) >> 8;
		txbuf[NVM_RF_TL]    = (((pprotocol->conf.send_delay-270)* 3)&0xFF);
	}
	else
		return -6;

 	{
 		txbuf[NVM_TEST]        = txbuf[NVM_TEST] & 0x08;
 		txbuf[NVM_LV_REP]      = (txbuf[NVM_LV_REP] & 0x01) | ((system_rtc_timer.year % 10) << 4);
 		txbuf[NVM_LV_REP]      = 0x01;
 		txbuf[NVM_TRIM_CLOCK]  = (txbuf[NVM_TRIM_CLOCK] & 0xF0) | 0x60;
 	}
 
 	{
 		txbuf[NVM_FEATURE]  = txbuf[NVM_FEATURE] & 0x10;
 	}
	return 0;
}

/*---------------------------- Э��˽�к������� ------------------------*/
int16_t zkxl_yyk_protocol_update_uid( void *pprotocol, uint8_t *data )
{
	char str[20];
	uint8_t pwdata[5],rdata_index = 0;
	uint8_t prdata[10] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t write_flag = 0;
	uint8_t no_power_flag = 0;
	uint8_t re_write_count = 0;
	uint8_t program_count  = 0;
	yyk_pro_tyedef *ppro = (yyk_pro_tyedef *)pprotocol;

	/* ͬ��UID */
	if(ppro->conf.data_len <= 32)
	{
		sprintf(str, "%010u" , *(uint32_t *)(data));
		memset(pwdata,0,5);
		while( rdata_index < strlen( str ) )
		{
			pwdata[rdata_index/2] = (((str[rdata_index  ]-'0') << 4) & 0xF0) | 
								 ((str[rdata_index+1]-'0') & 0x0F);
			rdata_index = rdata_index + 2;
		}
		memcpy( txdata + 3, pwdata, 5 );
		memcpy( ppro->conf.data + 3, pwdata, 5 );
	}

	/* ͬ��ϵͳʱ�� */
	if(ppro->conf.data_len <= 32)
	{
		uint8_t time = (system_rtc_timer.mon - 1)* 3 + system_rtc_timer.date / 10 + 1;
		memset(pwdata,0,5);
		pwdata[0] = (time / 10) << 4 | (time % 10);
		pwdata[1] = (system_rtc_timer.year % 10) << 4;
		pwdata[3] = (ppro->conf.version % 10) & 0x0F;
		
		memcpy( txdata, pwdata, 3 );
		memcpy( ppro->conf.data, pwdata, 3 );
	}

	/* ���UID�������Ƿ�����д�� */
	for(re_write_count=0;re_write_count<=6;re_write_count++)
	{		
		rdata_index = 0;
		write_flag  = 0;

    /* д����� */
		program_count = si24r2e_read_nvm( prdata );
		no_power_flag = get_power_status();
		if( program_count > 127 | no_power_flag ==0 )
			return -3;

		if(show_log_flag >= 1)
		{
			b_print("{\"fun\":\"nvm_opration\",\"operation\": \"rd\",\"read_burn_count\": \"%d\",",program_count);
			b_print("\"read_nvm_data\": \"%02x %02x %02x %02x %02x %02x %02x %02x\"}",
			prdata[0],prdata[1],prdata[2],prdata[3],prdata[4],prdata[5],prdata[6],prdata[7]);
		}
		else
			b_print("{\"fun\":\"nvm_opration\",\"operation\": \"rd\",\"read_burn_count\": \"%d\"}",program_count);

		if( program_count == 127 )
			return -2;

		/* �ϵ��� */
		for(rdata_index = 0; rdata_index<6; rdata_index++ )
		{
			if( prdata[rdata_index] != prdata[rdata_index+1])
				no_power_flag = 1;
		}

		for(rdata_index = 0; rdata_index<ppro->conf.data_len; rdata_index++ )
		{
			if( prdata[rdata_index] != ppro->conf.data[rdata_index] )
			{
				write_flag = 1;
			}
		}

		if( write_flag == 1 )
		{
			si24r2e_write_nvm(ppro->conf.data);
			if(show_log_flag >= 1)
				b_print("{\"fun\":\"nvm_opration\",\"operation\": \"wr\"}\r\n");
			re_write_count++;
			if( re_write_count >= 6 )
			{
				write_flag = 0;
				return -1;
			}
		}
		else
		{
			re_write_count = 6;
			return 0;
		}
	}
	return 0;
}

int16_t zkxl_yyk_protocol_check_rssi( void *pprotocol, uint8_t *data )
{
	uint8_t i;
	int8_t  check_flg = 0;
	
	for(i = 0; i<yyk_pro_list[current_protocol]->conf.data_len; i++ )
	{
		if( data[i+1] != yyk_pro_list[current_protocol]->conf.data[i] )
		{
			check_flg = -1;
		}
	}

	return check_flg;
}

void zkxl_yyk_protocol_check_rssi_print( void *pprotocol, uint8_t *data, int16_t result)
{
	b_print("{\r\n");
	b_print("  \"fun\": \"rssi_check\",\r\n");
  b_print("  \"pro_name\": \"%s\",\r\n",yyk_pro_list[current_protocol]->name);
	if( result == 0 )
	{
		b_print("  \"card_id\": \"%02x%02x%02x%02x%02x\",\r\n",data[4],
					data[5],data[6],data[7],data[8]);
		b_print("  \"check_rssi\": \"-%d dBm\",\r\n",data[0]);
	}
	else
	{
		b_print("  \"card_id\": \"%02x%02x%02x%02x%02x\",\r\n",0xFF,0xFF,0xFF,0xFF,0xFF);
		b_print("  \"check_rssi\": \"%d dBm\",\r\n",0);
	}
	b_print("  \"result\": \"%d\"\r\n",result);
	b_print("}\r\n");
}
