# esp_oled_time_temp

- ESP8266 D1 Mini
- SSD1306 OLED display
- MCP9808 temperature sensor
- NTP time sync

![](esp_oled_time_temp.jpg)

## Building and Flashing

Build only

```
PLATFORMIO_BUILD_FLAGS="-DWIFI_SSID=some_wifi_ssid -DWIFI_PASS=some_wifi_password" \
  uv run pio run
```

Build and flash

```
PLATFORMIO_BUILD_FLAGS="-DWIFI_SSID=some_wifi_ssid -DWIFI_PASS=some_wifi_password" \
  uv run pio run --target upload -e d1_mini
```

Serial console (using screen; exit with `Ctrl-A \`)

```
screen /dev/ttyUSB0 115200
```
