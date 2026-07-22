#ifndef __RS485_H
#define __RS485_H

#include "gd32f10x.h"

#define RS485_RX_BUF_SIZE 128

#define RS485_Baud 115200

#define RS485_FRAME_TIMEOUT_MS 5

/**
 * PA1:RE/DE
 * PA2:TX
 * PA3:RX
 */
#define RS485_PORT GPIOA
#define RS485_DE_PIN GPIO_PIN_1
#define RS485_TX_PIN GPIO_PIN_2
#define RS485_RX_PIN GPIO_PIN_3

typedef struct
{
    uint8_t rx_buf[RS485_RX_BUF_SIZE];
    uint16_t rx_count;
    uint8_t rx_flag;
    uint32_t rx_tick;
    uint8_t frame_ready;
} RS485_Handle;

/**
 * @brief initilize RS485
 *
 * @param baud
 */
void RS485_Init(uint32_t baud);

/**
 * @brief RS485 send a byte
 *
 * @param data
 */
void RS485_SendByte(uint16_t data);

/**
 * @brief RS485 send array
 *
 * @param data
 * @param len
 */
void RS485_SendArray(uint8_t *data, uint16_t len);

/**
 * @brief RS485 send string
 *
 * @param str
 */
void RS485_SendString(char *str);

/**
 * @brief RS485 available
 *
 */
uint8_t RS485_Available(void);

/**
 * @brief RS485 read buffer
 *
 * @param buf
 * @return uint16_t
 */
uint16_t RS485_Read(uint8_t *buf);


/**
 * @brief Periodic invocation in the task Determine the end of a frame
 *
 * @return * Periodic
 */
void RS485_Task(void);

/**
 * @brief Check the completeness of the frame count
 *
 * @return uint8_t
 */
uint8_t RS485_FrameAvailable(void);

#endif // ! __RS485_H
