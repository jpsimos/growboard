# growboard

**Greenroom companion for the Adafruit Feather M0 WiFi**

Author: Jacob A. Psimos  
Repository: [github.com/jpsimos/growboard](https://github.com/jpsimos/growboard)

---

Greenroom companion board that controls relays and takes environmental measurements (temperature, humidity, air quality, light).  
Supports an SHT4x hygrometer, CCS811 air quality sensor, light sensor, RTC + SD card, and a Wi-Fi client with an embedded HTTP server exposing a basic RESTful API.

## Features

- Real-Time Clock (I2C PCF8523) + SD card (FAT32 via SdFat) for configuration, logging, and air-quality baseline persistence
- SHT4X digital hygrometer (temperature + relative humidity) with min/max tracking
- CCS811 air quality sensor (eCO₂ and TVOC) with baseline save/load and environmental compensation
- Light / multi-sensor readings (lux, temperature, pressure, humidity, altitude) via I²C device at address `0x5B`
- Relay control with debounce
- Wi-Fi client (ATWINC1500 / WiFi101) with HTTP server supporting basic RESTful API requests
- Modular middleware architecture with singleton instances and a central `ForEach` dispatcher
- Simple HTML template (`Index.html`) served over HTTP; JavaScript + C++ handling of GET `command=` parameters
- Configuration (`Config.txt`) loaded from the SD card at startup
- Watchdog timer support (Adafruit SleepyDog)

## Hardware

Built for the **Adafruit Feather M0 WiFi** (ATSAMD21 + ATWINC1500).

Key pin assignments (from the middleware headers):

| Function              | Pin |
|-----------------------|-----|
| Relay                 | 5   |
| RTC SQW (optional)    | 6   |
| WiFi IRQ              | 7   |
| WiFi CS               | 8   |
| SD Card CS            | 10  |
| WiFi RST              | 4   |
| WiFi EN               | 2   |

## Requirements

- Arduino IDE (or compatible) configured for **C++17** using the fork [github.com/jpsimos/ArduinoCore-samd](https://github.com/jpsimos/ArduinoCore-samd)
- Required WiFi101 fork: [github.com/jpsimos/WiFi101](https://github.com/jpsimos/WiFi101)
- SD card formatted as **FAT32**
- Adafruit libraries: `Adafruit_CCS811`, `Adafruit_SHT4x`, `RTClib`, `SdFat`, `elapsedMillis`, `Adafruit_SleepyDog`, `WiFi101`
- Configuration text file and HTML template placed on the SD card (see **Configuration**)

## Configuration

Wi-Fi credentials and other settings are read from `/Config.txt` on the SD card at startup by `SdMiddleware::LoadConfigTxt`.

Example (`Config.txt`):

```
ssid=YourNetworkName
key=YourPassword
autoconnect=true
autoreconnect=true
```

The HTML interface is served from `/Index.html` on the SD card when a GET request has no query parameters.

Air-quality baseline is persisted as a 2-byte file `/AirQualityBaseline.bin`.

## Architecture

The sketch uses a middleware pattern. Each component is a singleton that inherits from `Middleware`, registers itself, implements `Loop()`, and optionally handles serial and RESTful commands.

Components registered in `growboard.ino`:

- `RtcMiddleware` — I2C RTC for timekeeping and filesystem timestamps
- `SdMiddleware` — SD card access, config loading, baseline persistence, logging
- `WifiMiddleware` — Wi-Fi connection + HTTP server / REST API
- `HygrometerMiddleware` — SHT4x sensor
- `RelayMiddleware` — Relay control
- `AirQualityMiddleware` — CCS811 air quality sensing with calibration state machine
- `LightMiddleware` — Light / environmental multi-sensor (I²C 0x5B)

Main sketch: `growboard.ino`  
The main loop calls `Loop()` on every registered middleware, processes serial input, and resets the watchdog only if all middlewares report healthy.

### Serial / REST commands

Commands are plain text strings (case-sensitive unless noted). Serial responses are human-readable; REST responses are JSON.

| Command          | Serial / REST | Description |
|------------------|---------------|-------------|
| `hyg read` / `hyg status` | Both | Temperature (°F) and humidity (% rH); status also shows min/max |
| `aq status`      | Both | Calibration / baseline status |
| `aq read`        | Both | eCO₂ and TVOC |
| `aq cal`         | Both | Start calibration procedure |
| `relay on` / `relay off` / `relay status` | Both | Control or query relay |
| `rtc status` / `rtc now` / `rtc set MM/DD/YYYY HH:MM:SS` | Both | RTC control |
| `wifi scan` / `wifi status` | Serial | Wi-Fi diagnostics |
| `sd status`      | Serial | SD card information |
| `light read`     | Both | Lux, temperature, pressure, humidity, altitude |

HTTP usage example:  
`GET /?command=aq%20read`  
or use the served HTML interface.

## Differences from GitHub repository

The attached files differ from the current `main` branch at [github.com/jpsimos/growboard](https://github.com/jpsimos/growboard) in the following ways:

- **New component**: `LightMiddleware` (`.cpp` / `.h`) is present and registered. The published `Middleware.h` does not yet define `LIGHT_SENSOR`, and `growboard.ino` on GitHub does not include or register it.
- **AirQualityMiddleware**: The attached version implements a full calibration state machine (`Calibrate`, timers for warm-up / calibration / environment updates, failure flags). The published version uses a simpler `Configure()` method that loads or saves a baseline once.
- **Middleware.h**: Attached version adds `LIGHT_SENSOR` to the `MIDDLEWARE_TYPES` macro.
- **Templates**: `Config.txt` and `Index.html` are provided at the root in the attachment set; on GitHub they currently live in the `card/` subdirectory.
- Other middleware files (`Hygrometer`, `Relay`, `Rtc`, `Sd`, `Wifi`, `Common`) appear largely consistent with the published versions, with the air-quality and light additions being the primary functional differences.

These changes make the attached tree ready for a commit that adds light-sensor support and refines air-quality calibration.

## Notes

In the United States, Wi-Fi channels 12–14 are generally prohibited. The Atmel BSP / WiFi101 fork should be modified to mask those channels.

CCS811 warm-up is 20 minutes; full calibration baseline capture is 24 hours (see defines in `AirQualityMiddleware.h`).

## Getting Started

1. Install the Adafruit SAMD board package (or the C++17 fork) and select the Feather M0 WiFi board.
2. Ensure the Arduino IDE is set to use C++17 and install the required libraries and WiFi101 fork.
3. Copy `Config.txt` to the root of a FAT32 SD card and edit it with your Wi-Fi credentials.
4. Copy `Index.html` to the root of the same SD card.
5. Open `growboard.ino`, compile, and upload.
6. The board will connect to Wi-Fi (if autoconnect is enabled) and serve the HTTP interface / REST endpoints. Serial commands are available at 9600 baud.

Happy hacking.
