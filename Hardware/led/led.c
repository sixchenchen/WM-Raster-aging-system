#include "led.h"

/* initilize led */
void LED_Init(void)
{
    // 1.configure the clock enable
    rcu_periph_clock_enable(RCU_GPIOC);
    // 2.configure PC13 mode
    gpio_init(LED_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_PIN);
    // 3.default close
    LED_Off();
}

/* open led */
void LED_On(void)
{
    gpio_bit_reset(LED_PORT, LED_PIN); // led is low level effective
}

/* close led */
void LED_Off(void)
{
    gpio_bit_set(LED_PORT, LED_PIN); // led is high level effective
} 


/* toggle led */
void LED_Toggle(void)
{
    if (RESET == gpio_output_bit_get(LED_PORT, LED_PIN))
    {
        gpio_bit_set(LED_PORT, LED_PIN);
    }
    else if (SET == gpio_output_bit_get(LED_PORT, LED_PIN))
    {
        gpio_bit_reset(LED_PORT, LED_PIN);
    }
}
