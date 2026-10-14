# growboard

**Greenroom companion for the Adafruit Feather M0 WiFi**

Author: Jacob A. Psimos  
Repository: [github.com/jpsimos/growboard](https://github.com/jpsimos/growboard)

---

Greenroom companion board that controls relays and takes air quality measurements.  
Supports an SHT4x hygrometer, RTC + SD card, and a Wi-Fi client with an embedded HTTP server exposing a basic RESTful API.

## Features

- Real-Time Clock (I2C) + SD card (FAT32) for configuration and logging
- SHT4X digital hygrometer
- Air quality measurements
- Relay control
- Wi-Fi client with HTTP server supporting basic RESTful API requests
- Modular middleware architecture
- Simple HTML template served over HTTP (JavaScript + C++ handling of GET requests)
- Configuration loaded from a text file on the SD card

## Hardware

Built for the **Adafruit Feather M0 WiFi** (ATSAMD21 + ATWINC1500).

## Requirements

- Arduino IDE configured for **C++17** [github.com/jpsimos/ArduinoCore-samd](https://github.com/jpsimos/ArduinoCore-samd)
- SD card formatted as **FAT32**
- Configuration text file placed on the SD card (a template is provided with the project; place it on the card)

## Configuration

Wi-Fi credentials and other settings are read from a text file on the SD card at startup.  
The included template should be copied to a FAT32-formatted SD card and edited with your network details before first use.

## Architecture

The sketch uses a middleware pattern. Components are registered at startup:

- `RtcMiddleware` — I2C RTC for timekeeping and filesystem timestamps
- `SdMiddleware` — SD card access
- `WifiMiddleware` — Wi-Fi connection + HTTP server / REST API
- `HygrometerMiddleware` — SHT4x sensor
- `RelayMiddleware` — Relay control
- `AirQualityMiddleware` — Air quality sensing

Main sketch: `growboard.ino`

## Notes

In the United States, Wi-Fi channels 12–14 are generally prohibited. The Atmel BSP should be modified to mask those channels.  

Required WiFi101 fork at [github.com/jpsimos/WiFi101](https://github.com/jpsimos/WiFi101)

## Getting Started

1. Install the Adafruit SAMD board package and select the Feather M0 WiFi board.
2. Ensure the Arduino IDE is set to use C++17.
3. Place the config template on a FAT32 SD card and edit it with your Wi-Fi credentials.
4. Open `growboard.ino`, compile, and upload.
5. The board will connect to Wi-Fi and serve the HTTP interface / REST endpoints.

Happy hacking.
