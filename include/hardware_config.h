#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <Arduino.h>
#include <driver/gpio.h>

/**
 * ESP32 APRS Tracker Hardware Configuration
 * 
 * Aligned with esp32-tracker project GPIO definitions.
 * 
 * WIRING DIAGRAM:
 * ===============
 * 
 * DRA818 Radio Module:
 *   - PD (Power Down)  -> GPIO 5
 *   - PTT (Push-to-Talk) -> GPIO 33
 *   - RX (Serial)      -> GPIO 18
 *   - TX (Serial)      -> GPIO 19
 *   - Audio Out (DAC1) -> GPIO 25
 *   - Audio In (ADC)   -> GPIO 36 (input-only)
 *   - Audio Trigger    -> GPIO 32 (unused in code)
 * 
 * GPS Module:
 *   - RX (ESP32 RX <- GPS TX) -> GPIO 16
 *   - TX (ESP32 TX -> GPS RX) -> GPIO 17
 * 
 * BME280 Sensor (I2C):
 *   - SDA              -> GPIO 21
 *   - SCL              -> GPIO 22
 * 
 * Console:
 *   - USB Serial (Serial0) - GPIO 3/1 (implicit)
 */

// ============================================================================
// APRS/I2S Audio Configuration
// ============================================================================
#define RADIO_AUDIO_OUT         25           // I2S DAC output to radio audio in
#define RADIO_AUDIO_IN          36           // Radio audio out (ADC, input-only)
#define RADIO_AUDIO_TRIGGER     32           // Defined but not used

// ============================================================================
// Radio (DRA818) Control Pins
// ============================================================================
#define RADIO_PTT               33           // PTT control (active LOW)
#define RADIO_PD                5            // Power down control

// ============================================================================
// Serial Port Configurations
// ============================================================================

// Serial Port 0: USB/Debug Console (UART0 - default Serial)
// RX: GPIO 3 (implicit, USB)
// TX: GPIO 1 (implicit, USB)
#define CONSOLE_BAUDRATE        115200

// Serial Port 1: Radio (DRA818) Module (HardwareSerial(1))
#define RADIO_RX                18  // ESP32 RX <- Radio TX
#define RADIO_TX                19  // ESP32 TX -> Radio RX
#define RADIO_BAUDRATE          9600

// Serial Port 2: GPS Module (HardwareSerial(2))
#define GPS_RX                  16  // ESP32 RX <- GPS TX
#define GPS_TX                  17  // ESP32 TX -> GPS RX
#define GPS_BAUDRATE            9600

// ============================================================================
// I2C Bus Configuration (for sensors like BME280)
// ============================================================================
#define I2C_SDA                 21
#define I2C_SCL                 22
#define I2C_FREQUENCY           100000  // 100kHz

// ============================================================================
// Optional/Future Use Pins
// ============================================================================
#define ONE_WIRE_EXT            13

// ============================================================================
// I2S Configuration for AFSK/APRS Audio Generation
// ============================================================================
#define I2S_NUM                 I2S_NUM_0
#define I2S_SAMPLE_RATE         105600  // 13200 * 8 (oversampling)
#define I2S_SAMPLE_BITS         I2S_BITS_PER_SAMPLE_16BIT
#define I2S_CHANNEL_FORMAT      I2S_CHANNEL_FMT_ONLY_RIGHT
#define I2S_DMA_BUF_COUNT       2
#define I2S_DMA_BUF_LEN         300

// ============================================================================
// Radio Default Configuration
// ============================================================================
#define RADIO_FREC                    144.9900  // MHz
#define RADIO_SQUELCH_LEVEL           7         // 0-8
#define RADIO_AUDIO_OUTPUT_VOLUME     8         // 1-8
#define RADIO_MIC_VOLUME              8         // 1-8

// ============================================================================
// APRS Configuration Defaults
// ============================================================================
#define APRS_SSID               15
#define OPEN_SQUELCH            false
#define DEFAULT_APRS_CALLSIGN   "VA7RCV"
#define DEFAULT_APRS_SYMBOL     'n'          // Navigation/car
#define DEFAULT_APRS_TABLE      '/'          // Primary symbol table
#define DEFAULT_PREAMBLE_MS     350          // Pre-transmission flags
#define DEFAULT_TAIL_MS         50           // Post-transmission flags

// ============================================================================
// Timing Configuration
// ============================================================================
#define APRS_TX_CYCLE_SECONDS   300          // 5 minutes between transmissions
#define GPS_UPDATE_INTERVAL_MS  1000         // Check GPS every second
#define TELEMETRY_EVERY_N_POS   3            // Send telemetry every 3rd position

// ============================================================================
// PTT Configuration
// ============================================================================
#define PTT_ACTIVE_LOW          1            // PTT is active LOW
#define APRS_PTT_PRE_MS         250          // PTT lead time before audio
#define APRS_PTT_TAIL_MS        120          // PTT tail time after audio

#endif // HARDWARE_CONFIG_H
