#ifndef __TM1637_H
#define __TM1637_H

#include "gd32f10x.h"

#define TM1637_CLK_GPIO_PORT GPIOB
#define TM1637_CLK_GPIO_PIN GPIO_PIN_6
#define TM1637_CLK_GPIO_CLK_ENABLE() rcu_periph_clock_enable(RCU_GPIOB)
#define TM1637_CLK_HIGH gpio_bit_set(TM1637_CLK_GPIO_PORT, TM1637_CLK_GPIO_PIN)
#define TM1637_CLK_LOW gpio_bit_reset(TM1637_CLK_GPIO_PORT, TM1637_CLK_GPIO_PIN)

#define TM1637_DIO_GPIO_PORT GPIOB
#define TM1637_DIO_GPIO_PIN GPIO_PIN_7
#define TM1637_DIO_GPIO_CLK_ENABLE() rcu_periph_clock_enable(RCU_GPIOB)
#define TM1637_DIO_HIGH gpio_bit_set(TM1637_DIO_GPIO_PORT, TM1637_DIO_GPIO_PIN)
#define TM1637_DIO_LOW gpio_bit_reset(TM1637_DIO_GPIO_PORT, TM1637_DIO_GPIO_PIN)

// 切换 DIO 方向：输出（推挽）/ 输入（浮空）
#define TM1637_DIO_GPIO_OUTPUT gpio_init(TM1637_DIO_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, TM1637_DIO_GPIO_PIN)
#define TM1637_DIO_GPIO_INPUT gpio_init(TM1637_DIO_GPIO_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, TM1637_DIO_GPIO_PIN)

/**
 * @brief TM1637 初始化
 */
void tm1637_init(void);

/**
 * @brief 粗略延时
 * @param i 延时计数
 */
void delay_ns(uint32_t i);

/**
 * @brief TM1637 起始信号
 */
void tm1637_start(void);

/**
 * @brief 检测应答信号
 * @return 0: NACK, 1: ACK
 */
uint8_t tm1637_is_ack(void);

/**
 * @brief TM1637 停止信号
 */
void tm1637_stop(void);

/**
 * @brief 写一个字节（地址自动递增模式）
 * @param data 要发送的数据
 */
void tm1637_write_byte(uint8_t data);

/**
 * @brief 数码管显示数值
 * @param show_num 显示数值 0000~9999
 * @param colon_flag 冒号标志位，0:不显示，1:显示
 */
void smg_display(uint16_t show_num, uint8_t colon_flag);

#endif /* __TM1637_H */
