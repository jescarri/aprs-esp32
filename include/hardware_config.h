#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <Arduino.h>
#include <driver/gpio.h>

/**
 * ESP32 APRS Tracker Hardware Configuration
 * 
 * This file contains all GPIO pin assignments and hardware configuration
 * for the ESP32 APRS tracker based on NodeMCU-32S board.
 * 
 * WIRING DIAGRAM:
 * ===============
 * 
 * DRA818 Radio Module:
 *   - PD (Power Down)  -> GPIO 5
 *   - PTT (Push-to-Talk) -> GPIO 33
 *   - TX Power Control -> GPIO 26
 *   - RX (Serial)      -> GPIO 18
 *   - TX (Serial)      -> GPIO 19
 *   - Audio In         -> GPIO 25 (I2S DAC)
 * 
 * GPS Module:
 *   - RX (GPS TX)      -> GPIO 16
 *   - TX (GPS RX)      -> GPIO 17
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
#define APRS_AUDIO_OUT_PIN      GPIO_NUM_25  // I2S DAC output to radio audio in
#define APRS_PTT_PIN            GPIO_NUM_33  // Push-to-talk control

// ============================================================================
// Radio (DRA818) Control Pins
// ============================================================================
#define RADIO_PTT_PIN           GPIO_NUM_33  // PTT control (same as APRS_PTT_PIN)
#define RADIO_PD_PIN            GPIO_NUM_5   // Power down control
#define RADIO_TX_POW_PIN        GPIO_NUM_26  // TX power control (0=LOW, 1=HIGH)

// ============================================================================
// Serial Port Configurations
// ============================================================================

// Serial Port 0: USB/Debug Console
// RX: GPIO 3 (implicit, USB)
// TX: GPIO 1 (implicit, USB)
#define CONSOLE_BAUDRATE        115200

// Serial Port 1: GPS Module
#define GPS_RX_PIN              16  // ESP32 RX <- GPS TX
#define GPS_TX_PIN              17  // ESP32 TX -> GPS RX
#define GPS_BAUDRATE            9600

// Serial Port 2: Radio (DRA818) Module
#define RADIO_RX_PIN            18  // ESP32 RX <- Radio TX
#define RADIO_TX_PIN            19  // ESP32 TX -> Radio RX
#define RADIO_BAUDRATE          9600

// ============================================================================
// I2C Bus Configuration (for sensors like BME280)
// ============================================================================
#define I2C_SDA_PIN             21
#define I2C_SCL_PIN             22
#define I2C_FREQUENCY           100000  // 100kHz

// ============================================================================
// Optional/Future Use Pins
// ============================================================================
#define ONE_WIRE_EXT_PIN        13

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
#define DEFAULT_RADIO_FREQ      144.9900     // MHz
#define DEFAULT_RADIO_SQUELCH   7            // 0-8
#define DEFAULT_RADIO_VOLUME    8            // 1-8
#define DEFAULT_RADIO_MIC_GAIN  8            // 1-8
#define DEFAULT_RADIO_TX_POWER  0            // 0=LOW (0.5W), 1=HIGH (1W)

// ============================================================================
// APRS Configuration Defaults
// ============================================================================
#define DEFAULT_APRS_CALLSIGN   "NOCALL"
#define DEFAULT_APRS_SSID       15
#define DEFAULT_APRS_SYMBOL     'n'          // Navigation/car
#define DEFAULT_APRS_TABLE      '/'          // Primary symbol table
#define DEFAULT_PREAMBLE_MS     350          // Pre-transmission flags
#define DEFAULT_TAIL_MS         50           // Post-transmission flags

// ============================================================================
// Timing Configuration
// ============================================================================
#define APRS_TX_INTERVAL_MS     300000       // 5 minutes between transmissions
#define GPS_UPDATE_INTERVAL_MS  1000         // Check GPS every second
#define TELEMETRY_EVERY_N_POS   3            // Send telemetry every 3rd position

#endif // HARDWARE_CONFIG_H
