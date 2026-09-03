#ifndef __RS485_PROTOCOL_H
#define __RS485_PROTOCOL_H

#include "gd32f10x.h"

#define FRAME_HEAD 0xAA

// master request
#define CMD_GET_STATUS 0x01

// slave reply
#define CMD_STATUS_REPLY 0x81

#define CMD_CLEAR_COUNT 0x02

#define CMD_CLEAR_REPLY 0x82

#define DATA_LENGTH 0x03

uint8_t RS485_CalcCRC(uint8_t *buf, uint16_t len);

void RS485_Request(uint8_t addr);

void RS485_SendStatus(uint8_t addr, uint16_t trigger_count);

uint8_t RS485_Parse_Status(uint8_t *buf, uint16_t len);

uint8_t RS485_CheckFrame(uint8_t *buf, uint16_t len);

#endif
