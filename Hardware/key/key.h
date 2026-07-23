#ifndef __KEY_H
#define __KEY_H

#include "gd32f10x.h"

/*
    key define
*/
#define KEY_SET_PIN GPIO_PIN_15   // set key
#define KEY_UP_PIN GPIO_PIN_14    // up key
#define KEY_DOWN_PIN GPIO_PIN_13  // down key
#define KEY_RESET_PIN GPIO_PIN_12 // reset key

#define KEY_PORT GPIOB

/*
    key event
*/
typedef enum
{
    KEY_NONE = 0,

    KEY_SET_EVENT,         // set
    KEY_UP_EVENT,          // up
    KEY_DOWN_EVENT,        // down
    KEY_RESET_SHORT_EVENT, // reset short
    KEY_RESET_LONG_EVENT   // reset long
} Key_Event;

/*
    key initilize
*/
void Key_Init(void);

/*
    periodic invocation
    suggest to call it every 10 milisecondis
*/
void Key_Scan(void);

/*
    Obtain key press event
*/
Key_Event Key_Get_Event(void);

/*
    key task
*/
void Key_Task(void);

#endif
