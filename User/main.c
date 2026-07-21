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

int main(void)
{
	// init system tick
	systick_config();
	// init dip switch
	Dip_Switch_Init();
	RS485_Init(RS485_Baud);
	LED_Init();
	SEG_Init();
	Delay_init();
	Sensor_Init();
	Key_Init();
	g_config = DIP_Read_Config();

	while (1)
	{
		RS485_Task();
		// 2.Type selection mode for judgment
		if (g_config.board == BOARD_MASTER)
		{
			RS485_Master_Task();
		}
		else if (g_config.board == BOARD_SLAVE)
		{
			Sensor_Task();
			Key_Task();
			Setting_Task();
			SEG_Task();
			RS485_Slave_Task();
		}
	}
}
