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
    static uint16_t reset_time = 0;
    uint8_t now;

    // SET
    now = Key_Read(KEY_PORT, KEY_SET_PIN);
    if (now && !last_set)
    {
        key_event = KEY_SET_EVENT;
    }
    last_set = now;
    // UP
    now = Key_Read(KEY_PORT, KEY_UP_PIN);
    if (now && !last_up)
    {
        key_event = KEY_UP_EVENT;
    }
    last_up = now;
    // DOWN
    now = Key_Read(KEY_PORT, KEY_DOWN_PIN);
    if (now && !last_down)
    {
        key_event = KEY_DOWN_EVENT;
    }
    last_down = now;
    // RESET
    now = Key_Read(KEY_PORT, KEY_RESET_PIN);

    // 按下
    if (now)
    {
        reset_time++;
        // 长按2秒
        if (reset_time >= 200)
        {
            key_event = KEY_RESET_LONG_EVENT;
            reset_time = 0;
        }
    }
    else
    {
        // 松开
        if (last_reset)
        {
            /*
                如果按下时间小于2秒
                认为短按
            */

            if (reset_time > 0 && reset_time < 200)
            {
                key_event = KEY_RESET_SHORT_EVENT;
            }
        }
        reset_time = 0;
    }
    last_reset = now;
}

/*
    Obtain key press event
*/
Key_Event Key_Get_Event(void)
{
    Key_Event temp;
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
