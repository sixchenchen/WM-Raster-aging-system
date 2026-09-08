#include "rs485_slave_task.h"
#include "rs485.h"
#include "dip_switch.h"
#include "system_config.h"
#include "rs485_protocol.h"
#include "setting.h"

/*
    slave task:send current status
*/

void RS485_Slave_Task(void)
{
    const System_Config *config;
    config = System_Config_Get();

    if (!RS485_FrameAvailable())
        return;
    uint16_t len;
    // 本协议最大帧长12字节，16字节足够，避免占用大块栈空间
    uint8_t buf[16];
    len = RS485_Read(buf);
    if (!RS485_CheckFrame(buf, len))
        return;
    if (buf[1] != config->address)
        return;
    switch (buf[2])
    {
    case CMD_GET_STATUS:
        RS485_SendStatus(config->address, Setting_Get_Value());
        break;
    case CMD_CLEAR_COUNT: // Equivalent to triggering the long press duration
        Setting_Reset_Long();
        break;
    default:
        break;
    }
}
