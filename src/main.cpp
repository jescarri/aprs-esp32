#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <APRS.h>
#include "hardware_config.h"
#include "RadioManager.h"

// Disable Bluetooth and WiFi to save power
#include <esp_bt.h>
#include <esp_wifi.h>

// ============================================================================
// Global Objects
// ============================================================================
APRS::APRSClient aprs;
RadioManager radio;
TinyGPSPlus gps;
Adafruit_BME280 bme;

// ============================================================================
// State Variables
// ============================================================================
unsigned long lastTransmission = 0;
unsigned int transmissionCount = 0;
bool gpsValid = false;
float lastLat = 0.0;
float lastLon = 0.0;
float lastAlt = 0.0;

// ============================================================================
// Setup Functions
// ============================================================================

void setupSerial() {
    // === Serial 0: USB Console ===
    Serial.begin(CONSOLE_BAUDRATE);
    delay(500);
    Serial.println("\n\n=================================");
    Serial.println("ESP32 APRS Tracker");
    Serial.println("=================================");
    Serial.printf("[BOARD] UARTs: Console=UART0(USB), GPS=UART2(%d/%d@%d), Radio=UART1(%d/%d@%d)\n",
                 GPS_RX, GPS_TX, GPS_BAUDRATE, RADIO_RX, RADIO_TX, RADIO_BAUDRATE);
    
    // === Serial 2: GPS Module ===
    Serial.println("Initializing GPS (Serial1)...");
    Serial1.begin(GPS_BAUDRATE, SERIAL_8N1, GPS_RX, GPS_TX);
    while (!Serial1) { /* wait */ }
    Serial1.flush();
    delay(500);
    Serial.println("✓ GPS Serial initialized");
    
    // === Serial 1: Radio Module (DRA818) ===
    Serial.println("Initializing Radio (Serial2)...");
    Serial2.begin(RADIO_BAUDRATE, SERIAL_8N1, RADIO_RX, RADIO_TX);
    while (!Serial2) { /* wait */ }
    Serial2.flush();
    delay(500);
    Serial.println("✓ Radio Serial initialized");
}

void setupSensors() {
    Serial.println("\nInitializing I2C sensors...");
    Wire.begin(I2C_SDA, I2C_SCL);
    
    if (!bme.begin(0x76)) {
        Serial.println("⚠ BME280 sensor not found!");
    } else {
        Serial.println("✓ BME280 sensor initialized");
    }
}

void setupRadio() {
    Serial.println("\nInitializing DRA818 Radio...");
    Serial.printf("[GPIO] PTT=%d PD=%d DAC_OUT=%d\n", RADIO_PTT, RADIO_PD, RADIO_AUDIO_OUT);
    
    // Set up radio control pins
    pinMode(RADIO_PD, OUTPUT);
    pinMode(RADIO_PTT, OUTPUT);
    digitalWrite(RADIO_PD, HIGH);  // Power on
    digitalWrite(RADIO_PTT, PTT_ACTIVE_LOW ? HIGH : LOW);  // PTT off (idle)
    
    delay(100);  // Give radio time to power up
    
    // Configure radio
    RadioManager::RadioConfig radioConfig;
    radioConfig.frequency = RADIO_FREC;
    radioConfig.squelch_level = RADIO_SQUELCH_LEVEL;
    radioConfig.volume = RADIO_AUDIO_OUTPUT_VOLUME;
    radioConfig.mic_gain = RADIO_MIC_VOLUME;
    radioConfig.narrow_band = false;
    radioConfig.rx_enable = true;
    radioConfig.tx_enable = true;
    
    if (radio.begin(&Serial2, (gpio_num_t)RADIO_PD, (gpio_num_t)RADIO_PTT, radioConfig)) {
        Serial.println("✓ Radio initialized successfully");
        Serial.printf("[RADIO] Freq=%.4f MHz SQ=%d Mic=%d AF=%d PTTpol=%s PD=HIGH\n",
                     RADIO_FREC, RADIO_SQUELCH_LEVEL, RADIO_MIC_VOLUME,
                     RADIO_AUDIO_OUTPUT_VOLUME, PTT_ACTIVE_LOW ? "ACTIVE_LOW" : "ACTIVE_HIGH");
    } else {
        Serial.println("✗ Radio initialization FAILED!");
    }
}

void setupAPRS() {
    Serial.println("\nInitializing APRS...");
    
    APRS::Config aprsConfig;
    aprsConfig.callsign = DEFAULT_APRS_CALLSIGN;
    aprsConfig.ssid = APRS_SSID;
    aprsConfig.path1 = "WIDE1";
    aprsConfig.path1_ssid = 1;
    aprsConfig.path2 = "WIDE2";
    aprsConfig.path2_ssid = 2;
    aprsConfig.symbol = DEFAULT_APRS_SYMBOL;
    aprsConfig.symbol_table = DEFAULT_APRS_TABLE;
    aprsConfig.preamble_ms = DEFAULT_PREAMBLE_MS;
    aprsConfig.tail_ms = DEFAULT_TAIL_MS;
    aprsConfig.ptt_pin = RADIO_PTT;
    
    if (aprs.begin(aprsConfig)) {
        Serial.println("✓ APRS initialized");
        Serial.printf("  Callsign: %s-%d\n", aprsConfig.callsign, aprsConfig.ssid);
        Serial.printf("  Path: %s-%d,%s-%d\n", 
                     aprsConfig.path1, aprsConfig.path1_ssid,
                     aprsConfig.path2, aprsConfig.path2_ssid);
        Serial.printf("  Symbol: %c (table %c)\n", aprsConfig.symbol, aprsConfig.symbol_table);
    } else {
        Serial.println("✗ APRS initialization FAILED!");
    }
}

