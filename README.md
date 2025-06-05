# esp_oled_time_temp

- ESP8266 D1 Mini
- SSD1306 OLED display ([library](https://registry.platformio.org/libraries/adafruit/Adafruit%20SSD1306/))
- MCP9808 temperature sensor ([library](https://registry.platformio.org/libraries/adafruit/Adafruit%20MCP9808%20Library))
- NTP time sync ([library](https://registry.platformio.org/libraries/sstaub/NTP))

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
