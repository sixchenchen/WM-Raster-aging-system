#include "rs485_protocol.h"
#include "rs485_master_task.h"
#include "rs485.h"
#include "dip_switch.h"
#include "system_config.h"
#include "sensor.h"

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

void RS485_SendStatus(uint8_t addr, uint16_t trigger_count)
{
    uint8_t data[8];
    data[0] = FRAME_HEAD;
    data[1] = addr;
    data[2] = CMD_STATUS_REPLY;
    data[3] = DATA_LENGTH;                 // data bit length
    data[4] = (trigger_count >> 8) & 0xff; // hight 8 bit
    data[5] = trigger_count & 0xff;        // low 8 bit
    data[6] = SENSOR_NORMAL;               // TODO detecting update of the grating
    data[7] = RS485_CalcCRC(&data[1], 6);
    RS485_SendArray(data, 8);
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
uint8_t RS485_Parse_Status(uint8_t *buf, uint16_t len)
{
    if (len < 8)
        return 0;
    if (buf[0] != FRAME_HEAD)
        return 0;
    uint8_t addr = buf[1];
    if (buf[2] != CMD_STATUS_REPLY)
        return 0;
    uint8_t datalen = buf[3];
    if (len != datalen + 5)
        return 0;
    uint8_t crc;
    crc = RS485_CalcCRC(&buf[1], 3 + datalen);
    if (crc != buf[len - 1])
        return 0;
    if (addr < 1 || addr > SLAVE_COUNT)
        return 0;
    Slave_Info *slave;
    slave = &slave_list[addr - 1];
    slave->online = SLAVE_ONLINE;
    slave->trigger_count = ((uint16_t)buf[4] << 8) | buf[5];
    slave->sensor_status = buf[6];
    return 1;
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
