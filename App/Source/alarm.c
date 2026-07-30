#include "alarm.h"
#include "buzzer.h"
#include "sensor.h"
#include "systick.h"

static uint8_t alarm_active = 0;

static uint32_t alarm_tick;

#define ALARM_TIME 300

void Alarm_Init(void)
{
    alarm_active = 0;
    Buzzer_Off();
}

void Alarm_Task(void)
{
    /*
    检测光栅事件
    */
    if (alarm_event)
    {
        alarm_event = 0;
        alarm_active = 1;
        alarm_tick = GetTick();
        Buzzer_On();
    }
    /*
    300ms关闭
    */
    if (alarm_active)
    {
        if (GetTick() - alarm_tick >= ALARM_TIME)
        {
            Buzzer_Off();
            alarm_active = 0;
        }
    }
}
