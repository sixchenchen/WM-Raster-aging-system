#include "gd32f10x.h"
#include "delay.h"
#include "tm1637.h"
#include "systick.h"
#include "led.h"
#include "seg.h"
#include "key.h"
#include "setting.h"
#include "key_process.h"
#include "rs485.h"
#include "sensor.h"
#include "OLED.h"
#include "dip_switch.h"
#include "system_config.h"
#include "rs485_master_task.h"
#include "rs485_slave_task.h"
#include "oled_task.h"

int main(void)
{
	systick_config();
	System_Config_Init();
	Setting_Init();
	Sensor_Init();
	SEG_Init();
	Key_Init();
	Dip_Switch_Init();
	RS485_Init(RS485_Baud);
	LED_Init();
	OLED_Init();
	const System_Config *config = System_Config_Get();
	while (1)
	{
		if (config->board == BOARD_MASTER)
		{
			RS485_Task();
			RS485_Master_Task(); // 收集分板数据，并实时显示出来
			OLED_Task();
		}
		else if (config->board == BOARD_SLAVE)
		{
			Sensor_Task(); // 接收触发信号光栅信号
			RS485_Task();
			RS485_Slave_Task(); // 接收的光栅信号实时反馈给主板
			Key_Task();			// 收集按钮触发事件
			Key_Process_Task(); // 处理按钮出发时间
			SEG_Task();			// 显示按钮触发事件
		}
	}
}
