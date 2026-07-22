#include "usart.h"

static uint8_t rx_buf[USART2_RX_BUF_SIZE];

static volatile uint16_t rx_count = 0;

/*  USART2 init */
void USART2_Init(uint32_t baud)
{
    // 1. configure the clock enable
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_USART2);
    // 2. configure TX PB10 Reused push-pull output and RX PB11 Floating input
    gpio_init(USART2_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ,USART2_TX_PIN);
    gpio_init(USART2_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, USART2_RX_PIN);
    // 3. config usart
    usart_deinit(USART2);
    usart_baudrate_set(USART2, baud);
    /*
        8-bit data
        1 stop bit
        No parity check
     */
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);
    usart_parity_config(USART2, USART_PM_NONE);
    // Enable sending and receiving
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    //  Enable reception interruption
    usart_interrupt_enable(USART2, USART_INT_RBNE);
    nvic_irq_enable(USART2_IRQn, 1, 1);
    usart_enable(USART2);
}

void USART2_SendByte(uint8_t data)
{
    while (RESET == usart_flag_get(USART2, USART_FLAG_TBE))
        ;
    usart_data_transmit(USART2, data);
}

void USART2_SendArray(uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        USART2_SendByte(data[i]);
    }
}

void USART2_SendString(char *str)
{
    while (*str)
    {
        USART2_SendByte(*str++);
    }
}

/*
    Get the length of received data
*/
uint16_t USART2_GetRxLength(void)
{
    return rx_count;
}

/*
    Read received data
*/
uint8_t USART2_GetRxData(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    if (rx_count < len)
    {
        return 0;
    }
    for (i = 0; i < len; i++)
    {
        buf[i] = rx_buf[i];
    }

    /*
        clear buffer
    */

    rx_count = 0;

    return 1;
}

void USART2_ClearRxBuffer(void)
{
    rx_count = 0;
}

void USART2_IRQHandler(void)
{
    uint8_t data;
    if (RESET != usart_interrupt_flag_get(USART2, USART_INT_FLAG_RBNE))
    {
        data = usart_data_receive(USART2);

        if (rx_count < USART2_RX_BUF_SIZE)
        {
            rx_buf[rx_count++] = data;
        }
    }
}
