#include "rs485_master_task.h"
#include "rs485.h"
#include "dip_switch.h"
#include "system_config.h"
#include "rs485_protocol.h"
#include "systick.h"
#include "sensor_uplink.h"
#include "led.h"

static Master_State state = MASTER_DISCOVERY_SEND;
static uint8_t current_addr = 1; // Poll the starting address of the slave device
static uint32_t wait_tick;       // Host sending and receiving tick status
static uint8_t retry_counter[MAX_SLAVE_ADDRESS] = {0};
static uint8_t active_slave_list[MAX_SLAVE_ADDRESS]; // 当前实际参与正常轮询的从机地址
static uint8_t active_slave_count = 0;               //  当前实际发现的从机数量
static uint8_t poll_index = 0;                       // 当前轮询到 active_slave_list[] 的哪个位置
static uint8_t discovery_addr = 1;                   // 当前 Discovery 扫描地址
static uint32_t last_discovery_time = 0;             // 上一次 Discovery 完成/开始的时间

/*
    开始一轮全新扫描：
    清空所有从机的在线标记，并从地址1重新探测。
    保证 Discovery 结束后重建的活跃列表反映本轮真实在机情况，
    避免上一轮残留的 online 标志把掉线设备误留在列表中。
*/
static void BeginDiscoverySweep(void)
{
    uint8_t i;
    for (i = 0; i < MAX_SLAVE_ADDRESS; i++)
    {
        slave_list[i].online = SLAVE_OFFLINE;
    }
    discovery_addr = 1;
    state = MASTER_DISCOVERY_SEND;
}

/*
    根据 slave_list[].discovered,重新建立正常轮询列表
*/
static void RebuildActiveSlaveList(void)
{
    active_slave_count = 0;
    //  扫描整个地址空间 1 ~ 31
    for (uint8_t addr = 1; addr <= MAX_SLAVE_ADDRESS; addr++)
    {
        // 如果该地址已经发现过设备
        if (slave_list[addr - 1].online == SLAVE_ONLINE)
        {
            active_slave_list[active_slave_count] = addr;
            active_slave_count++;
            if (active_slave_count >= MAX_SLAVE_ADDRESS)
            {
                break;
            }
        }
    }
    //  防止 poll_index 越界
    if (active_slave_count == 0)
    {
        poll_index = 0;
    }
    else if (poll_index >= active_slave_count)
    {
        poll_index = 0;
    }
}

static void RemoveOfflineSlave(uint8_t addr)
{
    uint8_t i;
    for (i = 0; i < active_slave_count; i++)
    {
        if (active_slave_list[i] == addr)
        {
            /*
             * 后面的地址向前移动
             */
            for (uint8_t j = i; j < active_slave_count - 1; j++)
            {
                active_slave_list[j] = active_slave_list[j + 1];
            }
            active_slave_count--;
            /*
             * 防止 poll_index 越界
             */
            if (active_slave_count == 0)
            {
                poll_index = 0;
            }
            else if (poll_index >= active_slave_count)
            {
                poll_index = 0;
            }
            return;
        }
    }
}

static void NextPollDevice(void)
{
    //  当前没有发现任何设备
    if (active_slave_count == 0)
    {
        // 没有设备，进入一重新扫描，从地址1重新探测
        BeginDiscoverySweep();
        return;
    }
    //  进入下一个设备
    poll_index++;
    //  当前这一轮所有设备已经轮询完成
    if (poll_index >= active_slave_count)
    {
        // 回到第一个设备
        poll_index = 0;
        // 一轮轮询结束，通知 ESP32 上传数据
        SENSOR_UPLINK_TriggerSend();
    }
    // 获取下一个实际设备地址
    current_addr = active_slave_list[poll_index];
    // 进入正常发送
    state = MASTER_SEND;
}

static void NextDiscoveryAddress(void)
{
    // 扫描下一个地址
    discovery_addr++;
    // 1~31 全部扫描完成
    if (discovery_addr > MAX_SLAVE_ADDRESS)
    {
        //  回到地址1
        discovery_addr = 1;
        //  根据发现结果重新建立，正常轮询设备列表
        RebuildActiveSlaveList();
        // 有设备
        if (active_slave_count > 0)
        {
            // 从第一个设备开始轮询
            poll_index = 0;
            current_addr = active_slave_list[0];
            state = MASTER_SEND;
        }
        else
        {
            //  一个设备都没有， 继续 Discovery
            state = MASTER_DISCOVERY_SEND;
        }
        return;
    }
    // 继续 Discovery
    state = MASTER_DISCOVERY_SEND;
}

