#include "dip_switch.h"

/*
PA8 PA9 PA10

mode switch
*/
#define MODE_PORT GPIOA

#define SW_MODE0 GPIO_PIN_8
#define SW_MODE1 GPIO_PIN_9
#define SW_MODE2 GPIO_PIN_10

/*
PB1 PB0 PA7 PA6 PA5

address switch
*/

#define ADDR0_PORT GPIOB
#define ADDR0_PIN GPIO_PIN_1

#define ADDR1_PORT GPIOB
#define ADDR1_PIN GPIO_PIN_0

#define ADDR2_PORT GPIOA
#define ADDR2_PIN GPIO_PIN_7

#define ADDR3_PORT GPIOA
#define ADDR3_PIN GPIO_PIN_6

#define ADDR4_PORT GPIOA
#define ADDR4_PIN GPIO_PIN_5

/*
    initialize dip switch
*/
void Dip_Switch_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);

    /*
        all pins pull up input

        DIP ON -> LOW
    */

    gpio_init(
        GPIOA,
        GPIO_MODE_IPU,
        GPIO_OSPEED_50MHZ,
        GPIO_PIN_5 |
            GPIO_PIN_6 |
            GPIO_PIN_7 |
            GPIO_PIN_8 |
            GPIO_PIN_9 |
            GPIO_PIN_10);

    gpio_init(
        GPIOB,
        GPIO_MODE_IPU,
        GPIO_OSPEED_50MHZ,
        GPIO_PIN_0 |
            GPIO_PIN_1);
}

/*
    read one DIP
    ON  = 1
    OFF = 0
*/
static uint8_t Dip_Read_Pin(
    uint32_t port,
    uint32_t pin)
{
    if (gpio_input_bit_get(port, pin) == RESET)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
    read address
    bit:
    PB1 -> bit0
    PB0 -> bit1
    PA7 -> bit2
    PA6 -> bit3
    PA5 -> bit4
*/
uint8_t DIP_Read_Address(void)
{
    uint8_t address = 0;
    address |= Dip_Read_Pin(ADDR0_PORT, ADDR0_PIN) << 0;
    address |= Dip_Read_Pin(ADDR1_PORT, ADDR1_PIN) << 1;
    address |= Dip_Read_Pin(ADDR2_PORT, ADDR2_PIN) << 2;
    address |= Dip_Read_Pin(ADDR3_PORT, ADDR3_PIN) << 3;
    address |= Dip_Read_Pin(ADDR4_PORT, ADDR4_PIN) << 4;
    return address;
}

/*
    read mode
    PA8  bit0
    PA9  bit1
    PA10 bit2
*/
uint8_t DIP_Read_Mode(void)
{
    uint8_t mode = 0;
    mode |= Dip_Read_Pin(MODE_PORT, SW_MODE0) << 0;
    mode |= Dip_Read_Pin(MODE_PORT, SW_MODE1) << 1;
    mode |= Dip_Read_Pin(MODE_PORT, SW_MODE2) << 2;
    return mode;
}
