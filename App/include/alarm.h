#ifndef __ALARM_H
#define __ALARM_H

#include "gd32f10x.h"

/*
    Alarm type
*/
typedef enum
{

    ALARM_NONE = 0,

    // Raster trigger
    ALARM_SENSOR,

    // RS485 communication failure
    ALARM_COMM,

    // system error
    ALARM_ERROR

} Alarm_Type;

/*
    initialize alarm
*/
void Alarm_Init(void);

/*
    start alarm

    type:
        ALARM_SENSOR
        ALARM_COMM
*/
void Alarm_Start(Alarm_Type type);

/*
    stop alarm
*/
void Alarm_Stop(void);

/*
    alarm task

    call in while(1)
*/
void Alarm_Task(void);

/*
    get alarm status
*/
uint8_t Alarm_IsActive(void);

#endif