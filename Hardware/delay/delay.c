#include "gd32f10x.h"
#include "systick.h"

void Delay_init(void)
{
    /*
        SysTick is reserved for the 1 ms system time base.
        Keep this function for compatibility with existing callers.
    */
}

void Delay_us(uint32_t us)
{
    uint32_t loops;

    /*
        Do not use SysTick here: it is configured as the 1 ms interrupt
        source. This short busy wait is intended for GPIO bit-banging only.
    */
    loops = (SystemCoreClock / 4000000U) * us;
    while (loops--)
    {
        __NOP();
    }
}

void Delay_ms(uint32_t ms)
{
    uint32_t start;

    if (ms == 0U)
    {
        return;
    }

    start = GetTick();
    while (GetTimeElapsed(start) < ms)
    {
        /* Keep interrupts enabled so the system tick continues running. */
    }
}
