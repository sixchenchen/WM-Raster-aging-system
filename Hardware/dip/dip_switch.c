#include "dip_switch.h"

/*
 PA8 PA9 PA10
 Three-position mode selector
*/
#define MODE_PORT GPIOA

#define SW_MODE0 GPIO_PIN_8
#define SW_MODE1 GPIO_PIN_9
#define SW_MODE2 GPIO_PIN_10

/*
 PB1 PB0 PA7 PA6 PA5
 Five-digit address code
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

/**
 * @brief initialize the dip switch
 *
 */
void Dip_Switch_Init(void)
{
    // 1.config GPIOA,GPIOB clock enable
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    // 2.init GPIO,pull-up input mode
    gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,
              GPIO_PIN_5 |
                  GPIO_PIN_6 |
                  GPIO_PIN_7 |
                  GPIO_PIN_8 |
                  GPIO_PIN_9 |
                  GPIO_PIN_10);
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ,
              GPIO_PIN_1 |
                  GPIO_PIN_0);
}

/**
 * @brief Read a single GPIO status
 *
 * @param port
 * @param pin
 * @return uint8_t
 */
static uint8_t Read_SW(uint32_t port, uint32_t pin)
{
    if ((gpio_input_bit_get(port, pin)) == RESET) // Toggling the DIP switch pulls the pin low to ground level
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Obitain the address of the toggle switch
 *
 * @return uint8_t
 */
uint8_t DIP_Read_Address(void)
{
    uint8_t address = 0;
    address |= Read_SW(ADDR0_PORT, ADDR0_PIN) << 0;
    address |= Read_SW(ADDR1_PORT, ADDR1_PIN) << 1;
    address |= Read_SW(ADDR2_PORT, ADDR2_PIN) << 2;
    address |= Read_SW(ADDR3_PORT, ADDR3_PIN) << 3;
    address |= Read_SW(ADDR4_PORT, ADDR4_PIN) << 4;
    return address;
}

/**
 * @brief Obitain Configuration information
 * mode:
 * 0 -> MASTER
 * 1 -> SLAVE  PNP NO
 * 2 -> SLAVE  PNP NC
 * 3 -> SLAVE  NPN NO
 * 4 -> SLAVE  NPN NC
 * default -> MASTER
 * @return Dip_Config
 */
Dip_Config DIP_Read_Config(void)
{
    Dip_Config config;
    uint8_t mode = 0;
    mode |= Read_SW(MODE_PORT, SW_MODE0) << 0;
    mode |= Read_SW(MODE_PORT, SW_MODE1) << 1;
    mode |= Read_SW(MODE_PORT, SW_MODE2) << 2;
    switch (mode)
    {
    case 0:
        config.board = BOARD_MASTER;
        break;
    case 1:
        config.board = BOARD_SLAVE;
        config.sensor_type = SENSOR_PNP;
        config.sensor_mode = SENSOR_NO;
        break;
    case 2:
        config.board = BOARD_SLAVE;
        config.sensor_type = SENSOR_PNP;
        config.sensor_mode = SENSOR_NC;
        break;
    case 3:
        config.board = BOARD_SLAVE;
        config.sensor_type = SENSOR_NPN;
        config.sensor_mode = SENSOR_NO;
        break;
    case 4:
        config.board = BOARD_SLAVE;
        config.sensor_type = SENSOR_NPN;
        config.sensor_mode = SENSOR_NC;
        break;
    default:
        config.board = BOARD_ERROR;
        break;
    }
    config.address = DIP_Read_Address();
    return config;
}
