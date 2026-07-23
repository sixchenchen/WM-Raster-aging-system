#include "buzzer.h"

/*
    buzzer init
*/
void Buzzer_Init(void)
{

    // 1. First, disable JTAG and keep SWD (release PA15)
    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);

    // 2. Initialize PA15 as push-pull output
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(BUZZER_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, BUZZER_PIN);

    // 3. Buzzer is disabled by default
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