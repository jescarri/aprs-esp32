#ifndef APRS_TELEMETRY_H
#define APRS_TELEMETRY_H

#include <stdint.h>
#include <stddef.h>

namespace APRS {

/**
 * APRS Telemetry Data Structure
 * 
 * Holds up to 5 analog channels and 8 digital channels
 * Standard tracker usage:
 * - A1: Battery voltage
 * - A2: Temperature
 * - A3: Pressure
 * - A4: Humidity
 * - A5: Altitude
 */
struct TelemetryData {
    float analog[5];      // 5 analog channels
    uint8_t digital;      // 8 digital bits (bit 0-7)
};

/**
 * APRS Telemetry Packet Builder
 * 
 * Creates APRS telemetry packets in standard format:
 * T#SSS,A1,A2,A3,A4,A5,DDDDDDDD
 * 
 * Also handles telemetry definition messages (PARM, UNIT, EQNS, BITS)
 */
class TelemetryBuilder {
public:
    /**
     * Build telemetry data packet
     * 
     * @param sequence Sequence number (000-999, wraps around)
     * @param data Telemetry data structure
     * @param buffer Output buffer (must be at least 64 bytes)
     * @return Length of packet, or 0 on error
     * 
     * Format: T#003,123,456,789,012,345,00000000
     */
    static size_t buildDataPacket(uint16_t sequence, 
                                   const TelemetryData& data,
                                   char* buffer);
    
    /**
     * Build PARM (parameter names) definition packet
     * 
     * @param names Array of 5 parameter names
     * @param buffer Output buffer (must be at least 128 bytes)
     * @return Length of packet
     * 
     * Format: PARM.Battery,Temp,Pressure,Humidity,Altitude
     */
    static size_t buildParmPacket(const char* names[5], char* buffer);
    
    /**
     * Build UNIT (units) definition packet
     * 
     * @param units Array of 5 unit names
     * @param buffer Output buffer (must be at least 128 bytes)
     * @return Length of packet
     * 
     * Format: UNIT.volts,deg.C,mbar,%,meters
     */
    static size_t buildUnitPacket(const char* units[5], char* buffer);
    
    /**
     * Build standard tracker telemetry PARM packet
     * Defines: Battery, Temp, Pressure, Humidity, Altitude
     */
    static size_t buildStandardParmPacket(char* buffer);
    
    /**
     * Build standard tracker telemetry UNIT packet
     * Defines: volts, deg.C, mbar, %, meters
     */
    static size_t buildStandardUnitPacket(char* buffer);
    
    /**
     * Convert float to APRS telemetry format (3 decimal digits)
     * Values are scaled to 000-999 range
     */
    static int floatToTelemetryValue(float value, float min_val, float max_val);
};

} // namespace APRS

#endif // APRS_TELEMETRY_H
