#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

#include "dip_switch.h"
#include "stdint.h"
#include "sensor.h"
#define MAX_SLAVE_NUM 16 // slave number 16

/* slave information */
typedef struct
{
    uint8_t address;             // address
    uint8_t online;              // online status
    uint16_t trigger_count;      // trigger count
    Sensor_Status sensor_status; // sensor status
} Slave_Info;

extern Slave_Info slave_list[MAX_SLAVE_NUM];

extern Dip_Config g_config;

#endif
