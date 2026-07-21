#include "rs485.h"
#include "string.h"
#include "systick.h"

RS485_Handle rs485;

/**
 * PA1:RE/DE
 * PA2:TX
 * PA3:RX
 */

#define RS485_PORT GPIOA
#define RS485_DE_PIN GPIO_PIN_1
#define RS485_TX_PIN GPIO_PIN_2
#define RS485_RX_PIN GPIO_PIN_3

/**
 *  RS485 transimission mode
 */
void RS485_TX_Mode(void)
{
    gpio_bit_set(RS485_PORT, RS485_DE_PIN);
}

/**
 * RS485 receiving mode
 */
void RS485_RX_Mode(void)
{
    gpio_bit_reset(RS485_PORT, RS485_DE_PIN);
}

/**
 * initilize RS485
 */
void RS485_Init(uint32_t baud)
{
    // 1.configure the clock enable
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART2);
    // 2.configure pin mode
    gpio_init(RS485_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RS485_DE_PIN);
    gpio_init(RS485_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, RS485_RX_PIN);
    gpio_init(RS485_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, RS485_TX_PIN);
    // 3.set RS485 mode
    RS485_RX_Mode();
    // 4.configure usart2
    usart_deinit(USART2);
    usart_baudrate_set(USART2, baud);
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);
    usart_parity_config(USART2, USART_PM_NONE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    // 5.open reciveing interrupt
    usart_interrupt_enable(USART2, USART_INT_RBNE);
    nvic_irq_enable(USART2_IRQn, 1, 1);
    usart_enable(USART2);
}

/**
 * RS485 send byte
 */
void RS485_SendByte(uint16_t data)
{
    RS485_TX_Mode();
    while (RESET == usart_flag_get(USART2, USART_FLAG_TBE))
        ;
    usart_data_transmit(USART2, data);
    while (RESET == usart_flag_get(USART2, USART_FLAG_TC))
        ;
    RS485_RX_Mode();
}

/**
 * RS485 send array
 */
void RS485_SendArray(uint8_t *data, uint16_t len)
{
    uint8_t i;
    RS485_TX_Mode();
    for (i = 0; i < len; i++)
    {
        while (RESET == usart_flag_get(USART2, USART_FLAG_TBE))
            ;
        usart_data_transmit(USART2, data[i]);
    }

    while (RESET == usart_flag_get(USART2, USART_FLAG_TC))
        ;
    RS485_RX_Mode();
}

/**
 * RS485 send string
 */
void RS485_SendString(char *str)
{
    while (*str)
    {
        RS485_SendByte(*str++);
    }
}

/**
 * Receiving interrupt
 */
void USART2_IRQHandler(void)
{
    if (usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE))
    {
        uint8_t data;
        data = usart_data_receive(USART2);
        if (rs485.rx_count < RS485_RX_BUF_SIZE)
        {
            rs485.rx_buf[rs485.rx_count++] = data;
            /* Each byte received Update time */
            rs485.rx_tick = GetTick();
            rs485.rx_flag = 1;
        }
    }
}
/**
 * RS485 available
 */
uint8_t RS485_Available(void)
{
    return rs485.rx_flag;
}

/**
 * RS485 read
 */

uint16_t RS485_Read(uint8_t *buf)
{
    uint16_t len;
    __disable_irq();
    len = rs485.rx_count;
    memcpy(buf, rs485.rx_buf, len);
    rs485.rx_count = 0;
    rs485.frame_ready = 0;
    rs485.rx_flag = 0;
    __enable_irq();
    return len;
}

void RS485_Task(void)
{
    if (rs485.rx_count == 0)
        return;
    /*
        9600 baud rate
        One character saves 1ms
        It is considered over if it lasts for more than 5 consecutive milliseconds
    */
    if (GetTimeElapsed(rs485.rx_tick) > 5)
    {
        rs485.frame_ready = 1;
    }
}
/* Check the completeness of the frame count */
uint8_t RS485_FrameAvailable(void)
{
    // No data was received
    if (rs485.rx_count == 0)
    {
        return 0;
    }
    // Judge the frame interval
    if (GetTimeElapsed(rs485.rx_tick) >= RS485_FRAME_TIMEOUT_MS)
    {
        rs485.frame_ready = 1;
    }
    return rs485.frame_ready;
}
