#include "sensor_uplink.h"
#include "usart.h"
#include "system_config.h"
#include "systick.h"

/* 发送控制状态 */
static bool s_send_pending = FALSE;
static uint8_t s_seq = 0;
static uint32_t s_last_send_time = 0;

/* 最小发送间隔 (ms)，防止 UART 拥塞 */
#define MIN_SEND_INTERVAL_MS 100u

/* 发送缓冲区，放静态区避免栈溢出 (约 1KB+) */
static uint8_t s_tx_buf[SEN_MAX_FRAME_LEN];

/**
 * @brief 计算异或校验 (CRC)
 * @param buf 数据缓冲区
 * @param len 数据长度
 * @return CRC 值
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
 * @brief 打包并发送数据给 ESP32
 * @note 由 SENSOR_UPLINK_Poll() 调用，外部不要直接调用
 */
static void SENSOR_UPLINK_SendData(void)
{
    uint32_t now_ms = GetTick();

    
    /* 间隔保护：时间未到则保留标志，下次 Poll 再试 */
    if ((now_ms - s_last_send_time) < MIN_SEND_INTERVAL_MS)
    {
        return;
    }

    /* 统计在线从机 */
    uint8_t online_count = 0;
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        if (slave_list[i].online == SLAVE_ONLINE)
        {
            online_count++;
        }
    }

    /* 无在线从机：清标志、更新时间戳后返回 */
    if (online_count == 0)
    {
        s_send_pending = FALSE;
        s_last_send_time = now_ms;
        return;
    }

    uint16_t index = 0;
    uint16_t data_len = 0;

    /* 1. 帧头 */
    s_tx_buf[index++] = SEN_FRAME_HEAD;

    /* 2. 目标地址：ESP32 */
    s_tx_buf[index++] = SEN_ADDR_ESP32;

    /* 3. 命令：传感器数据 */
    s_tx_buf[index++] = SEN_CMD_SENSOR_DATA;

    /* 4. 序列号 */
    s_tx_buf[index++] = s_seq++;

    /* 5. 数据长度占位 (大端) */
    uint16_t len_pos = index;
    index += 2;

    /* 6. 填充数据：每个在线从机 7 字节 */
    for (int i = 0; i < SLAVE_COUNT; i++)
    {
        if (slave_list[i].online == SLAVE_ONLINE)
        {
            /* sensor_id */
            s_tx_buf[index++] = (uint8_t)(i + 1);

            /* timestamp: 4 字节，小端 */
            uint32_t ts = now_ms;
            s_tx_buf[index++] = (uint8_t)(ts >> 0);
            s_tx_buf[index++] = (uint8_t)(ts >> 8);
            s_tx_buf[index++] = (uint8_t)(ts >> 16);
            s_tx_buf[index++] = (uint8_t)(ts >> 24);

            /* trigger_count: 2 字节，小端 */
            uint16_t count = slave_list[i].trigger_count;
            s_tx_buf[index++] = (uint8_t)(count >> 0);
            s_tx_buf[index++] = (uint8_t)(count >> 8);

            data_len += SEN_ITEM_SIZE;
        }
    }

    /* 7. 回填数据长度 (大端) */
    s_tx_buf[len_pos] = (uint8_t)(data_len >> 8);
    s_tx_buf[len_pos + 1] = (uint8_t)(data_len >> 0);

    /* 8. CRC：从 ADDR 到 DATA 结束 */
    uint8_t crc = calc_crc(&s_tx_buf[1], index - 1);
    s_tx_buf[index++] = crc;

    /* 9. 通过 USART2 发送 */
    USART2_SendArray(s_tx_buf, index);

    /* 清标志、记录发送时间 */
    s_send_pending = FALSE;
    s_last_send_time = now_ms;
}

/**
 * @brief 触发发送（由 RS485 主机任务在一轮轮询结束后调用）
 */
void SENSOR_UPLINK_TriggerSend(void)
{
    s_send_pending = TRUE;
}

/**
 * @brief 轮询函数，在主循环中周期性调用
 */
void SENSOR_UPLINK_Poll(void)
{
    if (s_send_pending)
    {
        SENSOR_UPLINK_SendData();
    }
}

/**
 * @brief 初始化
 */
void SENSOR_UPLINK_Init(void)
{
    s_send_pending = FALSE;
    s_seq = 0;
    s_last_send_time = 0;
}