# ESP32 APRS Tracker - Refactored Library

A clean, modern C++ implementation of APRS for ESP32 with proper encapsulation, type-safe APIs, and no global state anti-patterns.

## Features

✅ **Clean Modern API** - No more manual string formatting or global variables  
✅ **Automatic Coordinate Conversion** - Pass float lat/lon directly  
✅ **Type-Safe Telemetry** - Structured data instead of manual packet construction  
✅ **Modular Design** - Separate concerns for protocol, position, and telemetry  
✅ **No Decoder Code** - Transmit-only, optimized for trackers  
✅ **Full Hardware Abstraction** - Configurable GPIO pins  
✅ **Three Serial Ports** - Console (USB), GPS, and Radio properly managed  

## Hardware Requirements

- **ESP32 Board**: NodeMCU-32S or compatible
- **Radio**: DRA818V/U VHF/UHF transceiver
- **GPS**: Any NMEA-compatible GPS module (Serial)
- **Sensor** (optional): BME280 for temperature/humidity/pressure

## Wiring Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                         ESP32 NodeMCU-32S                      │
│                                                                │
│  USB ─────────────────── Serial0 (Console Debug)              │
│                                                                │
│  GPIO 16 (RX1) ────────┬─ GPS Module TX                      │
│  GPIO 17 (TX1) ────────┴─ GPS Module RX                      │
│                                                                │
│  GPIO 18 (RX2) ────────┬─ DRA818 TX                          │
│  GPIO 19 (TX2) ────────┴─ DRA818 RX                          │
│                                                                │
│  GPIO 25 (I2S DAC) ─────── DRA818 Audio In                    │
│  GPIO 33 (PTT) ─────────── DRA818 PTT                         │
│  GPIO 5 (PD) ───────────── DRA818 Power Down                  │
│  GPIO 26 (TX POW) ──────── DRA818 TX Power Select             │
│                                                                │
│  GPIO 21 (SDA) ────────┬─ BME280 SDA                         │
│  GPIO 22 (SCL) ────────┴─ BME280 SCL                         │
│                                                                │
└──────────────────────────────────────────────────────────────┘
```

### Serial Port Summary

| Port | ESP32 Pins | Device | Baudrate | Purpose |
|------|-----------|---------|----------|---------|
| **Serial0** | GPIO 3/1 (USB) | Console | 115200 | Debug output |
| **Serial1** | GPIO 16/17 | GPS Module | 9600 | NMEA sentences |
| **Serial2** | GPIO 18/19 | DRA818 Radio | 9600 | AT commands |

### Pin Definitions

All pin definitions are in `include/hardware_config.h` and can be easily customized.

## Installation

### PlatformIO

1. Clone or copy this project to your workspace
2. Open in PlatformIO
3. **IMPORTANT**: Edit `src/main.cpp` and change the callsign:
   ```cpp
   aprsConfig.callsign = "YOUR_CALL";  // ⚠ CHANGE THIS!
   aprsConfig.ssid = 15;
   ```
4. Build and upload:
   ```bash
   pio run -t upload
   pio device monitor
   ```

## Usage Examples

### Basic Position Report

```cpp
#include <APRS.h>

APRS::APRSClient aprs;

void setup() {
    APRS::Config config;
    config.callsign = "VA7RCV";
    config.ssid = 15;
    
    aprs.begin(config);
    
    // Send position - coordinates converted automatically!
    aprs.sendPosition(49.1023, -122.6365, "ESP32 Tracker");
}
```

### Telemetry with Structured Data

```cpp
// No more manual string formatting!
APRS::TelemetryData telem;
telem.analog[0] = 3.7;      // Battery voltage
telem.analog[1] = 22.5;     // Temperature
telem.analog[2] = 1013.2;   // Pressure
telem.analog[3] = 65.0;     // Humidity
telem.analog[4] = 123.0;    // Altitude
telem.digital = 0;

