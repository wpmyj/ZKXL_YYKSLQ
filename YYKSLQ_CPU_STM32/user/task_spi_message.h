#ifndef	__APP_SEND_DATA_PROCESS_H_
#define	__APP_SEND_DATA_PROCESS_H_

#include "main.h"
#include "task_show_message.h"

//#define OPEN_BUFFER_ACK_SHOW
#define OPEN_BUFFER_DATA_SHOW
//#define OPEN_ACK_TABLE_SHOW

/* 打印信息控制 */
#ifdef 	OPEN_BUFFER_ACK_SHOW
#define DEBUG_BUFFER_ACK_LOG							     DEBUG_LOG
#else
#define DEBUG_BUFFER_ACK_LOG(...)
#endif

#ifdef 	OPEN_BUFFER_DATA_SHOW
#define DEBUG_BUFFER_DTATA_LOG							   DEBUG_LOG
#else
#define DEBUG_BUFFER_DTATA_LOG(...)
#endif


/* 状态 */
#define SEND_IDLE_STATUS                0
#define SEND_500MS_DATA_STATUS          (SEND_IDLE_STATUS+1)
#define SEND_500MS_TIMER_START_STATUS   (SEND_500MS_DATA_STATUS+1)
#define SEND_500MS_TIMER_TIMEOUT_STATUS (SEND_500MS_TIMER_START_STATUS+1)
#define SEND_2S_DATA_STATUS             (SEND_500MS_TIMER_TIMEOUT_STATUS+1)
#define SEND_2S_TIMER_START_STATUS      (SEND_2S_DATA_STATUS+1)
#define SEND_2S_TIMER_TIMEOUT_STATUS    (SEND_2S_TIMER_START_STATUS+1)

#define RETRANSMIT_SEND_DATA_COUNT  (send_data_process_tcb.retransmit_count)//5

/* 前导帧 */
#define SEND_PRE_COUNT              (send_data_process_tcb.pre_data_count)
#define SEND_PRE_DELAY100US         (send_data_process_tcb.pre_data_delay100us)
#define SEND_PRE_TABLE              12

/* 数据帧 */
#define SEND_DATA_COUNT             2
#define SEND_DATA_DELAY100US        (send_data_process_tcb.data_delay100us)//50
#define SEND_DATA_ACK_TABLE         2


#define PACKAGE_NUM_ADD             1
#define PACKAGE_NUM_SAM             0

#define CLICKER_TIMER_STR_LEN       35

typedef struct
{
	uint8_t  pre_data_count;
	uint16_t pre_data_delay100us;
	uint8_t  data_count;
	uint16_t data_delay100us;
	uint16_t rand_delayms;
	uint8_t  retransmit_count;
	uint8_t  is_pack_add;
}send_data_process_tcb_tydef;

extern task_tcb_typedef send_data_task;
extern volatile send_data_process_tcb_tydef send_data_process_tcb;

void App_clickers_send_data_process( void );
void set_spi_rf_rev_status( uint8_t new_status );
void spi_rev_timer_init(void);
uint8_t get_spi_rf_rev_status( void );
#endif
