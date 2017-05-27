/**
  ******************************************************************************
  * @file   	Main.c
  * @author  	Tian erjun
  * @version 	V1.0.0.0
  * @date   	2015.11.05
  * @brief   	main function for STM32F103RB
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stdlib.h"
#include "board.h"
#include "task_serial_cmd.h"
#include "task_spi_message.h"
#include "task_show_message.h"
#include "task_find_card.h"

/******************************************************************************
  Function:main
  Description:
  Input:None
  Output:
  Return:
  Others:None
******************************************************************************/
int main(void)
{
	/* System initialize -------------------------------------------------------*/
	Platform_Init();
	
	while(1)
	{	
		/*clickers send data process */
		App_clickers_send_data_process();

		/* serial cmd processing process */
		App_seirial_cmd_process();

		/* MI Card processing process */
		App_card_process();
	}	
}