void RS485_Master_Task(void)
{
    // RS485接收缓冲区：本协议应答帧12字节，另有主站自身5字节回声前缀可能拼接成帧，预留至24字节，避免回声+应答拼接时溢出栈缓冲。
    uint8_t rx_buf[24];
    switch (state)
    {
    case MASTER_SEND:
        // 检查是否到了Discovery时间，例如每5秒重新扫描一次
        if (GetTimeElapsed(last_discovery_time) >= DISCOVERY_INTERVAL_MS)
        {
            // 更新Discovery时间
            last_discovery_time = GetTick();
            // 从地址1开始全新扫描
            BeginDiscoverySweep();
            break;
        }
        // 没有任何实际设备
        if (active_slave_count == 0)
        {
            // 进入一重新扫描，从地址1重新探测
            BeginDiscoverySweep();
            break;
        }
        // 当前设备地址从active_slave_list获取
        current_addr = active_slave_list[poll_index];
        // 发送状态请求
        RS485_Request(current_addr);
        // 保存发送时间
        wait_tick = GetTick();
        // 进入等待
        state = MASTER_WAIT;
        break;
    case MASTER_WAIT:
        // 收到RS485数据
        if (RS485_FrameAvailable())
        {
            uint16_t len;
            // 读取数据
            len = RS485_Read(rx_buf);
            // 解析数据，expected_addr必须等于当前询问地址
            if (RS485_Parse_Status(rx_buf, len, current_addr))
            {
                // 通信成功，清除重试次数
                retry_counter[current_addr - 1] = 0;
                // 设置在线
                slave_list[current_addr - 1].online = SLAVE_ONLINE;
                // 设置已经发现
                slave_list[current_addr - 1].discovered = 1;
                // 更新最近通信时间
                slave_list[current_addr - 1].last_time = GetTick();
            }
            else
            {
                // 收到错误数据
                retry_counter[current_addr - 1]++;
            }
            // 继续下一个设备
            NextPollDevice();
        }
        // 等待超时
        else if (GetTimeElapsed(wait_tick) > RESPONSE_TIMEOUT_MS)
        {
            // 当前设备通信失败
            retry_counter[current_addr - 1]++;
            // 连续失败3次
            if (retry_counter[current_addr - 1] >= RETRY_COUNT)
            {
                // 判定设备离线
                slave_list[current_addr - 1].online = SLAVE_OFFLINE;
                // 清除重试次数
                retry_counter[current_addr - 1] = 0;
                // 清除触发次数（设备下线上线后数据是清零状态）
                slave_list[current_addr - 1].trigger_count = 0;
                RemoveOfflineSlave(current_addr);
            }
            // 继续下一个设备
            NextPollDevice();
        }
        break;
    case MASTER_DISCOVERY_SEND:
        // 请求当前Discovery地址
        RS485_Request(discovery_addr);
        // 保存发送时间
        wait_tick = GetTick();
        // 等待回复
        state = MASTER_DISCOVERY_WAIT;
        break;
    case MASTER_DISCOVERY_WAIT:
        // 收到数据
        if (RS485_FrameAvailable())
        {
            uint16_t len;
            // 读取数据
            len = RS485_Read(rx_buf);
            // 检查回复是否来自当前正在扫描的地址
            if (RS485_Parse_Status(rx_buf, len, discovery_addr))
            {
                // 确认该地址存在从机
                slave_list[discovery_addr - 1].discovered = 1;
                // 设置在线
                slave_list[discovery_addr - 1].online = SLAVE_ONLINE;
                // 清除重试次数
                retry_counter[discovery_addr - 1] = 0;
                // 更新时间
                slave_list[discovery_addr - 1].last_time = GetTick();
                // 扫描下一个地址
                NextDiscoveryAddress();
            }
            /*
                帧内容与当前扫描地址不匹配（上一个地址迟到的应答/残留帧）：
                只消费掉该帧，不推进扫描位，
                继续等待当前地址的真正应答或超时，避免设备被误跳过。
            */
        }
        // 当前地址没有响应
        else
        {
            /*
                时间基准：用 persisted 的 discovered 标志区分"已知设备"与"未知空地址"。
                - 曾回复过的设备（discovered=1）：给足 RESPONSE_TIMEOUT_MS，避免慢设备被误判为空。
                - 从未见过的空地址（discovered=0）：用 DISCOVERY_TIMEOUT_FAST_MS 快速判空跳过，
                  大幅缩短全量重扫的停摆时间（28 个空址 × 80ms → × 20ms）。
            */
            uint32_t discovery_to = slave_list[discovery_addr - 1].discovered ? (uint32_t)RESPONSE_TIMEOUT_MS : (uint32_t)DISCOVERY_TIMEOUT_FAST_MS;
            if (GetTimeElapsed(wait_tick) > discovery_to)
            {
                // 不响应：不能认为后面地址也不存在，所以直接扫描下一个地址
                NextDiscoveryAddress();
            }
        }
        break;
    default:
        // 异常状态，从地址1开始重新扫描
        BeginDiscoverySweep();
        break;
    }
}