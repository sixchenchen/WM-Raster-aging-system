#include "gd32f10x.h"

void Delay_init(void)
{

    SysTick->CTRL = 0;

    SysTick->LOAD = 0xFFFFFF;

    SysTick->VAL = 0;

    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
}

void Delay_us(uint32_t us)
{
    uint32_t ticks;
    uint32_t start;
    ticks = us * (SystemCoreClock / 1000000);
    start = SysTick->VAL;
    while (1)
    {
        uint32_t now = SysTick->VAL;
        if (start >= now)
        {
            if (start - now >= ticks)
                break;
        }

        else
        {
            if (start + (0xFFFFFF - now) >= ticks)
                break;
        }
    }
}

void Delay_ms(uint32_t ms)
{
    while (ms--)
    {
        Delay_us(1000);
    }
}
