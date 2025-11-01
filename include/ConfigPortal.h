#ifndef CONFIG_PORTAL_H
#define CONFIG_PORTAL_H

#include <Arduino.h>

/**
 * ConfigPortal - WiFi configuration portal for APRS settings
 * 
 * Provides a web interface for configuring:
 * - WiFi credentials (via WiFiManager's built-in scan)
 * - APRS callsign and SSID
 * - APRS paths and digipeaters
 * - Radio frequency
 * - TX timing parameters
 * 
 * Based on the pattern from e-paper project.
 */

/**
 * Start the WiFi configuration portal
 * 
 * Creates an access point with SSID based on callsign (APRS-<CALLSIGN>)
 * or "APRS-NODE" if not yet configured.
 * 
 * The portal will run until either:
 * - User submits configuration
 * - Timeout is reached (if timeoutSeconds > 0)
 * 
 * @param timeoutSeconds Portal timeout in seconds (0 = no timeout)
 * @return true if configuration was saved, false on timeout or error
 */
bool startConfigPortal(int timeoutSeconds = 0);

#endif // CONFIG_PORTAL_H
