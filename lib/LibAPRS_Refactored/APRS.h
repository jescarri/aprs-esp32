#ifndef APRS_H
#define APRS_H

#include "APRS_Protocol.h"
#include "APRS_Position.h"
#include "APRS_Telemetry.h"

namespace APRS {

/**
 * Main APRS Configuration
 */
struct Config {
    const char* callsign = "NOCALL";
    uint8_t ssid = 0;
    const char* path1 = "WIDE1";
    uint8_t path1_ssid = 1;
    const char* path2 = "WIDE2";
    uint8_t path2_ssid = 2;
    char symbol = 'n';               // APRS symbol (default: car)
    char symbol_table = '/';          // Symbol table (/ or \)
    uint16_t preamble_ms = 350;
    uint16_t tail_ms = 50;
    uint8_t ptt_pin = 33;  // GPIO pin number
};

/**
 * Main APRS Class
 * 
 * Provides high-level API for APRS operations:
 * - Send position reports from float coordinates
 * - Send telemetry with structured data
 * - Send custom messages
 * 
 * Example usage:
 * 
 *   APRS::APRSClient aprs;
 *   APRS::Config config;
 *   config.callsign = "VA7RCV";
 *   config.ssid = 15;
 *   
 *   if (aprs.begin(config)) {
 *       aprs.sendPosition(49.1023, -122.6365, "ESP32 Tracker");
 *   }
 */
class APRSClient {
public:
    APRSClient() : _telemetry_seq(0) {}
    
    /**
     * Initialize APRS
     * @param config APRS configuration
     * @return true on success
     */
    bool begin(const Config& config);
    
    /**
     * Send position report with automatic coordinate conversion
     * 
     * @param lat Latitude in decimal degrees
     * @param lon Longitude in decimal degrees
     * @param comment Optional comment (max 43 characters)
     * @param power PHG power code (0-9, default 1)
     * @param height PHG height code (0-9, default 1)
     * @param gain PHG gain code (0-9, default 1)
     * @param directivity PHG directivity code (0-8, default 0)
     * @return true on success
     */
    bool sendPosition(float lat, float lon, 
                     const char* comment = nullptr,
                     uint8_t power = 1,
                     uint8_t height = 1,
                     uint8_t gain = 1,
                     uint8_t directivity = 0);
    
    /**
     * Send telemetry data
     * 
     * @param data Telemetry data structure (5 analog channels)
     * @param auto_increment If true, automatically increment sequence number
     * @return true on success
     */
    bool sendTelemetry(const TelemetryData& data, bool auto_increment = true);
    
    /**
     * Send telemetry definition packets (PARM and UNIT)
     * Should be sent periodically or at startup
     * 
     * @return true on success
     */
    bool sendTelemetryDefinitions();
    
    /**
     * Send raw APRS message packet
     * 
     * @param message Message content
     * @return true on success
     */
    bool sendMessage(const char* message);
    
    /**
     * Send raw packet with custom payload
     * 
     * @param payload Raw packet data
     * @param length Payload length
     * @return true on success
     */
    bool sendRawPacket(const uint8_t* payload, size_t length);
    
    /**
     * Check if currently transmitting
     */
    bool isBusy() const { return _protocol.isBusy(); }
    
    /**
     * Get current telemetry sequence number
     */
    uint16_t getTelemetrySequence() const { return _telemetry_seq; }
    
    /**
     * Set telemetry sequence number (for manual control)
     */
    void setTelemetrySequence(uint16_t seq) { _telemetry_seq = seq % 1000; }
    
    /**
     * Manual PTT control (for testing or custom applications)
     */
    void setPTT(bool enable) { _protocol.setPTT(enable); }
    
private:
    Config _config;
    Protocol _protocol;
    uint16_t _telemetry_seq;
    
    // Build path array from config
    void buildPath(AX25Call* path, size_t& path_len);
    
    // Create AX25Call from string
    AX25Call makeCall(const char* callsign, uint8_t ssid);
};

} // namespace APRS

#endif // APRS_H
