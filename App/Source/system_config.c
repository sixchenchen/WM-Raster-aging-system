#include "system_config.h"
#include <string.h>
#include "dip_switch.h"
#include "sensor.h"

/*
    global configuration instance
*/
static System_Config sys_config;
Slave_Info slave_list[MAX_SLAVE_ADDRESS];

/*
    initialize system configuration
*/
void System_Config_Init(void)
{
    uint8_t mode;
    /*
        read address
    */
    memset(slave_list, 0, sizeof(slave_list));
    sys_config.address = DIP_Read_Address();
    /*
        read mode switch
        PA8 PA9 PA10
    */
    mode = DIP_Read_Mode();
    /*
        default
    */
    sys_config.board = BOARD_ERROR;
    sys_config.sensor_type = SENSOR_PNP;
    sys_config.sensor_mode = SENSOR_NO;
    switch (mode)
    {
    /*
        000
        MASTER
    */
    case 0:
        sys_config.board = BOARD_MASTER;
        break;
    /*
        001
        SLAVE
        PNP NO
    */
    case 1:
        sys_config.board = BOARD_SLAVE;
        sys_config.sensor_type = SENSOR_PNP;
        sys_config.sensor_mode = SENSOR_NO;
        break;
    /*
        010
        SLAVE
        PNP NC
    */
    case 2:
        sys_config.board = BOARD_SLAVE;
        sys_config.sensor_type = SENSOR_PNP;
        sys_config.sensor_mode = SENSOR_NC;
        break;
    /*
        011
        SLAVE
        NPN NO
    */
    case 3:
        sys_config.board = BOARD_SLAVE;
        sys_config.sensor_type = SENSOR_NPN;
        sys_config.sensor_mode = SENSOR_NO;
        break;

    /*
        100
        SLAVE
        NPN NC
    */
    case 4:
        sys_config.board = BOARD_SLAVE;
        sys_config.sensor_type = SENSOR_NPN;
        sys_config.sensor_mode = SENSOR_NC;
        break;
    default:
        sys_config.board = BOARD_ERROR;
        break;
    }
}

/*
    get current configuration
*/
const System_Config *System_Config_Get(void)
{
    return &sys_config;
}