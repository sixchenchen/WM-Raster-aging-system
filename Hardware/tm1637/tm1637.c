#include "tm1637.h"
#include <string.h>

// 共阴极段码表（注意：你的数码管如果是共阳极，需要取反或用共阳段码表）
const uint8_t num_tab[] = {
    // 0,    1,    2,    3,    4,    5,    6,    7,    8,    9,    A,    b,    C,    d,    E,    F,   :(冒号)
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
    0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x80};

// 显示缓冲区，从左到右
uint8_t show_buffer[4] = {0};

/**
 * @brief TM1637 初始化
 */
void tm1637_init(void)
{

    rcu_periph_clock_enable(RCU_GPIOB);

    gpio_init(
        GPIOB,
        GPIO_MODE_OUT_PP,
        GPIO_OSPEED_50MHZ,
        GPIO_PIN_6);

    gpio_init(
        GPIOB,
        GPIO_MODE_OUT_OD,
        GPIO_OSPEED_50MHZ,
        GPIO_PIN_7);

    TM1637_CLK_HIGH;
    TM1637_DIO_HIGH;
}
/**
 * @brief 粗略延时
 * @param i 延时计数
 */
void delay_ns(uint32_t i)
{
    while (i--)
    {
        __NOP();
    }
}

/**
 * @brief TM1637 起始信号
 */
void tm1637_start(void)
{
    TM1637_DIO_GPIO_OUTPUT;
    TM1637_CLK_HIGH;
    TM1637_DIO_HIGH;
    delay_ns(6);
    TM1637_DIO_LOW;
}

/**
 * @brief 检测应答信号
 * @return 0: NACK, 1: ACK
 */
uint8_t tm1637_is_ack(void)
{
    uint8_t ack_flag = 0;
    uint8_t time = 60;

    // DIO 切换为输入
    TM1637_DIO_GPIO_INPUT;

    TM1637_CLK_LOW;

    while (time)
    {
        time--;
        if (RESET == gpio_input_bit_get(TM1637_DIO_GPIO_PORT, TM1637_DIO_GPIO_PIN))
        {
            ack_flag = 1;
            break;
        }
    }

    TM1637_CLK_HIGH;
    delay_ns(6);
    TM1637_CLK_LOW;

    // DIO 恢复输出
    TM1637_DIO_GPIO_OUTPUT;

    return ack_flag;
}

/**
 * @brief TM1637 停止信号
 */
void tm1637_stop(void)
{
    TM1637_DIO_GPIO_OUTPUT;
    TM1637_CLK_LOW;
    delay_ns(5);
    TM1637_DIO_LOW;
    delay_ns(5);
    TM1637_CLK_HIGH;
    delay_ns(5);
    TM1637_DIO_HIGH;
}

/**
 * @brief 写一个字节（地址自动递增模式）
 * @param data 要发送的数据
 */
void tm1637_write_byte(uint8_t data)
{
    uint8_t i;

    TM1637_DIO_GPIO_OUTPUT;

    for (i = 0; i < 8; i++)
    {
        TM1637_CLK_LOW;

        if (data & 0x01)
        {
            TM1637_DIO_HIGH;
        }
        else
        {
            TM1637_DIO_LOW;
        }

        delay_ns(3);
        data = data >> 1;
        TM1637_CLK_HIGH;
        delay_ns(3);
    }
}

/**
 * @brief 数码管显示数值
 * @param show_num 显示数值 0000~9999
 * @param colon_flag 冒号标志位，0:不显示，1:显示
 */
void smg_display(uint16_t show_num, uint8_t colon_flag)
{
    uint8_t i;
    uint8_t ack_flag = 0;

    memset(show_buffer, 0, sizeof(show_buffer));

    show_buffer[0] = num_tab[show_num / 1000];

    if (colon_flag)
    {
        show_buffer[1] = num_tab[show_num / 100 % 10] | 0x80;
    }
    else
    {
        show_buffer[1] = num_tab[show_num / 100 % 10];
    }

    show_buffer[2] = num_tab[show_num / 10 % 10];
    show_buffer[3] = num_tab[show_num % 10];

    // 发送数据命令：地址自动加1模式
    tm1637_start();
    tm1637_write_byte(0x40);
    ack_flag = tm1637_is_ack();
    if (!ack_flag)
    {
        return;
    }
    tm1637_stop();

    // 设置显示地址并写入数据
    tm1637_start();
    tm1637_write_byte(0xC0);
    ack_flag = tm1637_is_ack();
    if (!ack_flag)
    {
        return;
    }

    for (i = 0; i < 4; i++)
    {
        tm1637_write_byte(show_buffer[i]);
        ack_flag = tm1637_is_ack();
        if (!ack_flag)
        {
            return;
        }
    }
    tm1637_stop();

    // 开显示，设置亮度
    tm1637_start();
    tm1637_write_byte(0x8B); // 开显示，亮度 10/16
    ack_flag = tm1637_is_ack();
    if (!ack_flag)
    {
        return;
    }
    tm1637_stop();
}
