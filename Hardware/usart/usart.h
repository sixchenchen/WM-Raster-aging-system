#ifndef __USART_H
#define __USART_H

#include "gd32f10x.h"

#define USART2_RX_BUF_SIZE 128

#define USART2_BAUD 115200
#define USART2_PORT GPIOB
#define USART2_RX_PIN GPIO_PIN_11
#define USART2_TX_PIN GPIO_PIN_10

void USART2_Init(void);

void USART2_SendByte(uint8_t data);

void USART2_SendArray(uint8_t *data, uint16_t len);

void USART2_SendString(char *str);

/*
    Get the length of received data
*/
uint16_t USART2_GetRxLength(void);

/*
    Read received data,读取接收数据
    return:
    1 success
    0 fail
*/
uint8_t USART2_GetRxData(uint8_t *buf, uint16_t len);

/*
    Clear the receiving buffer
*/
void USART2_ClearRxBuffer(void);

#endif