aprs.sendTelemetry(telem);  // Automatic sequence numbering
```

### Complete Tracker Example

See `src/main.cpp` for a full working example with:
- All 3 serial ports properly initialized
- GPS integration
- BME280 sensor reading
- Periodic transmission
- Telemetry definitions

## API Reference

### APRS::APRSClient

#### Configuration

```cpp
struct Config {
    const char* callsign;
    uint8_t ssid;
    const char* path1;
    uint8_t path1_ssid;
    const char* path2;
    uint8_t path2_ssid;
    char symbol;              // APRS symbol
    char symbol_table;        // '/' or '\\'
    uint16_t preamble_ms;
    uint16_t tail_ms;
    gpio_num_t ptt_pin;
};
```

#### Methods

| Method | Description |
|--------|-------------|
| `bool begin(const Config&)` | Initialize APRS |
| `bool sendPosition(lat, lon, comment)` | Send position (auto-converts coordinates) |
| `bool sendTelemetry(const TelemetryData&)` | Send telemetry with structured data |
| `bool sendTelemetryDefinitions()` | Send PARM and UNIT packets |
| `bool sendMessage(const char*)` | Send text message |
| `bool isBusy()` | Check if transmitting |

### APRS::TelemetryData

```cpp
struct TelemetryData {
    float analog[5];    // 5 analog channels
    uint8_t digital;    // 8 digital bits
};
```

Standard channel mapping:
- `analog[0]` - Battery voltage
- `analog[1]` - Temperature
- `analog[2]` - Pressure
- `analog[3]` - Humidity
- `analog[4]` - Altitude

## Comparison: Old vs New API

### Position Updates

**Old (Original Library):**
```cpp
// Manual string formatting required
char lat[9], lon[10];
locationUpdate(gps.location.lat(), gps.location.lng());  // Complex function
APRS_setLat(lat);
APRS_setLon(lon);
APRS_setPower(1);
APRS_setHeight(10);
APRS_setGain(1);
APRS_setDirectivity(0);
APRS_sendLoc(comment, strlen(comment));
```

**New (Refactored):**
```cpp
// Single function call with float coordinates
aprs.sendPosition(gps.location.lat(), gps.location.lng(), 
                 "Comment", 1, 10, 1, 0);
```

### Telemetry

**Old (Original Library):**
```cpp
// Manual packet construction
char telempkt[200];
snprintf(telempkt, sizeof(telempkt), 
         "T#%03d,%s,%s,%s,%s,%s,00000000",
         telemId, vBat, temp, pressure, humidity, altitude);
APRS_sendPkt(&telempkt, strlen(telempkt));
```

**New (Refactored):**
```cpp
// Type-safe structured data
APRS::TelemetryData telem;
telem.analog[0] = battery_voltage;
telem.analog[1] = temperature;
// ... etc
aprs.sendTelemetry(telem);  // Handles formatting automatically
```

## Benefits of Refactoring

| Aspect | Old Library | New Library |
|--------|-------------|-------------|
| **Lines of Code** | ~150 lines for position/telemetry | ~15 lines |
| **Memory Safety** | Manual string management | Automatic bounds checking |
| **Coordinate Input** | Pre-formatted strings | Float lat/lon |
| **Telemetry** | Manual packet construction | Structured data |
| **State Management** | Global variables | Encapsulated in class |
| **Initialization** | Complex multi-step | Single `begin()` call |
| **Error Handling** | void functions | Returns bool |
| **Decoder Code** | Included (unused) | Removed |

## APRS Configuration Reference

### SSID Values (Station Types)

The SSID field (0-15) indicates the type of station. Use these standard values:

| SSID | Icon | Description | Typical Use |
|------|------|-------------|-------------|
| **0** | 🏠 | Primary Station | Home/fixed station |
| **1** | 📡 | Generic | Generic additional station |
| **2** | 📡 | Generic | Generic additional station |
| **3** | 📡 | Generic | Generic additional station |
| **4** | 📡 | Generic | Generic additional station |
| **5** | 🌐 | Other Networks | Gateway/IGate |
| **6** | 📡 | Special Activity | Satellite operations |
| **7** | 📱 | Handheld | Walkie-talkie/HT |
| **8** | ⛵ | Boats/Maritime | Ships, boats, maritime mobile |
| **9** | 🚗 | Mobile | Primary vehicle |
| **10** | 💻 | Internet/D-STAR | Internet/D-STAR connected |
| **11** | 🎈 | Balloons | High-altitude balloons |
| **12** | 🎒 | Portable | Camping, field operations |
| **13** | 🌦️ | Weather Station | Automatic weather station |
| **14** | 🚚 | Trucking | Trucking/shipping |
| **15** | ➕ | Generic Additional | Generic additional/experimental |

**Example Configuration:**
```cpp
config.ssid = 9;  // Mobile vehicle tracker
```

### APRS Symbols

The symbol character defines what icon appears on APRS maps. Common symbols:

#### Vehicles
| Code | Symbol | Description |
|------|--------|-------------|
| `n` | 🚗 | Car/Truck (navigation) |
| `>` | 🚙 | Small car |
| `v` | 🚐 | Van |
| `j` | 🚙 | Jeep |
| `u` | 🚚 | Truck (large) |
| `R` | 🚘 | Recreational Vehicle (RV) |
| `b` | 🚲 | Bicycle |

#### Aircraft & Marine
| Code | Symbol | Description |
|------|--------|-------------|
| `'` | ✈️ | Small aircraft |
| `^` | 🛩️ | Large aircraft |
| `X` | 🚁 | Helicopter |
| `k` | 🚢 | Ship/boat (large) |
| `s` | ⛵ | Sailboat |
| `Y` | ⛵ | Yacht |

