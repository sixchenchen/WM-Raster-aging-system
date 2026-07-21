#include "buzzer.h"
#include "systick.h"

static uint8_t buzzer_enable = 0;

/*
    0 close
    1 ringing
    2 waiting interval
*/
static uint8_t buzzer_state = 0;
static uint8_t alarm_count = 0;
static uint8_t alarm_times = 0;
static uint32_t on_time = 0;
static uint32_t off_time = 0;
static uint32_t buzzer_tick = 0;

/* initilizer buzzer */
void Buzzer_Init(void)
{
    // 1. configure the clock enable
    rcu_periph_clock_enable(RCU_GPIOA);
    // 2. configure pin mode
    gpio_init(BUZZER_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, BUZZER_PIN);
    // 3. default close
    gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
}

/* open buzzer */
void Buzzer_On(void)
{
    gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
}

/* close buzzer */
void Buzzer_Off(void)
{
    gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
}

/* buzzer toggle blocking type  */
void Buzzer_Toggle(void)
{
    if (RESET == gpio_output_bit_get(BUZZER_PORT, BUZZER_PIN))
    {
        gpio_bit_set(BUZZER_PORT, BUZZER_PIN);
    }
    else if (SET == gpio_output_bit_get(BUZZER_PORT, BUZZER_PIN))
    {
        gpio_bit_reset(BUZZER_PORT, BUZZER_PIN);
    }
}

/* Non-blocking buzzer  */
void Buzzer_Start(uint32_t time_ms)
{
    buzzer_state = 1;
    buzzer_tick = system_ms;
    on_time = time_ms;
    Buzzer_On();
}

/* buzzer beep */
void Buzzer_Beep(uint32_t time_ms)
{
    Buzzer_On();
    delay_1ms(time_ms);
    Buzzer_Off();
}

/* buzzer alarm */
void Buzzer_Alarm(uint8_t times, uint32_t on_ms, uint32_t off_ms)
{
    alarm_times = times;
    alarm_count = 0;
    on_time = on_ms;
    off_time = off_ms;
    buzzer_state = 1;
    buzzer_tick = system_ms;
    Buzzer_On();
}

/* buzzer task */
void Buzzer_Task()
{
    if (buzzer_state == 0)
    {
        return;
    }
    switch (buzzer_state)
    {
    case 1: // ringing
        if (system_ms - buzzer_tick >= on_time)
        {
            Buzzer_Off();
            alarm_count++;
            /* the first sound */
            if (alarm_times == 0)
            {
                buzzer_state = 0;
            }
            /* finish */
            else if (alarm_count >= alarm_times)
            {
                buzzer_state = 0;
            }
            /* next */
            else
            {
                buzzer_state = 2;
                buzzer_tick = system_ms;
            }
        }
        break;
    case 2: // waiting next ringing
        if (system_ms - buzzer_tick >= off_time)
        {
            Buzzer_On();
            buzzer_state = 1;
            buzzer_tick = system_ms;
        }
        break;
    default:
        buzzer_state = 0;
        break;
    }
}
