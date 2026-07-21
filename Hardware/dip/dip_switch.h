#ifndef __DIP_SWITCH_H
#define __DIP_SWITCH_H

#include "gd32f10x.h"

/**
 * @brief  board type
 * @param  None
 * @retval None
 */
typedef enum
{
    BOARD_SLAVE = 0,
    BOARD_MASTER = 1,
    BOARD_ERROR = 2,
} Board_Type;

/**
 * @brief sensor type
 *
 */
typedef enum
{
    SENSOR_NPN = 0,
    SENSOR_PNP = 1,
} Sensor_Type;

/**
 * @brief sensor mode
 *
 */
typedef enum
{
    SENSOR_NO = 0,
    SENSOR_NC = 1,
} Sensor_Mode;

/**
 * @brief dip switch config type
 *
 */
typedef struct
{
    Board_Type board;
    Sensor_Type sensor_type;
    Sensor_Mode sensor_mode;
    uint8_t address;
} Dip_Config;

/**
 * @brief Init dip switch
 *
 */
void Dip_Switch_Init(void);

/**
 * @brief dip switch address
 *
 */
uint8_t DIP_Read_Address(void);

/**
 * @brief dip switch config
 *
 * @return Dip_Config
 */
Dip_Config DIP_Read_Config(void);

#endif
