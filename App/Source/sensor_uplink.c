#include "sensor_uplink.h"
#include "usart.h"
#include "system_config.h"
#include "systick.h"

/* Send control status */
static bool s_send_pending = FALSE;
static uint8_t s_seq = 0;
static uint32_t s_last_send_time = 0;

/* 最小传输间隔 (ms)，用于防止 UART 拥塞；单机时避免高频冗余上报 */
#define MIN_SEND_INTERVAL_MS 300u

/* Send buffer, placed in static area to avoid stack overflow (approximately 1KB+) */
static uint8_t s_tx_buf[SEN_MAX_FRAME_LEN];

/**
* @brief Calculate XOR checksum (CRC)
* @param buf Data buffer
* @param len Data length
* @return CRC value
 */
static uint8_t calc_crc(const uint8_t *buf, uint16_t len)
{
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= buf[i];
    }
    return crc;
}

/**
 * @brief Package and send the data to ESP32
 * @note Called by SENSOR_UPLINK_Poll(), do not call it directly from outside.
 */
static void SENSOR_UPLINK_SendData(void)
{
    uint32_t now_ms = GetTick();

    /* Interval protection: If the time has not elapsed, the flag will be retained and the next Poll attempt will be made. */
    if ((now_ms - s_last_send_time) < MIN_SEND_INTERVAL_MS)
    {
        return;
    }

    /* Statistical Online Server */
    uint8_t online_count = 0;
    for (int i = 0; i < MAX_SLAVE_ADDRESS; i++)
    {
        if (slave_list[i].online == SLAVE_ONLINE)
        {
            online_count++;
        }
    }

    /* Offline slave machine: Clear the flag, update the timestamp and then return */
    if (online_count == 0)
    {
        s_send_pending = FALSE;
        s_last_send_time = now_ms;
        return;
    }

    uint16_t index = 0;
    uint16_t data_len = 0;

    /* 1. HEAD */
    s_tx_buf[index++] = SEN_FRAME_HEAD;

    /* 2. Target address：ESP32 */
    s_tx_buf[index++] = SEN_ADDR_ESP32;

    /* 3. CMD：Sensor cmd */
    s_tx_buf[index++] = SEN_CMD_SENSOR_DATA;

    /* 4. Serialization */
    s_tx_buf[index++] = s_seq++;

    /* 5. Data length padding (big-endian) */
    uint16_t len_pos = index;
    index += 2;

    /* 6. Fill data: 7 bytes for each online slave device */
    for (int i = 0; i < MAX_SLAVE_ADDRESS; i++)
    {
        if (slave_list[i].online == SLAVE_ONLINE)
        {
            /* sensor_id */
            s_tx_buf[index++] = (uint8_t)(i + 1);

            /* timestamp: 4 bytes，little-endian */
            uint32_t ts = slave_list[i].timestamp; 
            s_tx_buf[index++] = (uint8_t)(ts >> 0);
            s_tx_buf[index++] = (uint8_t)(ts >> 8);
            s_tx_buf[index++] = (uint8_t)(ts >> 16);
            s_tx_buf[index++] = (uint8_t)(ts >> 24);

            /* trigger_count: 2 bytes，little-endian */
            uint16_t count = slave_list[i].trigger_count;
            s_tx_buf[index++] = (uint8_t)(count >> 0);
            s_tx_buf[index++] = (uint8_t)(count >> 8);

            data_len += SEN_ITEM_SIZE;
        }
    }

    /* 7. Data length for backfilling (big-endian) */
    s_tx_buf[len_pos] = (uint8_t)(data_len >> 8);
    s_tx_buf[len_pos + 1] = (uint8_t)(data_len >> 0);

    /* 8. CRC: From ADDR to DATA ends */
    uint8_t crc = calc_crc(&s_tx_buf[1], index - 1);
    s_tx_buf[index++] = crc;

    /* 9. Send data by USART2  */
    USART2_SendArray(s_tx_buf, index);

    /* 10. Clear the flag and record the sending time */
    s_send_pending = FALSE;
    s_last_send_time = now_ms;
}

/**
 * @brief Triggered sending (called by the RS485 host task after each round of polling)
 */
void SENSOR_UPLINK_TriggerSend(void)
{
    s_send_pending = TRUE;
}

/**
 * @brief Polling function, which is called periodically in the main loop
 */
void SENSOR_UPLINK_Poll(void)
{
    if (s_send_pending)
    {
        SENSOR_UPLINK_SendData();
    }
}

/**
 * @brief initialize the sensor uplink module
 */
void SENSOR_UPLINK_Init(void)
{
    s_send_pending = FALSE;
    s_seq = 0;
    s_last_send_time = 0;
}