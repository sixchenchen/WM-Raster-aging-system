#include "rs485.h"
#include "string.h"
#include "systick.h"

RS485_Handle rs485;

/**
 *  RS485 transimission mode
 */
static void RS485_TX_Mode(void)
{
    gpio_bit_set(RS485_PORT, RS485_DE_PIN);
}

/**
 * RS485 receiving mode
 */
static void RS485_RX_Mode(void)
{
    gpio_bit_reset(RS485_PORT, RS485_DE_PIN);
}

/**
 * initilize RS485
 */
void RS485_Init(void)
{
    // 1.configure the clock enable
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_USART1);
    // 2.configure pin mode
    gpio_init(RS485_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, RS485_DE_PIN);
    gpio_init(RS485_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, RS485_RX_PIN);
    gpio_init(RS485_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, RS485_TX_PIN);
    // 3.set RS485 mode
    RS485_RX_Mode();
    // 4.configure USART1
    usart_deinit(USART1);
    usart_baudrate_set(USART1, RS485_BAUD);
    usart_word_length_set(USART1, USART_WL_8BIT);
    usart_stop_bit_set(USART1, USART_STB_1BIT);
    usart_parity_config(USART1, USART_PM_NONE);
    usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART1, USART_RECEIVE_ENABLE);
    // 5.open reciveing interrupt
    usart_interrupt_enable(USART1, USART_INT_RBNE);
    nvic_irq_enable(USART1_IRQn, 1, 1);
    usart_enable(USART1);
}

/**
 * RS485 send byte
 */
void RS485_SendByte(uint8_t data)
{
    RS485_TX_Mode();
    delay_1ms(1);
    uint32_t timeout = 10000;
    while (RESET == usart_flag_get(USART1, USART_FLAG_TBE))
    {
        if (--timeout == 0)
        {
            RS485_RX_Mode();
            return;
        }
    }
    usart_data_transmit(USART1, data);

    timeout = 10000;
    while (RESET == usart_flag_get(USART1, USART_FLAG_TC))
    {
        if (--timeout == 0)
        {
            break;
        }
    }
    delay_1ms(1);
    RS485_RX_Mode();
    delay_1ms(1);
}
/**
 * RS485 send array
 */
void RS485_SendArray(uint8_t *data, uint16_t len)
{
    uint16_t i;
    
    if (len == 0 || data == NULL)
        return;
    
    RS485_TX_Mode();
    
    for (volatile uint32_t delay = 0; delay < 100; delay++);
    
    for (i = 0; i < len; i++)
    {
        uint32_t timeout = 10000;
        while (RESET == usart_flag_get(USART1, USART_FLAG_TBE))
        {
            if (--timeout == 0)
            {
                RS485_RX_Mode();
                return;
            }
        }
        usart_data_transmit(USART1, data[i]);
    }
    
    uint32_t timeout = 10000;
    while (RESET == usart_flag_get(USART1, USART_FLAG_TC))
    {
        if (--timeout == 0)
        {
            break;
        }
    }
    for (volatile uint32_t delay = 0; delay < 500; delay++);  // 约100us
    
    // 6. 切换回接收模式
    RS485_RX_Mode();
    for (volatile uint32_t delay = 0; delay < 50; delay++);
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
    uint16_t len = 0;
    
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    
    if (rs485.frame_ready && rs485.rx_count > 0)
    {
        len = rs485.rx_count;
        if (len > RS485_RX_BUF_SIZE)
            len = RS485_RX_BUF_SIZE;
        
        memcpy(buf, rs485.rx_buf, len);
        
        // 清空所有状态
        rs485.rx_count = 0;
        rs485.frame_ready = 0;
        rs485.rx_flag = 0;
    }
    
    __set_PRIMASK(primask);
    return len;
}

void RS485_Task(void)
{
    if (rs485.rx_count == 0)
        return;

    if (GetTimeElapsed(rs485.rx_tick) >= RS485_FRAME_TIMEOUT_MS)
    {
        rs485.frame_ready = 1;
    }
}
/* Check the completeness of the frame count */
uint8_t RS485_FrameAvailable(void)
{
    // 如果已经有帧就绪标志，直接返回
    if (rs485.frame_ready)
        return 1;
    
    // 没有数据
    if (rs485.rx_count == 0)
        return 0;
    
    // 检查超时
    if (GetTimeElapsed(rs485.rx_tick) >= RS485_FRAME_TIMEOUT_MS)
    {
        rs485.frame_ready = 1;
        return 1;
    }
    
    return 0;
}

/**
 * Receiving interrupt
 */
void USART1_IRQHandler(void)
{
    if (usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE))
    {
        uint16_t data;
        data = usart_data_receive(USART1);
        if (rs485.rx_count < RS485_RX_BUF_SIZE)
        {
            rs485.rx_buf[rs485.rx_count++] = (uint8_t)data;
            /* Each byte received Update time */
            rs485.rx_tick = GetTick();
            rs485.rx_flag = 1;
        }
    }
}

/*
    RS485 Echo Test
*/
void RS485_EchoTest(void)
{
    uint8_t rx_buffer[RS485_RX_BUF_SIZE];
    uint16_t rx_len;

    if (RS485_FrameAvailable())
    {
        rx_len = RS485_Read(rx_buffer);
        if (rx_len > 0)
        {
            // 发送收到的数据
            RS485_SendArray(rx_buffer, rx_len);
        }
    }
}
void RS485_ClearBuffer(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    rs485.rx_count = 0;
    rs485.frame_ready = 0;
    rs485.rx_flag = 0;
    __set_PRIMASK(primask);
}