#ifndef __SENSOR_H
#define __SENSOR_H

#include "gd32f10x.h"

// Raster trigger flag bit
extern volatile uint8_t sensor_event;
extern volatile uint8_t alarm_event;

typedef enum
{
    SENSOR_NORMAL = 0,
    SENSOR_TRIGGER,
    SENSOR_ERROR
} Sensor_Status;

/*
   PA4:Low level = triggered
*/
#define SENSER_PORT GPIOA
#define SENSOR_PIN GPIO_PIN_4

/* initilize sensor */
void Sensor_Init(void);

/* senosr task */
void Sensor_Task(void);

#endif
