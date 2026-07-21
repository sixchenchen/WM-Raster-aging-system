#include "key.h"
#include "systick.h"

static Key_Event key_event = KEY_NONE;

/*
    key initilize
*/
void Key_Init(void)
{
    // 1.initialize key
    rcu_periph_clock_enable(RCU_GPIOB);
    // 2.config GPIOB12、13、14、15 pull-up input mode
    gpio_init(KEY_PORT, GPIO_MODE_IPU,
              GPIO_OSPEED_50MHZ,
              GPIO_PIN_12 |
                  GPIO_PIN_13 |
                  GPIO_PIN_14 |
                  GPIO_PIN_15);
}

/* read key pin */
static uint8_t Key_Read(uint32_t port, uint32_t pin)
{
    if (gpio_input_bit_get(port, pin) == RESET)
    {
        return 1; // press
    }
    else
    {
        return 0; // release
    }
}

/*
    periodic invocation
    suggest to call it every 10 milisecondis
*/
void Key_Scan(void)
{
    static uint8_t last_set = 0;
    static uint8_t last_up = 0;
    static uint8_t last_down = 0;
    static uint8_t last_reset = 0;
    uint8_t now;
    // key set event
    now = Key_Read(KEY_PORT, KEY_SET);
    if (now && !last_set)
    {
        key_event = KEY_SET;
    }
    last_set = now;
    // key up event
    now = Key_Read(KEY_PORT, KEY_UP);
    if (now && !last_up)
    {
        key_event = KEY_UP;
    }
    last_up = now;
    // key donw event
    now = Key_Read(KEY_PORT, KEY_DOWN);
    if (now && !last_down)
    {
        key_event = KEY_DOWN;
    }
    last_down = now;
    // key reset event
    now = Key_Read(KEY_PORT, KEY_RESET);
    if (now && !last_reset)
    {
        key_event = KEY_RESET;
    }
    last_reset = now;
}

/*
    Obtain key press event
*/
Key_Event Key_Get_Event(void)
{
    static uint8_t temp;
    temp = key_event;
    key_event = KEY_NONE;
    return temp;
}

/* key task */
void Key_Task(void)
{
    static uint32_t last_tick = 0;
    if (GetTick() - last_tick >= 10)
    {
        last_tick = GetTick();
        Key_Scan();
    }
}