// ============================================================================
// GPS Processing
// ============================================================================

void updateGPS() {
    while (Serial1.available() > 0) {
        char c = Serial1.read();
        if (gps.encode(c)) {
            if (gps.location.isValid()) {
                gpsValid = true;
                lastLat = gps.location.lat();
                lastLon = gps.location.lng();
                
                if (gps.altitude.isValid()) {
                    lastAlt = gps.altitude.meters();
                }
                
                // Print GPS info occasionally
                static unsigned long lastPrint = 0;
                if (millis() - lastPrint > 10000) {  // Every 10 seconds
                    lastPrint = millis();
                    Serial.printf("\n[GPS] Lat: %.6f, Lon: %.6f, Alt: %.1fm, Sats: %d\n",
                                 lastLat, lastLon, lastAlt, gps.satellites.value());
                }
            } else {
                gpsValid = false;
            }
        }
    }
}

// ============================================================================
// APRS Transmission
// ============================================================================

void sendAPRSPosition() {
    Serial.println("\n--- Sending APRS Position ---");
    
    String comment = "ESP32-Tracker";
    if (!gpsValid) {
        comment += " GPS-INVALID";
    }
    
    if (aprs.sendPosition(lastLat, lastLon, comment.c_str(), 1, 1, 1, 0)) {
        Serial.println("✓ Position sent successfully");
    } else {
        Serial.println("✗ Position transmission failed");
    }
}

void sendAPRSTelemetry() {
    Serial.println("\n--- Sending APRS Telemetry ---");
    
    // Read sensors
    APRS::TelemetryData telem;
    telem.analog[0] = 3.7;  // Battery voltage (TODO: read from ADC)
    telem.analog[1] = bme.readTemperature();
    telem.analog[2] = bme.readPressure() / 100.0;  // Convert Pa to mbar
    telem.analog[3] = bme.readHumidity();
    telem.analog[4] = lastAlt;
    telem.digital = 0;  // No digital channels used
    
    Serial.printf("  Battery: %.2fV\n", telem.analog[0]);
    Serial.printf("  Temp: %.1f°C\n", telem.analog[1]);
    Serial.printf("  Pressure: %.1fmbar\n", telem.analog[2]);
    Serial.printf("  Humidity: %.1f%%\n", telem.analog[3]);
    Serial.printf("  Altitude: %.1fm\n", telem.analog[4]);
    
    if (aprs.sendTelemetry(telem)) {
        Serial.println("✓ Telemetry sent successfully");
    } else {
        Serial.println("✗ Telemetry transmission failed");
    }
}

void transmitAPRS() {
    unsigned long now = millis();
    
    // Check if it's time to transmit
    if (now - lastTransmission < APRS_TX_INTERVAL_MS && lastTransmission != 0) {
        return;  // Not time yet
    }
    
    Serial.println("\n=====================================");
    Serial.printf("Transmission #%d\n", transmissionCount + 1);
    Serial.println("=====================================");
    
    // Send position
    sendAPRSPosition();
    delay(2000);  // Wait between packets
    
    // Send telemetry every 3rd transmission
    if (transmissionCount % TELEMETRY_EVERY_N_POS == 0) {
        // Send telemetry definitions first time
        if (transmissionCount == 0) {
            Serial.println("\n--- Sending Telemetry Definitions ---");
            if (aprs.sendTelemetryDefinitions()) {
                Serial.println("✓ Telemetry definitions sent");
            }
            delay(2000);
        }
        
        sendAPRSTelemetry();
    }
    
    lastTransmission = now;
    transmissionCount++;
    
    Serial.printf("\nNext transmission in %d seconds\n", APRS_TX_INTERVAL_MS / 1000);
    Serial.println("=====================================\n");
}

// ============================================================================
// Main Setup and Loop
// ============================================================================

void setup() {
    // Disable BT and WiFi to save power
    btStop();
    esp_wifi_stop();
    esp_bt_controller_disable();
    
    // Initialize all subsystems
    setupSerial();
    setupSensors();
    setupRadio();
    setupAPRS();
    
    Serial.println("\n✓ All systems initialized!");
    Serial.println("Waiting for GPS lock...\n");
    
    // Set default position (will be overwritten when GPS locks)
    lastLat = 49.102421;
    lastLon = -122.653579;
    lastAlt = 100.0;
}

void loop() {
    // Update GPS data
    updateGPS();
    
    // Transmit when ready
    transmitAPRS();
    
    // Small delay to prevent watchdog issues
    delay(100);
}
