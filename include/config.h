#pragma once

// WiFi
#define WIFI_SSID "猕猴桃"
#define WIFI_PASS "12345678"

// NTP 时区（CST-8 = 中国标准时间 UTC+8）
#define NTP_TZ "CST-8"
#define NTP_SERVER1 "ntp.aliyun.com"
#define NTP_SERVER2 "ntp.tencent.com"
#define NTP_SERVER3 "pool.ntp.org"

// I2C 引脚（NodeMCU / D1 mini：D1=GPIO5, D2=GPIO4）
#define I2C_SDA 4
#define I2C_SCL 5
#define I2C_CLOCK 400000
