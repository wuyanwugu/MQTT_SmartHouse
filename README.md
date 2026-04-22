# MQTT_OK_demo - 基于STM32的MQTT物联网智能控制系统

## 项目简介

本项目是一个基于STM32F103单片机和FreeRTOS实时操作系统的物联网控制系统，通过MQTT协议实现远程设备控制与数据采集。系统支持多种传感器和执行器的实时监控与控制，具备高可靠性和实时响应能力。

## 硬件平台

- **主控芯片**: STM32F103C8T6
- **WiFi模块**: ESP8266 (AT指令模式)
- **开发环境**: STM32CubeMX + Keil MDK
- **操作系统**: FreeRTOS

## 主要功能

### 远程设备控制

- **LED控制**: 远程开关LED
- **蜂鸣器控制**: 主动蜂鸣器和被动蜂鸣器控制
- **SG90舵机**: 0-180度角度控制
- **RGB全彩LED**: 颜色控制
- **OLED显示**: 文本显示和清屏

### 传感器数据采集

- **MPU6050**: 六轴传感器数据采集（加速度、陀螺仪）
- **DHT11**: 温湿度传感器数据采集

### 通信功能

- **MQTT协议**: 消息订阅和发布
- **AT指令**: ESP8266 WiFi模块控制
- **DMA传输**: 高效的串口数据传输

## 项目结构

```
MQTT_OK_demo/
├── Core/
│   ├── Inc/           # 头文件
│   │   ├── mqttsmart.h
│   │   ├── tim.h
│   │   └── ...
│   └── Src/           # 源文件
│       ├── mqttsmart.c    # MQTT智能控制
│       ├── freertos.c     # FreeRTOS任务
│       └── main.c         # 主程序
├── Drivers/
│   ├── DshanMCU-F103/     # 外设驱动
│   │   ├── driver_led.c
│   │   ├── driver_mpu6050.c
│   │   ├── driver_dht11.c
│   │   ├── driver_sg90.c
│   │   ├── driver_color_led.c
│   │   └── ...
│   └── STM32F1xx_HAL_Driver/  # HAL库
├── Middlewares/
│   └── MQTTclient/       # MQTT客户端
│       └── platform/
│           └── FreeRTOS/
│               └── platform_esp8266_AT.c
└── 01_freertos_template.ioc  # STM32CubeMX配置文件
```

## MQTT指令格式

### 设备控制指令

| 设备     | 指令格式                           | 示例                    |
| -------- | ---------------------------------- | ----------------------- |
| LED      | `{led_on}` / `{led_off}`       | 开关LED                 |
| 蜂鸣器   | `{buzzer_on}` / `{buzzer_off}` | 开关蜂鸣器              |
| SG90舵机 | `{sg90_angle_90}`                | 设置角度为90度          |
| RGB LED  | `{COLOR_LED_FF0000}`             | 设置红色 (16进制颜色值) |
| OLED显示 | `{OLED_print_10_20_Hello}`       | 在(10,20)显示"Hello"    |
| OLED清屏 | `{OLED_clear}`                   | 清空OLED屏幕            |

### 传感器读取指令

| 设备    | 指令格式           | 返回数据           |
| ------- | ------------------ | ------------------ |
| DHT11   | `{dht11_read}`   | 温湿度数据         |
| MPU6050 | `{mpu6050_read}` | 加速度和陀螺仪数据 |

## 技术特点

### 系统架构

- **多任务并发**: 基于FreeRTOS的多任务架构
- **资源保护**: 使用互斥量保护共享资源
- **实时响应**: 优化的任务调度和优先级管理

### 通信优化

- **DMA传输**: ESP8266串口接收采用DMA模式
- **环形缓冲区**: 高效的数据缓冲管理
- **错误处理**: 完善的异常检测和恢复机制

### 驱动开发

- **模块化设计**: 独立的外设驱动模块
- **统一接口**: 标准化的驱动接口
- **易于扩展**: 支持新设备的快速集成

## 开发环境配置

### 必需软件

- STM32CubeMX (用于硬件配置)
- Keil MDK-ARM (代码编译)
- MQTT客户端工具 (如MQTTX、mosquitto)

### 硬件连接

```
STM32F103C8T6          ESP8266
PB10 (USART3_TX) <---> RX
PB11 (USART3_RX) <---> TX
GND                 <---> GND
VCC (3.3V)          <---> VCC
```

### 编译和下载

1. 使用STM32CubeMX打开 `01_freertos_template.ioc` 文件
2. 生成代码
3. 使用Keil MDK打开项目文件
4. 编译并下载到STM32开发板

## 使用说明

### 1. 初始化配置

- 确保ESP8266已正确连接
- 修改MQTT服务器配置（IP、端口、用户名、密码）在\Middlewares\MQTTclient\platform\FreeRTOS\platform_net_socket.c文件中修改WiFi配置，
- 编译并下载程序

### 2. 启动系统

- 上电后系统自动初始化
- ESP8266自动连接WiFi和MQTT服务器
- 系统进入待机状态，等待MQTT指令

### 3. 发送控制指令

- 使用MQTT客户端连接到同一服务器
- 订阅相关主题
- 发送控制指令

## 已知问题和解决方案

### ESP8266上电顺序问题

**问题**: ESP8266先上电，STM32后上电时，ESP8266不发送数据

**解决方案**:

- 建议同时上电或STM32先上电
- 可添加ESP8266硬件复位控制
- 在初始化代码中添加延时

### DMA接收优化

**改进**: 将ESP8266串口接收从中断模式改为DMA模式，提高传输效率

### 互斥量初始化

**优化**: 调整互斥量初始化策略，解决AT命令时序冲突

## 技术支持

如有问题，请参考以下资源：

- STM32官方文档
- FreeRTOS官方文档
- MQTT协议规范
- ESP8266 AT指令集

## 许可证

本项目仅供学习和研究使用。

## 作者

MQTT物联网智能控制系统项目组

## 更新日志

### v1.0

- 完成基础MQTT功能
- 实现7种外设控制
- 优化DMA传输
- 添加FreeRTOS多任务支持
