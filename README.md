# 光栅老化系统（RS485 主从轮询 + UART 上行）

基于 **GD32F103C8T6**（Cortex-M3）的嵌入式固件。同一块硬件通过**拨码开关**配置成\*\*主机（Master）**或**从机（Slave）\*\*两种角色，多块板子经低成本半双工 **RS485 总线**相连构成一主多从的“光栅老化”数据采集系统，主机把从机数据汇总后经 **UART 上行到 ESP32**。

***

## 1. 系统总体架构

```
                          RS485 总线（半双工，115200）
  ┌──────────┐   ┌──────────┐   ┌──────────┐          ┌──────────┐
  │ 主机 #0   │   │ 从机 #01 │   │ 从机 #02 │   ...    │ 从机 #31 │
  │ Master   │◄─►│ Slave    │◄─►│ Slave    │◄─►       │ Slave    │
  └──────────┘   └──────────┘   └──────────┘          └──────────┘
        │  UART2 (PB10/PB11, 115200)
        ▼
   [ ESP32 ]  ← 收到主机汇总帧
```

- **主机（BOARD\_MASTER）**：拨码模式 = 000。主动轮询所有从机，收集各自光栅触发数据，汇总后一帧上行给 ESP32，并负责 OLED 显示。

- **从机（BOARD\_SLAVE）**：拨码模式 = 001\~100。接光栅传感器，只监听属于自己地址的轮询请求，收到后上报本地触发数据。其余时间采集光栅信号、按键、本地显示与报警。

- 最多支持 **31 个从机地址（1\~31）**；地址 0 保留给主机。

***

## 2. 角色与地址配置（拨码开关）

角色和从机地址都由拨码开关决定，见 [system\_config.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/system_config.c)：

- **地址（Address）**：`DIP_Read_Address()` 读取 1\~31，从机地址来源。

- **模式（Mode）**：由 `PA8/PA9/PA10` 三路拨码组合读取 `DIP_Read_Mode()`。

| 拨码 Mode | 角色     | 传感器类型 | 触发模式  |
| ------- | ------ | ----- | ----- |
| `000`   | **主机** | –     | –     |
| `001`   | 从机     | PNP   | 常开 NO |
| `010`   | 从机     | PNP   | 常闭 NC |
| `011`   | 从机     | NPN   | 常开 NO |
| `100`   | 从机     | NPN   | 常闭 NC |

