#include "alarm.h"
#include "buzzer.h"
#include "systick.h"

/*
    alarm status
*/
static uint8_t alarm_active = 0;

/*
    current alarm type
*/
static Alarm_Type alarm_type = ALARM_NONE;

/*
    beep parameters

    ON time
    OFF time
*/
static uint32_t alarm_on_time = 200;

static uint32_t alarm_off_time = 300;

/*
    alarm repeat count

    0 = forever
*/
static uint8_t alarm_total_count = 3;

static uint8_t alarm_count = 0;

/*
    state machine

    0 stop
    1 buzzer on
    2 wait off

*/
static uint8_t alarm_state = 0;

static uint32_t alarm_tick = 0;

/*
    Alarm Init
*/
void Alarm_Init(void)
{

    alarm_active = 0;
    alarm_type = ALARM_NONE;
    alarm_state = 0;
    Buzzer_Off();
}

/*
    Start alarm
*/
void Alarm_Start(Alarm_Type type)
{
    alarm_type = type;
    alarm_active = 1;
    alarm_count = 0;
    alarm_state = 1;
    alarm_tick = GetTick();
    Buzzer_On();
}

/*
    Stop alarm
*/
void Alarm_Stop(void)
{
    alarm_active = 0;
    alarm_type = ALARM_NONE;
    alarm_state = 0;
    Buzzer_Off();
}

/*
    Alarm Task

    non-blocking
*/
void Alarm_Task(void)
{
    uint32_t now;
    if (alarm_active == 0)
    {
        return;
    }
    now = GetTick();
    switch (alarm_state)
    {
    /*
        buzzer ringing
    */
    case 1:
        if (now - alarm_tick >= alarm_on_time)
        {
            Buzzer_Off();
            alarm_count++;
            /*
                finished
            */
            if (alarm_total_count != 0 && alarm_count >= alarm_total_count)
            {
                Alarm_Stop();
            }
            else
            {
                alarm_state = 2;
                alarm_tick = now;
            }
        }
        break;

    /*
        waiting next beep
    */
    case 2:
        if (now - alarm_tick >= alarm_off_time)
        {
            Buzzer_On();
            alarm_state = 1;
            alarm_tick = now;
        }
        break;
    default:
        Alarm_Stop();
        break;
    }
}

uint8_t Alarm_IsActive(void)
{
    return alarm_active;
}