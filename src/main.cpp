#include "WiFiUdp.h"
#include <Adafruit_GFX.h>
#include <Adafruit_MCP9808.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Fonts/FreeMonoOblique9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <NTP.h>
#include <SPI.h>
#include <Wire.h>
// order matters for u8g2
// clang-format off
#include <U8g2lib.h>
#include <MUIU8g2.h>
// clang-format on

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3D // 0x3D for 128x64, 0x3C for 128x32
U8G2_SSD1306_128X64_NONAME_F_HW_I2C
u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE, /* clock=*/D1,
     /* data=*/D2); // ESP32 Thing, HW I2C with pin remapping

#define STRINGIZE(arg) #arg
#define SVAL(x) STRINGIZE(x)
#ifndef WIFI_SSID
#error "WIFI_SSID not defined"
#endif
#ifndef WIFI_PASS
#error "WIFI_PASS not defined"
#endif
#define WIFI_SSID_SVAL SVAL(WIFI_SSID)
#define WIFI_PASS_SVAL SVAL(WIFI_PASS)

const char *wifi_ssid = WIFI_SSID_SVAL;
const char *wifi_pass = WIFI_PASS_SVAL;
uint8_t macAddr[WL_MAC_ADDR_LENGTH];
char macStr[WL_MAC_ADDR_LENGTH * 2 + 1];

Adafruit_MCP9808 tempSensor;
WiFiUDP wifiUdp;
NTP ntpSyd(wifiUdp);
NTP ntpPT(wifiUdp);
NTP ntpET(wifiUdp);
NTP ntpUTC(wifiUdp);
NTP ntpSG(wifiUdp);
NTP ntpIndia(wifiUdp);

// const uint8_t *u8g2_font = u8g2_font_shylock_nbp_t_all;
// const int u8g2_font_height = 12;
const uint8_t *u8g2_font = u8g2_font_7x14_mf; // u8g2_font_pxplusibmvga8_m_all;
const int u8g2_font_height = 10;
const uint8_t *u8g2_font2 = u8g2_font_t0_17b_mf; // u8g2_font_shylock_nbp_t_all;
const int u8g2_font2_height = 11;
const uint8_t *u8g2_font3 = u8g2_font_t0_13_mf ; // u8g2_font_shylock_nbp_t_all;
const int u8g2_font3_height = 9;

const int line_spacing = 3;

void setup_wifi() {
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_pass);
  WiFi.macAddress(macAddr);
  for (int i = 0; i < WL_MAC_ADDR_LENGTH; i++) {
    sprintf(macStr + 2 * i, "%02x", macAddr[i]);
  }
  macStr[WL_MAC_ADDR_LENGTH * 2] = 0;
  Serial.print("mac address: ");
  Serial.println(macStr);
  Serial.print("wifi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  randomSeed(micros());

  Serial.print("wifi connected, IP address: ");
  Serial.println(WiFi.localIP());

  u8g2.clearBuffer();
  int display_y = 0;
  display_y += u8g2_font_height + line_spacing;
  u8g2.setCursor(0, display_y);
  u8g2.print(F("wifi connected."));
  display_y += u8g2_font_height + line_spacing;
  u8g2.setCursor(0, display_y);
  u8g2.print(WiFi.localIP());
  display_y += u8g2_font_height + line_spacing;
  u8g2.setCursor(0, display_y);
  u8g2.print(macStr);
  u8g2.sendBuffer();

  delay(1000);
}

void setup() {
  // Initialize LED pin
  pinMode(LED_BUILTIN, OUTPUT);
  // LED off (active low)
  digitalWrite(LED_BUILTIN, HIGH);

  // Initialize serial port
  Serial.begin(115200);
  Serial.println("setup()");

  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font);

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  int display_y = 0;
  display_y += u8g2_font_height + line_spacing;
  u8g2.setCursor(0, display_y);
  u8g2.print(F("init..."));
  u8g2.sendBuffer();

  setup_wifi();

  // Initialize temperature sensor
  if (!tempSensor.begin()) {
    Serial.println("Failed to find Adafruit MCP9808 chip");
    while (1) {
      delay(10);
    }
  }

  // Initialize NTP
  ntpSyd.ruleDST("AEDT", First, Sun, Oct, 2, 11 * 60);
  ntpSyd.ruleSTD("AEST", First, Sun, Apr, 3, 10 * 60);
  ntpSyd.begin();
  ntpPT.ruleDST("PDT", Second, Sun, Mar, 2, -7 * 60);
  ntpPT.ruleSTD("PST", First, Sun, Nov, 3, -8 * 60);
  ntpPT.begin();
  ntpET.ruleDST("EDT", Second, Sun, Mar, 2, -4 * 60);
  ntpET.ruleSTD("EST", First, Sun, Nov, 3, -5 * 60);
  ntpET.begin();
  ntpUTC.begin();
  ntpIndia.timeZone(5, 30);
  ntpIndia.isDST(false);
  ntpIndia.begin();
  ntpSG.timeZone(8, 0);
  ntpSG.isDST(false);
  ntpSG.begin();

  Serial.println("setup() done");
}

void loop() {

  sensors_event_t event;
  tempSensor.getEvent(&event);

  // Serial.print("Temperature: ");
  // Serial.print(event.temperature);
  // Serial.print(" degC");
  // Serial.println("");

  char tempStr[10];
  sprintf(tempStr, "%.1f°C", event.temperature);

  u8g2.clearBuffer();
  int display_y = 0;

  display_y += u8g2_font_height + line_spacing;
  u8g2.setCursor(0, display_y);
  u8g2.print(ntpSyd.formattedTime("%Y-%m-%d"));

  u8g2.setCursor(128-u8g2.getUTF8Width(tempStr), display_y);
  u8g2.print(tempStr);

  // use a different font for the time
  u8g2.setFont(u8g2_font2);
  display_y += u8g2_font2_height + line_spacing;
  display_y += 3; // adjust
  u8g2.setCursor(0, display_y);
  u8g2.print(ntpSyd.formattedTime("%H:%M:%S"));
  if (ntpSyd.isDST()) {
    u8g2.println(" AEDT");
  } else {
    u8g2.println(" AEST");
  }
  u8g2.setFont(u8g2_font); // reset font

  u8g2.setFont(u8g2_font3);
  display_y += u8g2_font3_height + line_spacing;
  display_y += 8; // adjust
  u8g2.setCursor(0, display_y);
  u8g2.print(ntpPT.formattedTime("%H"));
  if (ntpPT.isDST()) {
    u8g2.println("pdt");
  } else {
    u8g2.println("pst");
  }
  u8g2.print(" ");
  u8g2.print(ntpET.formattedTime("%H"));
  u8g2.print("ny");
  u8g2.print(" ");
  u8g2.print(ntpUTC.formattedTime("%H"));
  u8g2.print("utc");
  display_y += u8g2_font3_height + line_spacing;
  u8g2.setCursor(0, display_y);
  u8g2.print(ntpIndia.formattedTime("%H:%M"));
  u8g2.print("in");
  u8g2.print(" ");
  u8g2.print(ntpSG.formattedTime("%H"));
  u8g2.print("sg");

  u8g2.sendBuffer();
  delay(10);
}