#### People & Buildings
| Code | Symbol | Description |
|------|--------|-------------|
| `[` | 👤 | Person (jogger) |
| `-` | 🏠 | House |
| `a` | 🚑 | Ambulance |
| `f` | 🚒 | Fire truck |
| `!` | 🚨 | Police/Emergency |

#### Weather & Special
| Code | Symbol | Description |
|------|--------|-------------|
| `_` | 🌦️ | Weather station |
| `O` | 🎈 | Balloon |
| `/` | ⚫ | Dot (generic position) |

### Symbol Tables

APRS has two symbol tables:

| Table | Character | Description |
|-------|-----------|-------------|
| **Primary** | `/` | Most common symbols (default) |
| **Alternate** | `\` | Alternate/overlaid symbols |

**Example Configuration:**
```cpp
config.symbol = 'n';           // Car symbol
config.symbol_table = '/';     // Primary table
```

### Complete Configuration Example

```cpp
APRS::Config config;
config.callsign = "VA7RCV";    // Your callsign
config.ssid = 9;                // Mobile vehicle
config.symbol = 'n';            // Car icon
config.symbol_table = '/';      // Primary table
config.path1 = "WIDE1";
config.path1_ssid = 1;
config.path2 = "WIDE2";
config.path2_ssid = 2;
```

## Hardware Configuration

Edit `include/hardware_config.h` to customize:
- GPIO pin assignments
- Radio frequency
- Transmission intervals
- APRS defaults

## Troubleshooting

### Radio Not Initializing

- Check Serial2 wiring (GPIO 18/19)
- Verify DRA818 power supply (7-9V)
- Ensure PD pin is HIGH

### No GPS Lock

- GPS needs clear sky view
- Cold start can take 5-10 minutes
- Check Serial1 wiring (GPIO 16/17)
- Monitor Serial console for GPS data

### I2S Audio Issues

- GPIO 25 is the only DAC pin on ESP32
- Check audio coupling to radio
- Verify radio audio input levels

### APRS Not Decoding

- Verify callsign is set correctly
- Check radio frequency matches APRS frequency
- Ensure proper audio levels to radio
- Verify PTT control is working

## License

This refactored code is based on the original LibAPRS-esp32-i2s library.
Refactoring and improvements by the community.

## Credits

- Original LibAPRS: Mark Qvist
- ESP32 Port: Various contributors
- Refactoring: Community effort
- DRA818 Library: jescarri

## Contributing

Contributions welcome! Areas for improvement:
- [ ] Add more APRS packet types (weather, objects)
- [ ] Implement APRS messaging
- [ ] Add GPS filtering (smart beaconing)
- [ ] Power management optimizations
- [ ] Unit tests

## Support

For issues specific to this refactored version, please file an issue with:
- Hardware configuration
- Serial monitor output
- PlatformIO version
- ESP32 board type
