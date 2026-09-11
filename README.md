# 光栅老化系统（RS485 主从轮询 + UART 上行）

基于 **GD32F103C8T6**（Cortex-M3）的嵌入式固件。同一块硬件通过**拨码开关**配置成**主机（Master）**或**从机（Slave）**两种角色，多块板子经低成本半双工 **RS485 总线**相连构成一主多从的“光栅老化”数据采集系统，主机把从机数据汇总后经 **UART 上行到 ESP32**。

---

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

- **主机（BOARD_MASTER）**：拨码模式 = 000。主动轮询所有从机，收集各自光栅触发数据，汇总后一帧上行给 ESP32，并负责 OLED 显示。
- **从机（BOARD_SLAVE）**：拨码模式 = 001~100。接光栅传感器，只监听属于自己地址的轮询请求，收到后上报本地触发数据。其余时间采集光栅信号、按键、本地显示与报警。
- 最多支持 **31 个从机地址（1~31）**；地址 0 保留给主机。

---

## 2. 角色与地址配置（拨码开关）

角色和从机地址都由拨码开关决定，见 [system_config.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/system_config.c)：

- **地址（Address）**：`DIP_Read_Address()` 读取 1~31（5 位地址拨码），从机地址来源。
- **模式（Mode）**：由 `PA8/PA9/PA10` 三路拨码组合读取 `DIP_Read_Mode()`。

| 拨码 Mode | 角色   | 传感器类型 | 触发模式 |
| ------- | ------ | ----- | ----- |
| `000`   | **主机** | –     | –     |
| `001`   | 从机   | PNP   | 常开 NO |
| `010`   | 从机   | PNP   | 常闭 NC |
| `011`   | 从机   | NPN   | 常开 NO |
| `100`   | 从机   | NPN   | 常闭 NC |

