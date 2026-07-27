#include "sensor.h"
#include "systick.h"
#include "setting.h"

// Raster trigger flag bit
volatile uint8_t sensor_event = 0;
/*
    Save the previous state Prevent continuous repeated counting
*/
static uint8_t sensor_last_state = 1;
/* initilize sensor */
void Sensor_Init(void)
{
    // 1.config GPIOA、AF clock enable
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_AF);
    // 2.config PA4 mode
    gpio_init(SENSER_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, SENSOR_PIN);
    /*
       PA4
       EXTI4
   */
    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_4);

    exti_init(EXTI_4, EXTI_INTERRUPT, EXTI_TRIG_FALLING);

    exti_interrupt_flag_clear(EXTI_4);

    nvic_irq_enable(EXTI4_IRQn, 1, 1);
}

/* sensor task */
void Sensor_Task(void)
{
    static uint32_t last_trigger_time = 0;
    if (sensor_event)
    {
        /*
           Software debouncing Prevent optocoupler jitter
        */
        if (GetTimeElapsed(last_trigger_time) > 50)
        {
            Setting_AddCount();
            last_trigger_time = GetTick();
        }
        sensor_event = 0;
    }
}

void EXTI4_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_4))
    {
        sensor_event = 1;
        exti_interrupt_flag_clear(EXTI_4);
    }
}
