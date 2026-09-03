#ifndef __SENSOR_UPLINK_H__
#define __SENSOR_UPLINK_H__

#include "gd32f10x.h"

/* SEN 协议定义 */
#define SEN_FRAME_HEAD 0x55u
#define SEN_ADDR_ESP32 0x01u
#define SEN_ADDR_GD32 0x02u
#define SEN_CMD_SENSOR_DATA 0x01u
#define SEN_CMD_ACK 0x20u
#define SEN_CMD_SYNC_REQ 0x60u
#define SEN_CMD_SYNC_ACK 0x61u

/* 每个传感器数据项: sensor_id(1) + timestamp(4) + count(2) = 7字节 */
#define SEN_ITEM_SIZE 7u
#define SEN_MAX_DATA_LEN 1024u
#define SEN_MAX_FRAME_LEN (1u + 1u + 1u + 1u + 2u + SEN_MAX_DATA_LEN + 1u)

/* 对外接口 */
void SENSOR_UPLINK_Init(void);
void SENSOR_UPLINK_Poll(void);
void SENSOR_UPLINK_TriggerSend(void);

#endif /* __SENSOR_UPLINK_H__ */