#include "rs485_master_task.h"
#include "rs485.h"
#include "dip_switch.h"
#include "system_config.h"
#include "rs485_protocol.h"
#include "systick.h"

static Master_State state = MASTER_SEND;
static uint8_t current_addr = 1; // Poll the starting address of the slave device
static uint32_t wait_tick;       // Host sending and receiving tick status
static uint8_t retry_counter[SLAVE_COUNT] = {0};

static void NextDevice(void)
{
    current_addr++;
    if (current_addr > SLAVE_COUNT)
    {
        current_addr = 1;
    }
    state = MASTER_SEND;
}

void RS485_Master_Task(void)
{
    uint8_t rx_buf[RS485_RX_BUF_SIZE];
    switch (state)
    {
    case MASTER_SEND: // master send task
        RS485_Request(current_addr);
        wait_tick = GetTick();
        state = MASTER_WAIT;
        break;
    case MASTER_WAIT:
        if (RS485_FrameAvailable())
        {
            uint16_t len;
            len = RS485_Read(rx_buf);
            if (RS485_Parse_Status(rx_buf, len))
            {
                retry_counter[current_addr - 1] = 0;
                NextDevice();
            }
            else
            {
                // error frame
                NextDevice();
            }
        }
        else if (GetTimeElapsed(wait_tick) > RESPONSE_TIMEOUT_MS)
        {
            retry_counter[current_addr - 1]++;
            if (retry_counter[current_addr - 1] >= RETRY_COUNT)
            {
                slave_list[current_addr - 1].online = 0;
                retry_counter[current_addr - 1] = 0;
            }
            NextDevice();
        }
        break;
    default:
        break;
    }
}
