# CLAUDE.md

## 项目
ESP8266（NodeMCU / Wemos D1 mini）+ SH1106 128x64 I2C OLED 网络时钟固件。
技术栈：PlatformIO + Arduino 框架 + U8g2。

## 常用命令
- 编译：`pio run`
- 烧录：`pio run -t upload`
- 串口监视：`pio device monitor`
- CI：推送到 `main` 由 GitHub Actions 自动编译，固件在 Actions artifact 中

## 硬件接线（4 针 I2C OLED）
| OLED | ESP8266 |
|------|---------|
| GND  | GND     |
| VCC  | 3.3V    |
| SCL  | D1 (GPIO5) |
| SDA  | D2 (GPIO4) |

## 规则
- WiFi SSID/密码、时区等配置统一放 `include/config.h`
- SH1106 必须用 U8g2 驱动（Adafruit SSD1306 不兼容 SH1106 的 132 列偏移）
- 屏幕字体仅支持 ASCII，UI 文案不得使用中文
- 提交前 `pio run` 必须通过
