-include .env

ifndef WIFI_SSID
$(error WIFI_SSID is not set)
endif
ifndef WIFI_PASS
$(error WIFI_PASS is not set)
endif

PLATFORMIO_BUILD_FLAGS=-DWIFI_SSID=${WIFI_SSID} -DWIFI_PASS=${WIFI_PASS}
export

build:
	uv run pio run

flash:
	uv run pio run --target upload -e d1_mini

clean:
	rm -r .pio/build
