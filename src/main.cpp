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

#ifndef NUM_DISPLAYS
#error "NUM_DISPLAYS not defined"
#endif

U8G2_SSD1306_128X64_NONAME_F_HW_I2C
u8g2_1(U8G2_R0, U8X8_PIN_NONE, /*SCL*/ D1, /*SDA*/ D2);

#if NUM_DISPLAYS >= 2
U8G2_SSD1306_128X64_NONAME_F_SW_I2C
u8g2_2(U8G2_R0, /*SCL*/ D5, /*SDA*/ D6, U8X8_PIN_NONE);
#endif

const int numDisplays = NUM_DISPLAYS;
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
NTP ntpLON(wifiUdp);
NTP ntpSG(wifiUdp);
NTP ntpIndia(wifiUdp);

// const uint8_t *u8g2_font = u8g2_font_shylock_nbp_t_all;
// const int u8g2_font_height = 12;
const uint8_t *font1 = u8g2_font_7x14_mf; // u8g2_font_pxplusibmvga8_m_all;
const int font1_height = 10;
const uint8_t *font2 = u8g2_font_t0_17b_mf; // u8g2_font_shylock_nbp_t_all;
const int font2_height = 11;
const uint8_t *font3 = u8g2_font_t0_13_mf; // u8g2_font_shylock_nbp_t_all;
const int font3_height = 9;

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

  u8g2_1.clearBuffer();
  int display_y = 0;
  display_y += font1_height + line_spacing;
  u8g2_1.setCursor(0, display_y);
  u8g2_1.print(F("wifi connected."));
  display_y += font1_height + line_spacing;
  u8g2_1.setCursor(0, display_y);
  u8g2_1.print(WiFi.localIP());
  display_y += font1_height + line_spacing;
  u8g2_1.setCursor(0, display_y);
  u8g2_1.print(macStr);
  u8g2_1.sendBuffer();

  delay(1000);
}

