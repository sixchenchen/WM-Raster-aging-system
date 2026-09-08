#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

#include "gd32f10x.h"

#define MAX_SLAVE_ADDRESS 31
#define SLAVE_OFFLINE 0
#define SLAVE_ONLINE 1

/*
    Board type
*/
typedef enum
{
    BOARD_MASTER = 0,
    BOARD_SLAVE,
    BOARD_ERROR
} Board_Type;

/*
    Sensor type
*/
typedef enum
{
    SENSOR_PNP = 0,
    SENSOR_NPN
} Sensor_Type;

/*
    Sensor mode
*/
typedef enum
{
    SENSOR_NO = 0, // normally open
    SENSOR_NC      // normally close
} Sensor_Mode;

/*
    system configuration
*/
typedef struct
{
    /*
        board type
        master/slave
    */
    Board_Type board;
    /*
        RS485 address
        0~31
    */
    uint8_t address;
    /*
        sensor output type
        PNP/NPN
    */
    Sensor_Type sensor_type;
    /*
        sensor contact mode
        NO/NC
    */
    Sensor_Mode sensor_mode;
} System_Config;

typedef struct
{
    uint8_t online;
    uint8_t discovered;
    uint16_t trigger_count;
    uint8_t sensor_status;
    uint32_t last_time;
    uint32_t last_trigger_time;
    uint32_t timestamp;
} Slave_Info;

/*
initialize system config
*/
void System_Config_Init(void);

/*
get config pointer
*/
const System_Config *System_Config_Get(void);
extern Slave_Info slave_list[MAX_SLAVE_ADDRESS];

#endif