#include "rs485_slave_task.h"
#include "rs485.h"
#include "dip_switch.h"
#include "system_config.h"
#include "rs485_protocol.h"

/*
    slave task:send current status
*/
void RS485_Slave_Task(void)
{
    if (!RS485_FrameAvailable())
        return;
    uint16_t len;
    uint8_t buf[RS485_RX_BUF_SIZE];
    len = RS485_Read(buf);
    if (!RS485_CheckFrame(buf, len))
        return;
    if (buf[1] != g_config.address)
        return;
    switch (buf[2])
    {
    case CMD_GET_STATUS:
        RS485_SendStatus(g_config.address, Sensor_GetTriggerCount());
        break;
    case CMD_CLEAR_COUNT:
        Sensor_ClearCount();
        break;
    default:
        break;
    }
}
