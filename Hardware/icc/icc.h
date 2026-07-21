#ifndef __IIC_H
#define __IIC_H

#include "gd32f10x.h"

#define IIC_PORT GPIOB

#define IIC_SCL_PIN GPIO_PIN_10
#define IIC_SDA_PIN GPIO_PIN_11

void IIC_Init(void);

void IIC_Start(void);

void IIC_Stop(void);

void IIC_SendByte(uint8_t data);

uint8_t IIC_ReadByte(uint8_t ack);

uint8_t IIC_WaitAck(void);

void IIC_SendAck(void);

void IIC_SendNack(void);

#endif
