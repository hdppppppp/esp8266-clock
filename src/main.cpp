#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ESP8266WiFi.h>
#include <time.h>

#include "config.h"

U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, I2C_SCL, I2C_SDA);

static const char *const WEEKDAYS[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

static bool ntpSynced = false;
static unsigned long lastWifiCheck = 0;
static unsigned long lastNtpTry = 0;
static unsigned long lastDraw = 0;

static void splash(const char *line1, const char *line2) {
  display.clearBuffer();
  display.setFont(u8g2_font_6x12_tf);
  display.drawStr((128 - display.getStrWidth(line1)) / 2, 26, line1);
  if (line2) display.drawStr((128 - display.getStrWidth(line2)) / 2, 44, line2);
  display.sendBuffer();
}

static void drawWifiBars(int rssi) {
  int bars = WiFi.status() == WL_CONNECTED ? (rssi > -60 ? 4 : rssi > -70 ? 3 : rssi > -80 ? 2 : 1) : 0;
  for (int i = 0; i < 4; i++) {
    int h = 2 + i * 2;
    int x = 108 + i * 5;
    int y = 63 - h;
    if (i < bars) {
      display.drawBox(x, y, 3, h);
    } else {
      display.drawFrame(x, y, 3, h);
    }
  }
}

static void renderClock() {
  time_t now = time(nullptr);
  struct tm t;
  localtime_r(&now, &t);
  bool valid = ntpSynced && now > 1600000000L;

  display.clearBuffer();

  display.setFont(u8g2_font_6x12_tf);
  if (valid) {
    display.drawStr(0, 9, WEEKDAYS[t.tm_wday]);
    char sec[4];
    snprintf(sec, sizeof(sec), "%02d", t.tm_sec);
    display.drawStr(128 - display.getStrWidth(sec), 9, sec);
  }

  display.setFont(u8g2_font_logisoso42_tn);
  if (valid) {
    char hh[4], mm[4];
    snprintf(hh, sizeof(hh), "%02d", t.tm_hour);
    snprintf(mm, sizeof(mm), "%02d", t.tm_min);
    int wd = display.getStrWidth("0");
    int wc = display.getStrWidth(":");
    int x = (128 - (wd * 4 + wc)) / 2;
    int base = 52;
    display.drawStr(x, base, hh);
    if (t.tm_sec % 2 == 0) display.drawStr(x + wd * 2, base, ":");
    display.drawStr(x + wd * 2 + wc, base, mm);
  } else {
    const char *ph = "--:--";
    display.drawStr((128 - display.getStrWidth(ph)) / 2, 52, ph);
  }

  display.setFont(u8g2_font_6x12_tf);
  char date[16];
  if (valid) {
    snprintf(date, sizeof(date), "%04d-%02d-%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
  } else {
    strcpy(date, "---- -- --");
  }
  display.drawStr(0, 63, date);
  drawWifiBars(WiFi.RSSI());

  display.sendBuffer();
}

static void startNtp() {
  configTzTime(NTP_TZ, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);
  lastNtpTry = millis();
}

static void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  lastWifiCheck = millis();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  display.begin();
  Wire.setClock(I2C_CLOCK);

  splash("ESP8266 CLOCK", "WiFi...");
  Serial.printf("Connecting to WiFi...\n");

  connectWifi();
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(100);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("WiFi OK, IP: %s\n", WiFi.localIP().toString().c_str());
    splash("WiFi OK", "NTP sync...");
    startNtp();
    time_t now = time(nullptr);
    while (now < 1600000000L && millis() - start < 25000) {
      delay(100);
      now = time(nullptr);
    }
    ntpSynced = now > 1600000000L;
    Serial.printf("NTP %s\n", ntpSynced ? "OK" : "timeout");
  } else {
    Serial.printf("WiFi timeout, will retry in background\n");
  }

  renderClock();
}

void loop() {
  if (millis() - lastWifiCheck > 10000) {
    lastWifiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.printf("WiFi lost, reconnecting...\n");
      WiFi.reconnect();
    }
  }

  if (!ntpSynced && WiFi.status() == WL_CONNECTED && millis() - lastNtpTry > 30000) {
    startNtp();
  }
  if (!ntpSynced) {
    time_t now = time(nullptr);
    if (now > 1600000000L) {
      ntpSynced = true;
      Serial.printf("NTP synced\n");
    }
  }

  if (millis() - lastDraw >= 250) {
    lastDraw = millis();
    renderClock();
  }
  delay(20);
}