void setup() {
  // Initialize LED pin
  // pinMode(LED_BUILTIN, OUTPUT);
  // LED off (active low)
  // digitalWrite(LED_BUILTIN, HIGH);

  // Initialize serial port
  Serial.begin(115200);
  Serial.println("setup()");

  u8g2_1.begin();
  u8g2_1.enableUTF8Print();
  u8g2_1.setFont(font1);

  u8g2_1.clearBuffer();
  u8g2_1.setFontMode(1);
  int display_y = 0;
  display_y += font1_height + line_spacing;
  u8g2_1.setCursor(0, display_y);
  u8g2_1.print(F("init..."));
  u8g2_1.sendBuffer();

#if NUM_DISPLAYS >= 2
  u8g2_2.begin();
  u8g2_2.enableUTF8Print();
  u8g2_2.setFont(font1);

  u8g2_2.clearBuffer();
  u8g2_2.setFontMode(1);
  display_y = 0;
  display_y += font1_height + line_spacing;
  u8g2_2.setCursor(0, display_y);
  u8g2_2.print(F("init 2..."));
  u8g2_2.sendBuffer();
#endif

  setup_wifi();

  // Initialize temperature sensor
  if (!tempSensor.begin()) {
    Serial.println("Failed to find Adafruit MCP9808 chip");
    while (1) {
      delay(10);
    }
  }

  // Initialize NTP
  const int ntpUpdateIntervalMs = 10000; // 10s
  ntpSyd.ruleDST("AEDT", First, Sun, Oct, 2, 11 * 60);
  ntpSyd.ruleSTD("AEST", First, Sun, Apr, 3, 10 * 60);
  ntpSyd.updateInterval(ntpUpdateIntervalMs);
  ntpSyd.begin();
  ntpPT.ruleDST("PDT", Second, Sun, Mar, 2, -7 * 60);
  ntpPT.ruleSTD("PST", First, Sun, Nov, 3, -8 * 60);
  ntpPT.updateInterval(ntpUpdateIntervalMs);
  ntpPT.begin();
  ntpET.ruleDST("EDT", Second, Sun, Mar, 2, -4 * 60);
  ntpET.ruleSTD("EST", First, Sun, Nov, 3, -5 * 60);
  ntpET.updateInterval(ntpUpdateIntervalMs);
  ntpET.begin();
  ntpUTC.updateInterval(ntpUpdateIntervalMs);
  ntpUTC.begin();
  ntpLON.ruleDST("BST", Last, Sun, Mar, 1, 1 * 60);
  ntpLON.ruleSTD("GMT", Last, Sun, Oct, 2, 0);
  ntpLON.updateInterval(ntpUpdateIntervalMs);
  ntpLON.begin();
  ntpIndia.timeZone(5, 30);
  ntpIndia.isDST(false);
  ntpIndia.updateInterval(ntpUpdateIntervalMs);
  ntpIndia.begin();
  ntpSG.timeZone(8, 0);
  ntpSG.isDST(false);
  ntpSG.updateInterval(ntpUpdateIntervalMs);
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
  int display_y;

  sprintf(tempStr, "%.1f°C", event.temperature);

  if (numDisplays == 1) {
    u8g2_1.clearBuffer();
    display_y = 0;
    display_y += font1_height + line_spacing;
    u8g2_1.setCursor(0, display_y);
    u8g2_1.print(ntpSyd.formattedTime("%Y-%m-%d"));

    u8g2_1.setCursor(128 - u8g2_1.getUTF8Width(tempStr), display_y);
    u8g2_1.print(tempStr);

    // use a different font for the time
    u8g2_1.setFont(font2);
    display_y += font2_height + line_spacing;
    display_y += 3; // adjust
    u8g2_1.setCursor(0, display_y);
    u8g2_1.print(ntpSyd.formattedTime("%H:%M:%S"));
    if (ntpSyd.isDST()) {
      u8g2_1.println(" AEDT");
    } else {
      u8g2_1.println(" AEST");
    }
    u8g2_1.setFont(font1); // reset font

    u8g2_1.setFont(font3);
    display_y += font3_height + line_spacing;
    display_y += 8; // adjust
    u8g2_1.setCursor(0, display_y);
    u8g2_1.print(ntpPT.formattedTime("%H"));
    if (ntpPT.isDST()) {
      u8g2_1.println("pdt");
    } else {
      u8g2_1.println("pst");
    }
    u8g2_1.print(" ");
    u8g2_1.print(ntpET.formattedTime("%H"));
    u8g2_1.print("ny");
    u8g2_1.print(" ");
    u8g2_1.print(ntpUTC.formattedTime("%H"));
    u8g2_1.print("utc");
    display_y += font3_height + line_spacing;
    u8g2_1.setCursor(0, display_y);
    u8g2_1.print(ntpLON.formattedTime("%H"));
    u8g2_1.print("lon");
    u8g2_1.print(" ");
    u8g2_1.print(ntpIndia.formattedTime("%H%M"));
    u8g2_1.print("in");
    u8g2_1.print(" ");
    u8g2_1.print(ntpSG.formattedTime("%H"));
    u8g2_1.print("sg");

    u8g2_1.sendBuffer();
  } else if (numDisplays == 2) {

    u8g2_1.clearBuffer();
    display_y = 0;
    display_y += font1_height + line_spacing;
    u8g2_1.setCursor(0, display_y);
    u8g2_1.print(ntpSyd.formattedTime("%Y-%m-%d"));

    // use a different font for the time
    u8g2_1.setFont(font2);
    display_y += font2_height + line_spacing;
    display_y += 3; // adjust
    u8g2_1.setCursor(0, display_y);
    u8g2_1.print(ntpSyd.formattedTime("%H:%M:%S"));
    if (ntpSyd.isDST()) {
      u8g2_1.println(" AEDT");
    } else {
      u8g2_1.println(" AEST");
    }
    u8g2_1.setFont(font1); // reset font

    display_y += font1_height + line_spacing;
    u8g2_1.setCursor(0, display_y);
    u8g2_1.println(tempStr);


    u8g2_1.sendBuffer();


    u8g2_2.clearBuffer();
    u8g2_2.setFont(font1);
    display_y = 0;
    display_y += font1_height + line_spacing;
    display_y += 8; // adjust
    u8g2_2.setCursor(0, display_y);
    u8g2_2.print(ntpPT.formattedTime("%H"));
    if (ntpPT.isDST()) {
      u8g2_2.println(" pdt");
    } else {
      u8g2_2.println(" pst");
    }
    u8g2_2.print("  ");
    u8g2_2.print(ntpET.formattedTime("%H"));
    u8g2_2.print(" nyc");

    display_y += font1_height + line_spacing;
    u8g2_2.setCursor(0, display_y);
    u8g2_2.print(ntpUTC.formattedTime("%H"));
    u8g2_2.print(" utc");
    u8g2_2.print("  ");
    u8g2_2.print(ntpLON.formattedTime("%H"));
    u8g2_2.print(" lon");

    display_y += font1_height + line_spacing;
    u8g2_2.setCursor(0, display_y);
    u8g2_2.print(ntpIndia.formattedTime("%H%M"));
    u8g2_2.print(" in");
    u8g2_2.print("  ");
    u8g2_2.print(ntpSG.formattedTime("%H"));
    u8g2_2.print(" sg");

    u8g2_2.sendBuffer();

  } else {
    // unhandled number of displays
  }
  delay(10);
}
