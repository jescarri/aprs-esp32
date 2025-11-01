#ifndef APRS_CONFIG_H
#define APRS_CONFIG_H

#include <Arduino.h>

/**
 * APRS Configuration Structure
 * 
 * Contains all user-configurable APRS parameters that can be
 * set via the WiFi configuration portal.
 */
struct APRSConfig {
    char callsign[10];          // Max 6 chars + SSID separator + null
    uint8_t ssid;               // 0-15 (see https://www.aprs.org/aprs11/SSIDs.txt)
    char symbol;                // APRS symbol character
    char symbol_table;          // '/' (primary) or '\\' (alternate)
    char path1[10];             // First path (e.g., "WIDE1")
    uint8_t path1_ssid;         // Path1 SSID (1-7)
    char path2[10];             // Second path (e.g., "WIDE2")
    uint8_t path2_ssid;         // Path2 SSID (1-7)
    float frequency;            // Radio frequency in MHz (e.g., 144.9900)
    uint16_t preamble_ms;       // PTT lead time in milliseconds
    uint16_t tail_ms;           // PTT tail time in milliseconds
    uint16_t update_interval_min; // TX interval in minutes
};

/**
 * Load APRS configuration from persistent storage
 * 
 * @return APRSConfig structure with loaded values (or defaults if not configured)
 */
APRSConfig loadAPRSConfig();

/**
 * Save APRS configuration to persistent storage
 * 
 * @param config The configuration to save
 */
void saveAPRSConfig(const APRSConfig& config);

/**
 * Check if APRS has been configured
 * 
 * @return true if configuration exists, false otherwise
 */
bool isAPRSConfigured();

/**
 * Get default APRS configuration
 * 
 * @return APRSConfig structure with default values
 */
APRSConfig getDefaultAPRSConfig();

#endif // APRS_CONFIG_H