> PNP/NPN、常开/常闭 定义见 [system\_config.h](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/include/system_config.h#L23-L36)。

***

## 3. 主从机数据流

### 3.1 从机（Slave）→ 数据采集与本地处理

主循环见 [main.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/User/main.c#L49-L58)，从机每轮执行：

1. `Sensor_Task()`：读取光栅触发信号（`PA4`，低电平触发，见 [sensor.h](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/sensor/sensor.h#L20-L21)），累计 `trigger_count`。
2. `RS485_Task()`：帧边界检测（软件判定帧结束）。
3. `RS485_Slave_Task()`：监听总线，若收到发给本机地址的**状态请求**，把 `trigger_count` 打包成应答帧发回。
4. `Alarm_Task()` / `Key_Task()` / `Key_Process_Task()` / `SEG_Task()`：本地报警、按键处理和数码管显示。

### 3.2 主机（Master）→ 发现 + 轮询 + 上报

主循环见 [main.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/User/main.c#L42-L48)，主机每轮执行：

1. `RS485_Task()`：帧边界检测（每收一帧判定结束）。
2. `RS485_Master_Task()`：核心轮询状态机（见下方），负责设备发现与逐个查询。
3. `SENSOR_UPLINK_Poll()`：把轮询完一轮、已上线的从机数据组装成帧发给 ESP32。
4. `OLED_Task()`：主机状态显示。

#### 主机轮询状态机（[rs485\_master\_task.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_master_task.c#L168-L301)）

```
MASTER_DISCOVERY_SEND ──发发现请求──▶ MASTER_DISCOVERY_WAIT
  采集到应答/超时 ▲                        │ 在线则登记
                  └──────扫描下一地址──────┘
全地址扫描完 → 重建活跃表 active_slave_list
                   │
                   ▼
MASTER_SEND ──发状态请求──▶ MASTER_WAIT
  收到应答/超时              │ 连续失败≥3次→判定离线
   ◀──下一个活跃地址──────────┘
  整轮结束 → SENSOR_UPLINK (TriggerSend) 上行
```

- **发现（Discovery）**：从地址 1 起逐地址探测，收到应答即登记 `online`，并重建 `active_slave_list`（仅收集实际在线地址）。

- **周期重扫**：`DISCOVERY_INTERVAL_MS = 20000`（20s）自动全量重扫一次，用于发现新加入或掉线的设备。

- **正常轮询**：只轮询 `active_slave_list[]` 中的在线设备，单设备应答超时 `RESPONSE_TIMEOUT_MS = 80ms`。

- **离线判定**：同一地址连续无应答 `RETRY_COUNT = 3` 次 → 置离线并移出活跃表。

***

## 4. RS485 通信协议

协议定义见 [rs485\_protocol.h](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/include/rs485_protocol.h)：

- 帧头：`0xAA`

- 波特率：**115200**，8 数据位、无校验、1 停止位

- 帧间空闲判据 `RS485_FRAME_TIMEOUT_MS = 5ms`（软件判帧）

### 4.1 主机请求帧（[RS485\_Request](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_protocol.c#L62-L73)）

主站向某个从机地址发状态查询（CMD=0x01）或清零计数（CMD=0x02）。

### 4.2 从机应答帧

从机应答状态帧（CMD=0x81）共 12 字节：

| 字段 | HEAD | ADDR | CMD | LEN | DATA(7B) | CRC |
| -- | ---- | ---- | --- | --- | -------- | --- |
| 长度 | 1    | 1    | 1   | 1   | 7        | 1   |

DATA 内含 `trigger_count`（触发累计）等状态字段。校验用 [RS485\_CalcCRC](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_protocol.c#L31-L46)。

> **回声拼接**：半双工总线下，主机发送的请求帧会被自家接收端回读（5 字节回声），可能与从机应答 **拼接成同一帧**。解析器 [RS485\_Parse\_Status](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_protocol.c#L73-L121) 会在缓冲区内**逐位搜寻** `HEAD+ADDR+CMD` 应答帧前缀并做 CRC 校验，忽略主站自身回声，两种情况（回声+应答拼接 / 应答独立成帧）都能正确解析。主机任务缓冲按此预留 24 字节。

***

## 5. 上行到 ESP32（UART2）

协议定义见 [sensor\_uplink.h](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/include/sensor_uplink.h)，经 **USART2（PB10=TX / PB11=RX，115200）**：

- 上行帧头：`0x55`，目标地址 `0x01`（ESP32），命令 `0x01`（传感器数据）

- 每个从机数据项 7 字节：`sensor_id(1) + timestamp(4) + count(2)`

- 主机每轮询完把所有在线从机的 7 字节项拼成一帧发送（一帧内携带 ≥1 个从机，帧长随在线数量变化）

```
55 01 01 | SEQ | 00 LEN | item1(7B)  item2(7B) ... | CRC
HEAD      ADDR CMD   数据长度(2B, = 在线数×7)  CRC
```

由 [SENSOR\_UPLINK\_TriggerSend](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/sensor_uplink.c) 触发、[SENSOR\_UPLINK\_Poll](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/sensor_uplink.c) 在主循环周期性发送（带最小发送间隔节流，避免抢占总线）。

***

## 6. 外围设备一览

| 模块           | 位置/引脚                                                                                                                                                                       | 说明                                                                                                              |
| ------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| **RS485 总线** | USART1（PA1=RE/DE、PA2=TX、PA3=RX），115200                                                                                                                                      | 主从通信媒介，[rs485.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/rs485/rs485.c)                       |
| **UART 上行**  | USART2（PB10=TX、PB11=RX），115200                                                                                                                                              | 主机向 ESP32 上报，[usart.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/usart/usart.c)                 |
| **光栅传感器**    | PA4，低电平触发                                                                                                                                                                   | 老化触发信号采集，[sensor.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/sensor/sensor.c)，支持 PNP/NPN、NO/NC |
| **拨码开关**     | 地址 + 模式（PA8/PA9/PA10）                                                                                                                                                       | 决定角色/地址/传感器类型，[dip\_switch.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/dip/dip_switch.c)       |
| **按键**       | [key.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/key/key.c)                                                                                                | 本地触发/清零等操作，[key\_process.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/key_process.c) 处理逻辑     |
| **数码管**      | TM1637，[tm1637.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/tm1637/tm1637.c) + [seg.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/seg/seg.c) | 从机显示触发计数等                                                                                                       |
| **OLED**     | [OLED.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/oled/OLED.c)                                                                                             | 主机显示汇总状态，[oled\_task.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/oled_task.c)                |
| **蜂鸣器/报警**   | [buzzer.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/buzzer/buzzer.c) + [alarm.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/alarm.c)      | 触发/异常报警提示                                                                                                       |
| **LED**      | [led.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/led/led.c)                                                                                                | 指示灯                                                                                                             |
| **系统时钟**     | [systick.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/User/systick.c)                                                                                                | 提供 `GetTick()`/`GetTimeElapsed()` 供轮询与超时计时                                                                      |

***

## 7. 目录结构

```
光栅老化系统/
├── User/                main.c、中断、systick、外设库裁剪宏
├── App/
│   ├── include/         业务模块头文件
│   └── Source/          业务逻辑
│       ├── rs485_master_task.c   主机轮询状态机
│       ├── rs485_slave_task.c    从机监听应答
│       ├── rs485_protocol.c      帧封装/解析/CRC
│       ├── sensor_uplink.c       上行 ESP32 帧
│       ├── system_config.c       角色/地址/传感器配置
│       ├── oled_task.c / key_process.c / alarm.c / setting.c
├── Hardware/            底层驱动（rs485/usart/sensor/dip/key/seg/tm1637/oled/buzzer/led/delay/icc）
└── Firmware/            GD32F10x 标准外设库 + CMSIS
```

***

## 8. 关键参数速查

| 参数        | 值            |
| --------- | ------------ |
| 从机地址范围    | 1\~31（0 为主机） |
| RS485 波特率 | 115200       |
| UART2 波特率 | 115200       |
| 应答超时      | 80ms         |
| 离线阈值      | 连续 3 次无应答    |
| 全量重扫周期    | 20s          |
| 帧间空闲判据    | 5ms          |

***

## 9. 注意事项

- **半双工回声**：RS485 收发会回读本机发送字节，解析层已做回声兼容，勿改坏 `RS485_Parse_Status` 的裁帧逻辑。

- **24V 上电偶发卡死**：若在 24V 工业电源热插拔上电时偶发无响应、5V 下正常，优先排查上电浪涌/3.3V 跌落导致的复位振荡，并建议在主循环加 **IWDG 独立看门狗喂狗** 保底（当前工程未启用）。

- **改固件后回到 20s 重扫**：新设备会在一次全量重扫周期内被自动发现并加入上行帧。

<br />
