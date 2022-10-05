# ROV_SensorBoard

An opensource micro board with popular sensors using in ROV/AUV

Improved from https://github.com/redlightASl/ROV_WaterDepthAnalysisBoard-MS5837

Sensor as follow:

* SHT20
    * carbin humidity
    * carbin temperature
* MS5837
    * water temperature
    * water pressure
    * water depth

# ROV传感器集成板

一套开源的ROV/AUV常用传感器解算板，基于stm32f103c8t6，引出如下接口：

| 引出接口   | 功能                       |
| ---------- | -------------------------- |
| +5         | 5V供电                     |
| SWDIO      | 调试接口                   |
| SWCLK      | 调试接口                   |
| GND        | 数字地                     |
| WD_UART_TX | 主串口TX                   |
| WD_UART_RX | 主串口RX                   |
| UART2_TX   | 从机串口TX                 |
| UART2_RX   | 从机串口RX                 |
| Ctrl       | NRST复位控制               |
| GPIO_PA0   | IO接口，可用于PWM输出或ADC |
| Q          | IO接口，可用于PWM输出      |
| SPI1_SCLK  | SPI1 时钟                  |
| SPI1_N     | SPI1 MISO                  |
| SPI1_P     | SPI1 MOSI                  |
| SPI1_NSS   | SPI1 片选                  |

板载如下设备：

- 1.25mm间距5V供电I2C传感器连接座
    - SCL：PB6
    - SDA：PB7
    - 提供MS5837传感器数据解算
- SHT20温湿度传感器
- 板载LED（PB5）
- 连接到3.3V并带有1/2分压的ADC1（PA1）

另有一排标准JTAG接口

