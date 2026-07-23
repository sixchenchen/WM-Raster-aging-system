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

int main(void)
{

	systick_config();
	SEG_Init();
	Key_Init();
	Sensor_Init();
	while (1)
	{
		SEG_Task();
		Key_Task();
		Key_Process_Task();
		Sensor_Task();
	}
}
 