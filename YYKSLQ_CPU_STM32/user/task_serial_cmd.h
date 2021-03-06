#ifndef	__POS_HANDLE_LAYER_H_
#define	__POS_HANDLE_LAYER_H_
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cJSON.h"
/* Private define ------------------------------------------------------------*/

/* Uart to App cmd */
#define APP_SERIAL_CMD_STATUS_IDLE        0x00
#define APP_SERIAL_CMD_STATUS_WORK        0x01
#define APP_SERIAL_CMD_STATUS_ERR         0x02
#define APP_SERIAL_CMD_STATUS_IGNORE      0x03
#define APP_SERIAL_CMD_STATUS_WORK_IGNORE 0x04

#define START_SEND_DATA                   0
#define STOP_SEND_DATA                    1

#define LEN_LEN                           2
#define UID_LEN                           4
#define ACKTABLELEN                       16
#define REVICER_MESSAGE_LEN               750

void App_seirial_cmd_process(void);

/* Uart Message configuration */


/* Uart Message frame header and tail */
#define UART_SOF								          ('{')							//֡ͷ
#define UART_EOF 								          ('}')							//֡β

/* Uart message status */
#define UartSTART 							          (1)
#define UartDATA	 								        (2)
#define UartEND 								          (3)

#define ANSWER_STATUS_FUN                  1
#define ANSWER_STATUS_TIME                 2
#define ANSWER_STATUS_QUESTION             3
#define ANSWER_STATUS_DATA_TYPE            4
#define ANSWER_STATUS_DATA_ID              5
#define ANSWER_STATUS_DATA_RANGE           6
#define ANSWER_STATUS_HAND                 7
#define ANSWER_STATUS_SIGN                 8

#define IMPORT_STATUS_FUN                  0x01
#define IMPORT_STATUS_ADDR                 0x02
#define IMPORT_STATUS_TX_CH                0x03
#define IMPORT_STATUS_RX_CH                0x04
#define IMPORT_STATUS_TX_POWER             0x05
#define IMPORT_STATUS_LIST                 0x06
#define IMPORT_STATUS_UPOS                 0x07
#define IMPORT_STATUS_UID                  0x08

typedef struct
{
	char    *key;
	uint8_t key_str_len;
	uint8_t status;
}json_item_typedef;


typedef enum
{
	CLOSE = 0,
	OPEN,
}Bool_Typedf;

typedef enum
{
	REVICER_PACKAGE_DATA = 0,
	REVICER_PACKAGE_ACK,
}UartPac_Typedef;

typedef struct
{
	uint8_t srcid[4];
	uint8_t cmd_type;
	uint8_t stdid[10];
	uint8_t retransmit;
}task_tcb_typedef;

typedef struct
{
	const char *cmd_str;
	uint8_t    cmd_len;
	void      (*cmd_fun)(const cJSON *object);
}serial_cmd_typedef;

void serial_cmd_find_card(const cJSON *object);
void serial_cmd_get_device_info(const cJSON *object);
void serial_cmd_si24r2e_rd_wr_nvm(const cJSON *object);
void serial_cmd_si24r2e_show_log(const cJSON *object);
#endif // __POS_HANDLE_LAYER_H_
