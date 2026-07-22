#include "buzzer.h"

/*
    buzzer init
*/
void Buzzer_Init(void)
{

    rcu_periph_clock_enable(RCU_GPIOA);

    gpio_init(BUZZER_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, BUZZER_PIN);

    // default off
    Buzzer_Off();
}

/*
    open buzzer
*/
void Buzzer_On(void)
{
    gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
}

/*
    close buzzer
*/
void Buzzer_Off(void)
{
    gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
}

/*
    toggle
*/
void Buzzer_Toggle(void)
{
    if (gpio_output_bit_get(BUZZER_PORT, BUZZER_PIN) == RESET)
    {
        Buzzer_On();
    }
    else
    {
        Buzzer_Off();
    }
}