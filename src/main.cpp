#include "WiFiUdp.h"
#include <Adafruit_GFX.h>
#include <Adafruit_MCP9808.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <NTP.h>
#include <SPI.h>
#include <Wire.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3D // 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
NTP ntp(wifiUdp);

const int ledPin = D4; // GPIO2=D4 for LED on ESP8266 NodeMCU/D1mini

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

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(F("wifi connected."));
  display.println(WiFi.localIP());
  display.println(macStr);
  display.display();
  delay(1000);
}

void setup() {
  // Initialize LED pin
  pinMode(ledPin, OUTPUT);

  // Initialize serial port
  Serial.begin(115200);
  Serial.println("setup()");

  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    while (1) {
      delay(10);
    }
  }

  display.cp437(true);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setCursor(5, 5);
  display.setTextSize(1);
  display.println(F("init..."));
  display.display();

  setup_wifi();

  // Initialize temperature sensor
  if (!tempSensor.begin()) {
    Serial.println("Failed to find Adafruit MCP9808 chip");
    while (1) {
      delay(10);
    }
  }

  // Initialize NTP
  ntp.ruleDST("AEDT", First, Sun, Oct, 2, 11 * 60);
  ntp.ruleSTD("AEST", First, Sun, Apr, 3, 10 * 60);
  ntp.begin();

  Serial.println("setup() done");
}

void loop() {

  sensors_event_t event;
  tempSensor.getEvent(&event);

  // Serial.print("Temperature: ");
  // Serial.print(event.temperature);
  // Serial.print(" degC");
  // Serial.println("");

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(ntp.formattedTime("%Y-%m-%d"));
  display.println(ntp.formattedTime("%H:%M:%S"));
  if (ntp.isDST()) {
    display.println("      AEDT");
  } else {
    display.println("      AEST");
  }

  display.printf("temp: %.1f C\n", event.temperature);
  display.display();
  delay(50);
}
