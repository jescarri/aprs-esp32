# Quick Start Guide - ESP32 APRS Tracker

## 5-Minute Setup

### 1. Hardware Connections

Connect exactly as specified:
- **GPS**: RX1=GPIO16, TX1=GPIO17
- **Radio**: RX2=GPIO18, TX2=GPIO19, PTT=GPIO33, Audio=GPIO25
- **Power**: Ensure DRA818 has 7-9V supply

### 2. Change Your Callsign

Edit `src/main.cpp` line 103:
```cpp
aprsConfig.callsign = "YOUR_CALL";  // ⚠ REQUIRED!
aprsConfig.ssid = 15;
```

### 3. Build and Upload

```bash
cd /home/jescarri/workspace/iot/aprs-esp32
pio run -t upload
pio device monitor
```

### 4. Expected Output

```
=================================
ESP32 APRS Tracker
=================================
✓ GPS Serial initialized
✓ Radio Serial initialized
✓ BME280 sensor initialized
✓ Radio initialized successfully
✓ APRS initialized
  Callsign: YOUR_CALL-15
  
Waiting for GPS lock...
```

## Key Differences from Original Library

| Task | Old Way | New Way |
|------|---------|---------|
| **Position** | 8 function calls + manual formatting | `aprs.sendPosition(lat, lon, "comment")` |
| **Telemetry** | Manual snprintf packet building | `aprs.sendTelemetry(data)` |
| **Init** | Multiple global function calls | `aprs.begin(config)` |

## Serial Port Configuration

**YOU MUST** initialize all 3 serial ports in this order:

```cpp
// 1. Console (USB) - for debugging
Serial.begin(115200);

// 2. GPS Module
Serial1.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

// 3. Radio (DRA818)
Serial2.begin(9600, SERIAL_8N1, RADIO_RX_PIN, RADIO_TX_PIN);
```

## Common Issues

### "Radio initialization FAILED"
→ Check Serial2 wiring (GPIO 18/19) and power supply

### "GPS-INVALID" in transmission
→ GPS needs sky view, wait 5-10 minutes for cold start

### No audio on radio
→ GPIO 25 must connect to radio audio input

### PTT not working
→ GPIO 33 should be wired to radio PTT pin (active LOW)

## API Cheat Sheet

```cpp
// Initialize
APRS::APRSClient aprs;
APRS::Config config;
config.callsign = "MYCALL";
config.ssid = 15;
aprs.begin(config);

// Send position
aprs.sendPosition(latitude, longitude, "Comment");

// Send telemetry
APRS::TelemetryData telem;
telem.analog[0] = battery_voltage;
telem.analog[1] = temperature;
telem.analog[2] = pressure;
telem.analog[3] = humidity;
telem.analog[4] = altitude;
aprs.sendTelemetry(telem);

// Check if busy
if (!aprs.isBusy()) {
    // Ready to transmit
}
```

## Radio Frequency

Default: **144.990 MHz** (defined in `hardware_config.h`)

To change, edit:
```cpp
#define DEFAULT_RADIO_FREQ 144.9900
```

## Need Help?

1. Check serial monitor output
2. Verify all 3 serial ports initialized
3. Confirm callsign is set (not "NOCALL")
4. Check GPIO wiring matches hardware_config.h
5. See README.md for full troubleshooting

## What's Next?

- Adjust transmission interval: `APRS_TX_INTERVAL_MS` in hardware_config.h
- Change APRS symbol: `config.symbol = 'n'` (car/nav)
- Add smart beaconing
- Implement custom telemetry scaling
