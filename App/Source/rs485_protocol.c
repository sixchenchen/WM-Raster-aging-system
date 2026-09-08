#include "rs485_protocol.h"
#include "rs485_master_task.h"
#include "rs485.h"
#include "dip_switch.h"
#include "system_config.h"
#include "sensor.h"
#include "systick.h"

/*
    crc = addr ^ cmd ^ len;
*/
uint8_t RS485_CalcCRC(uint8_t *buf, uint16_t len)
{
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= buf[i];
    }
    return crc;
}

void RS485_Request(uint8_t addr)
{
    uint8_t data[5];
    data[0] = FRAME_HEAD;
    data[1] = addr;
    data[2] = CMD_GET_STATUS;
    data[3] = 0; // Data bit length
    data[4] = RS485_CalcCRC(&data[1], 3);
    RS485_SendArray(data, 5);
}

/*
    DATA: HEAD(1)+ADDR(1)+CMD(1)+LEN(1)+DATA(7)+CRC(1)=12
 */
void RS485_SendStatus(uint8_t addr, uint16_t trigger_count)
{
    uint8_t data[FRAME_LENGTH];     // Expand the buffer
    uint32_t timestamp = GetTick(); // Get the current system time
    data[0] = FRAME_HEAD;
    data[1] = addr;
    data[2] = CMD_STATUS_REPLY;
    data[3] = DATA_LENGTH; // the data length to 0x07

    // trigger_count (2 byte)
    data[4] = (uint8_t)(trigger_count >> 0);
    data[5] = (uint8_t)(trigger_count >> 8);

    // sensor_status (1 byte)
    data[6] = SENSOR_NORMAL;

    // Timestamp (4 bytes, little-endian format)
    data[7] = (uint8_t)(timestamp >> 0); // Lowest byte
    data[8] = (uint8_t)(timestamp >> 8);
    data[9] = (uint8_t)(timestamp >> 16);
    data[10] = (uint8_t)(timestamp >> 24); // Highest byte

    // CRC calculation: from ADDR to DATA end (address+command+data length+data)
    data[11] = RS485_CalcCRC(&data[1], 3 + DATA_LENGTH); // 3 + 7 = 10 bytes

    RS485_SendArray(data, FRAME_LENGTH); // Send FRAME_LENGTH bytes
}

/*
    485 Parse String
    buf:
    buf[0]:Header
    buf[1]:Address
    buf[2]:Command Length
    buf[4~Lenght]:Data
    buf[Lenght+1]:CRC
*/
uint8_t RS485_Parse_Status(uint8_t *buf, uint16_t len, uint8_t expected_addr)
{
    uint8_t addr;
    uint8_t datalen;
    uint8_t crc;
    uint16_t k;
    Slave_Info *slave;
    if (len < FRAME_LENGTH)
        return 0;

    /*
        主站自身发送的请求帧会经RS485半双工总线回读进本机RX，
        收到的buf可能是"主站请求回声 + 从机应答"拼接而成。
        因此不能假设应答从buf[0]开始，需在缓冲区内逐位搜寻真正的
        应答帧前缀(HEAD+ADDR+CMD_STATUS_REPLY)，避免解析失败导致
        从机一直被判定离线、主站陷入无限Discovery、上行不触发。
    */
    for (k = 0; k + FRAME_LENGTH <= len; k++)
    {
        if (buf[k] != FRAME_HEAD)
            continue;
        addr = buf[k + 1];
        if (addr != expected_addr)
            continue;
        if (addr < 1 || addr > MAX_SLAVE_ADDRESS)
            continue;
        if (buf[k + 2] != CMD_STATUS_REPLY)
            continue;
        datalen = buf[k + 3];
        if ((len - k) != datalen + 5)
            continue;
        crc = RS485_CalcCRC(&buf[k + 1], 3 + datalen);
        if (crc != buf[len - 1])
            continue;

        slave = &slave_list[addr - 1];
        slave->online = SLAVE_ONLINE;
        slave->discovered = 1;
        slave->trigger_count = ((uint16_t)buf[k + 4]) | ((uint16_t)buf[k + 5] << 8);
        slave->sensor_status = buf[k + 6];
        slave->timestamp = ((uint32_t)buf[k + 7] << 0) |
                           ((uint32_t)buf[k + 8] << 8) |
                           ((uint32_t)buf[k + 9] << 16) |
                           ((uint32_t)buf[k + 10] << 24);
        slave->last_time = GetTick();
        return 1;
    }
    return 0;
}

uint8_t RS485_CheckFrame(uint8_t *buf, uint16_t len)
{
    /*
        min frame : AA + ADDRESS + CMD + LEN（LEN = 0 + CRC）+ CRC = 5
        frame :
    */
    if (len < 5)
    {
        return 0;
    }
    /*
        cheak frame
    */
    if (buf[0] != FRAME_HEAD)
    {
        return 0;
    }
    /*
        Obtain the data length
        buf[3]
    */
    uint8_t data_len;
    data_len = buf[3];
    /*
        Judge the actual length
        HEAD ADDR CMD LEN DATA CRC = LEN+5
    */
    if (len != (data_len + 5))
    {
        return 0;
    }
    /*
        crc = crc = addr ^ cmd ^ len;
    */
    uint8_t crc;
    crc = RS485_CalcCRC(&buf[1], data_len + 3);
    if (crc != buf[len - 1])
    {
        return 0;
    }
    return 1;
}
