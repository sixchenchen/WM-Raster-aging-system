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

#define SEG_CLK_GPIO_PORT SEG_PORT
#define SEG_CLK_GPIO_PIN SEG_CLK_PIN
#define SEG_DIO_GPIO_PORT SEG_PORT
#define SEG_DIO_GPIO_PIN SEG_DIO_PIN

#define SEG_CLK_GPIO_CLK_ENABLE() rcu_periph_clock_enable(RCU_GPIOB)
#define SEG_CLK_HIGH gpio_bit_set(SEG_CLK_GPIO_PORT, SEG_CLK_GPIO_PIN)
#define SEG_CLK_LOW gpio_bit_reset(SEG_CLK_GPIO_PORT, SEG_CLK_GPIO_PIN)

#define SEG_DIO_GPIO_CLK_ENABLE() rcu_periph_clock_enable(RCU_GPIOB)
#define SEG_DIO_HIGH gpio_bit_set(SEG_DIO_GPIO_PORT, SEG_DIO_GPIO_PIN)
#define SEG_DIO_LOW gpio_bit_reset(SEG_DIO_GPIO_PORT, SEG_DIO_GPIO_PIN)

// Switch DIO direction: Output (Push-Pull) / Input (Floating)
#define SEG_DIO_GPIO_OUTPUT() gpio_init(SEG_DIO_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SEG_DIO_GPIO_PIN)
#define SEG_DIO_GPIO_INPUT() gpio_init(SEG_DIO_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, SEG_DIO_GPIO_PIN)

// CLK : Output push-pull
#define SEG_CLK_GPIO_OUTPUT() gpio_init(SEG_CLK_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, SEG_CLK_GPIO_PIN)

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
void SEG_DisplayDecimal(uint16_t num, uint8_t position);

/*
    period task
*/
void SEG_Task(void);

void SEG_Test(void);

#endif
