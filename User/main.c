#include "gd32f10x.h"
#include "systick.h"
#include <stdio.h>
#include "main.h"
#include "dip_switch.h"
#include "rs485.h"
#include "led.h"
#include "seg.h"
#include "delay.h"
#include "rs485_master_task.h"
#include "rs485_slave_task.h"
#include "system_config.h"
#include "key.h"
#include "setting.h"
#include "icc.h"
#include "usart.h"

int main(void)
{
	// init system tick
	systick_config();
	// init
	Delay_init();
	Dip_Switch_Init();
	System_Config_Init();
	LED_Init();
	USART2_Init(USART2_BAUD);
	USART2_SendString("GD32 USART OK\r\n");
	uint8_t buf[32];
	const System_Config *config;
	config = System_Config_Get();

	while (1)
	{
		// RS485_Task();
		//  2.Type selection mode for judgment
		if (config->board == BOARD_MASTER)
		{
			// RS485_Master_Task();
			LED_On();
			Delay_ms(1000);
		}
		else if (config->board == BOARD_SLAVE)
		{
			LED_Toggle();
			if (USART2_GetRxLength())
			{
				USART2_GetRxData(buf, USART2_GetRxLength());
				USART2_SendString("RX:\r\n");
				USART2_SendArray(buf, sizeof(buf));
			}
			Delay_ms(1000);
		}
	}
}