> PNP/NPN、常开/常闭 定义见 [system_config.h](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/include/system_config.h#L23-L36)。

---

## 3. 主从机数据流

### 3.1 从机（Slave）→ 数据采集与本地处理

主循环见 [main.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/User/main.c#L49-L58)，从机每轮执行：

1. `Sensor_Task()`：读取光栅触发信号（`PA4`，低电平触发，见 [sensor.h](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/sensor/sensor.h#L20-L21)），累计 `trigger_count`，带软件消抖。
2. `RS485_Task()`：帧边界检测（软件判定帧结束）。
3. `RS485_Slave_Task()`：监听总线，若收到发给本机地址的**状态请求**，把 `trigger_count` 打包成应答帧发回；收到清零命令则 `Setting_Reset_Long()`。
4. `Alarm_Task()` / `Key_Task()` / `Key_Process_Task()` / `SEG_Task()`：本地报警、按键处理和数码管显示。

### 3.2 主机（Master）→ 发现 + 轮询 + 上报

主循环见 [main.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/User/main.c#L42-L48)，主机每轮执行：

1. `RS485_Task()`：帧边界检测（每收一帧判定结束）。
2. `RS485_Master_Task()`：核心轮询状态机（见下方），负责设备发现与逐个查询。
3. `SENSOR_UPLINK_Poll()`：把轮询完一轮、已上线的从机数据组装成帧发给 ESP32。
4. `OLED_Task()`：主机状态显示。

#### 主机轮询状态机（[rs485_master_task.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_master_task.c#L154-L289)）

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
- **分档超时基准**：Discovery 无响应时，用**持久化的 `discovered` 标志**区分——“曾回复过的已知设备”给足 `RESPONSE_TIMEOUT_MS=80ms`，防止慢设备被误判为空；“从未见过的空地址”用 `DISCOVERY_TIMEOUT_FAST_MS=20ms` 快速判空跳过，把一次全量重扫从约 2.2s 缩短到约 0.56s。
- **周期重扫**：`DISCOVERY_INTERVAL_MS = 20000`（20s）自动全量重扫一次，用于发现新加入或掉线的设备。
- **正常轮询**：只轮询 `active_slave_list[]` 中的在线设备，单设备应答超时 `RESPONSE_TIMEOUT_MS = 80ms`。
- **离线判定**：同一地址连续无应答 `RETRY_COUNT = 3` 次 → 置离线、`trigger_count` 清零并移出活跃表。

---

## 4. RS485 通信协议

协议定义见 [rs485_protocol.h](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/include/rs485_protocol.h)：

- 帧头：`0xAA`
- 波特率：**115200**，8 数据位、无校验、1 停止位
- 帧间空闲判据 `RS485_FRAME_TIMEOUT_MS = 5ms`（软件判帧）

### 4.1 主机请求帧（[RS485_Request](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_protocol.c#L22-L31)）

主站向某个从机地址发状态查询（`CMD=0x01`）。数据长度 0，共 5 字节：`AA + ADDR + CMD + LEN(0) + CRC`。

### 4.2 从机应答帧（[RS485_SendStatus](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_protocol.c#L36-L62)）

从机应答状态帧（`CMD=0x81`）共 12 字节：

| 字段 | HEAD | ADDR | CMD | LEN | DATA(7B) | CRC |
| -- | ---- | ---- | --- | --- | -------- | --- |
| 长度 | 1    | 1    | 1   | 1   | 7        | 1   |

DATA 7 字节内含 `trigger_count`（2B，小端）、`sensor_status`（1B）、`timestamp`（4B，小端）。校验用 [RS485_CalcCRC](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_protocol.c#L12-L20)。

> **回声拼接**：半双工总线下，主机发送的请求帧会被自家接收端回读（5 字节回声），可能与从机应答**拼接成同一帧**。解析器 [RS485_Parse_Status](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_protocol.c#L90-L120) 会在缓冲区内**逐位搜寻** `HEAD+ADDR+CMD` 应答帧前缀并做 CRC 校验，忽略主站自身回声，两种情况（回声+应答拼接 / 应答独立成帧）都能正确解析。主机任务接收缓冲按此预留 24 字节（见 [rs485_master_task.c#L157](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/rs485_master_task.c#L157)）。

---

## 5. 上行到 ESP32（UART2）

协议定义见 [sensor_uplink.h](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/include/sensor_uplink.h)，经 **USART2（PB10=TX / PB11=RX，115200）**：

- 上行帧头：`0x55`，目标地址 `0x01`（ESP32），命令 `0x01`（传感器数据）
- 每个从机数据项 7 字节：`sensor_id(1) + timestamp(4) + count(2)`
- 主机每轮询完把所有在线从机的 7 字节项拼成一帧发送（一帧内携带 ≥1 个从机，帧长随在线数量变化）

```
55 01 01 | SEQ | 00 LEN | item1(7B)  item2(7B) ... | CRC
HEAD      ADDR CMD   数据长度(2B, = 在线数×7)  CRC
```

由 [SENSOR_UPLINK_TriggerSend](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/sensor_uplink.c#L127-L130) 触发（每轮轮询结束时调用），[SENSOR_UPLINK_Poll](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/sensor_uplink.c#L135-L141) 在主循环周期性发送，带 `MIN_SEND_INTERVAL_MS=300ms` 最小发送间隔节流，避免抢占/UART 拥塞。

---

## 6. 外围设备一览

| 模块         | 位置/引脚                                        | 说明                                                                                                   |
| ---------- | -------------------------------------------- | ---------------------------------------------------------------------------------------------------- |
| **RS485 总线** | USART1（PA1=RE/DE、PA2=TX、PA3=RX），115200          | 主从通信媒介，[rs485.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/rs485/rs485.c)               |
| **UART 上行** | USART2（PB10=TX、PB11=RX），115200                | 主机向 ESP32 上报，[usart.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/usart/usart.c)         |
| **光栅传感器**  | PA4，低电平触发（EXTI4）                              | 老化触发信号采集，[sensor.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/sensor/sensor.c)，支持 PNP/NPN、NO/NC |
| **拨码开关**   | 地址(5bit) + 模式（PA8/PA9/PA10）                     | 决定角色/地址/传感器类型，[dip_switch.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/dip/dip_switch.c)   |
| **按键**     | PB12~PB15                                        | 本地设置/清零等操作，[key.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/key/key.c) 采集，[key_process.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/key_process.c) 处理逻辑 |
| **数码管**    | TM1637（PB6=CLK、PB7=DIO）                        | 从机显示触发计数，见 [seg.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/seg/seg.c)（SEG_Task 驱动）        |
| **OLED**    | 软件 I2C                                         | 主机显示汇总状态，[OLED.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/oled/OLED.c) + [oled_task.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/oled_task.c) |
| **蜂鸣器/报警** | PA15                                            | 触发/异常报警提示，[buzzer.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/buzzer/buzzer.c) + [alarm.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/alarm.c) |
| **LED**    | PC13                                            | 指示灯，[led.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/Hardware/led/led.c)                   |
| **系统时钟**   | SysTick                                         | 提供 `GetTick()`/`GetTimeElapsed()` 供轮询与超时计时，[systick.c](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/User/systick.c) |

---

## 7. 目录结构与文件功能

```
光栅老化系统/                      GD32F103C8T6 固件工程（Keil MDK, Project.uvprojx）
│
├── User/                     启动与用户主入口
│   ├── main.c                程序入口。读取拨码配置，按主/从分支调用对应任务循环
│   ├── main.h                空头文件（占位）
│   ├── systick.c/h           SysTick 1ms 时基：GetTick()/GetTimeElapsed()/delay_1ms
│   ├── gd32f10x_it.c/h       系统异常（NMI/HardFault 等）+ SysTick_Handler
│   └── gd32f10x_libopt.h     外设库裁剪配置宏（开启/关闭用到的外设模块）
│
├── App/                      业务应用层（跨模块编排）
│   ├── include/              业务头文件
│   │   ├── system_config.h           系统配置结构体 + 全局 slave_list[31]
│   │   ├── rs485_master_task.h       主机状态机枚举 + 超时/重试/重扫参数
│   │   ├── rs485_protocol.h          RS485 帧常量（帧头/命令/长度）
│   │   ├── rs485_slave_task.h        从机任务声明
│   │   ├── sensor_uplink.h           上行 ESP32 协议定义（0x55 帧/数据项布局）
│   │   ├── setting.h / key_process.h / alarm.h / oled_task.h
│   └── Source/               业务逻辑实现
│       ├── system_config.c           读拨码 → 判定 主/从 + 传感器 PNP/NPN + NO/NC
│       ├── rs485_master_task.c       主机核心：Discovery/轮询状态机、活跃表、离线处理、触发上行
│       ├── rs485_slave_task.c        从机应答：收到本地址请求→回状态帧；收到清零→重置
│       ├── rs485_protocol.c          帧封装/解析/CRC；RS485_Parse_Status 做回声兼容裁帧
│       ├── sensor_uplink.c           汇总在线从机数据 → 组 0x55 帧 → USART2 发送（带节流）
│       ├── setting.c                 数码管数值：触发计数累加、SET/UP/DOWN 编辑、短/长按重置
│       ├── key_process.c             按键事件 → 映射到设置/增减/重置动作
│       ├── alarm.c                   触发事件后蜂鸣器响 300ms
│       └── oled_task.c               OLED 上显示公司名 + 在线设备 ID/状态/触发次数
│
├── Hardware/                底层驱动（每个外设一个子目录）
│   ├── rs485/   rs485.c/h        USART1 + DE/RE 控制。中断接收、逐字节发送、软件判帧边界
│   ├── usart/   usart.c/h        USART2 发送/接收（主机→ESP32 上行）＋回声测试
│   ├── sensor/  sensor.c/h       PA4/EXTI4 光栅触发，软件消抖累加计数
│   ├── dip/     dip_switch.c/h   读取模式(PA8~10)与地址(PB1,PB0,PA7,PA6,PA5)
│   ├── key/     key.c/h          PB12~15 按键扫描，支持短按/长按(2s)
│   ├── seg/     seg.c/h          TM1637 数码管（PB6/PB7）位选/段码/闪烁显示
│   ├── tm1637/  tm1637.c/h       TM1637 另一套驱动（与 seg.c 等价，当前未被 main 调用）
│   ├── oled/    OLED.c/h + OLED_Data.c   SSD1306 OLED 驱动 + 字库
│   ├── icc/     icc.c/h          软件 I2C（IIC_Start/SendByte/...）通用位操作驱动
│   ├── buzzer/  buzzer.c/h       PA15 蜂鸣器（需释放 JTAG、保留 SWD）
│   ├── led/     led.c/h          PC13 指示灯（低电平点亮）
│   ├── delay/   delay.c/h        阻塞延时（Delay_us/Delay_ms，配合 SysTick）
│   └── ...
│
├── Firmware/                官方库与内核
│   ├── CMSIS/                core_cm3 + GD32F10x 系统文件(system_gd32f10x.c/h)
│   └── GD32F10x_standard_peripheral/   GD32F10x 标准外设库（usart/gpio/rcu/exti/misc 等）
│
├── Project.uvprojx          Keil MDK 工程文件
├── Objects/                 编译输出（含 Project.sct 链接脚本）
└── RTE/                     运行时环境配置
```

### 7.1 关键全局数据

- `Slave_Info slave_list[MAX_SLAVE_ADDRESS]`（[system_config.h#L65-L74](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/include/system_config.h#L65-L74)）：主/从机共享的从机信息表，含 `online`、`discovered`（持久化“曾在线”标志）、`trigger_count`、`sensor_status`、`last_time`、`last_trigger_time`、`timestamp`。
- `config`（[system_config.c#L9](file:///d:/data/c_code/gd32f103c8t6/寄存器开发/光栅老化系统/App/Source/system_config.c#L9)）：全局只读配置（板型/地址/传感器类型/触发模式），由 `System_Config_Get()` 获取。

---

## 8. 关键参数速查

| 参数             | 值                    |
| -------------- | -------------------- |
| 从机地址范围        | 1~31（0 为主机）           |
| RS485 波特率     | 115200               |
| UART2 波特率     | 115200               |
| 应答超时          | 80ms                 |
| Discovery 空址快速超时 | 20ms                 |
| 离线阈值          | 连续 3 次无应答             |
| 全量重扫周期        | 20s                  |
| 上行最小发送间隔      | 300ms                |
| 帧间空闲判据        | 5ms                  |

---

## 9. 注意事项

- **半双工回声**：RS485 收发会回读本机发送字节，解析层已做回声兼容，勿改坏 `RS485_Parse_Status` 的裁帧逻辑。
- **驱动重复**：数码管存在 `seg.c`（当前 main 引用）与 `tm1637.c` 两套等价驱动，请以 `seg.c` 为准，避免改动冲突。
- **引脚复用**：USART2(PB10/PB11) 与软件 I2C 定义的引脚存在复述，实际启用以上行 UART 为准；数码管使用 PB6/PB7。
- **24V 上电偶发卡死**：若在 24V 工业电源热插拔上电时偶发无响应、5V 下正常，优先排查上电浪涌/3.3V 跌落导致的复位振荡，并建议在主循环加 **IWDG 独立看门狗喂狗** 保底（当前工程未启用）。
- **改固件后回到 20s 重扫**：新设备会在一次全量重扫周期内被自动发现并加入上行帧；掉线设备在重扫时若恢复，会被重新拉回活跃表。

<br />