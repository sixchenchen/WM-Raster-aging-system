#ifndef __SEG_H
#define __SEG_H

#include "gd32f10x.h"

// Stop seg
#define SEG_OFF 0x00

/*
    CLK -> PB6
    DIO -> PB7
*/
#define SEG_PORT GPIOB
#define SEG_CLK_PIN GPIO_PIN_6
#define SEG_DIO_PIN GPIO_PIN_7

/* Initialize the digital tube  */
void SEG_Init(void);

/*
    display number
    0~9999
*/
void SEG_DisplayNumber(uint16_t num);

/*
    display a 4-digit array
    eg:1234
*/
void SEG_DisplayDigits(uint8_t *data);

/* clear the digital tube */
void SEG_Clear(void);

/*
    set the brightness of the digital tube
    0~7
*/
void SEG_SetBrightness(uint8_t brightness);

/*
    display with decimal point
    position:0~3
*/
void SEG_DisplayDecimal(uint16_t num,uint8_t position);

/*
    period task
*/
void SEG_Task(void);


#endif
