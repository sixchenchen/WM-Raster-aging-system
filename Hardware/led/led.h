#ifndef __LED_H
#define __LED_H

#include "gd32f10x.h"

#define LED_PORT GPIOC
#define LED_PIN GPIO_PIN_13

/* initilize led */
void LED_Init(void);

/* open led */
void LED_On(void);

/* close led */
void LED_Off(void);

/* toggle led */
void LED_Toggle(void);

#endif
