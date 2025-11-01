#include "APRS_Telemetry.h"
#include <stdio.h>
#include <string.h>

namespace APRS {

size_t TelemetryBuilder::buildDataPacket(uint16_t sequence, 
                                          const TelemetryData& data,
                                          char* buffer) {
    // Wrap sequence at 999
    sequence = sequence % 1000;
    
    // Format digital bits as 8 binary digits
    char digital_str[9];
    for (int i = 7; i >= 0; i--) {
        digital_str[7 - i] = (data.digital & (1 << i)) ? '1' : '0';
    }
    digital_str[8] = '\0';
    
    // Build packet: T#SSS,A1,A2,A3,A4,A5,DDDDDDDD
    return snprintf(buffer, 64, "T#%03d,%.3f,%.3f,%.3f,%.3f,%.3f,%s",
                   sequence, 
                   data.analog[0], data.analog[1], data.analog[2], 
                   data.analog[3], data.analog[4], 
                   digital_str);
}

size_t TelemetryBuilder::buildParmPacket(const char* callsign, uint8_t ssid, 
                                          const char* names[5], char* buffer) {
    // Format: :CALLSIGN-SS:PARM.name1,name2,name3,name4,name5{msgID
    // Callsign field is 9 chars (padded with spaces)
    char call_field[10];
    int len = snprintf(call_field, sizeof(call_field), "%s-%d", callsign, ssid);
    // Pad with spaces to 9 characters
    while (len < 9) {
        call_field[len++] = ' ';
    }
    call_field[9] = '\0';
    
    // Use static counter for message ID (wraps at 999)
    static uint16_t msg_id = 1;
    uint16_t current_id = msg_id;
    msg_id = (msg_id % 999) + 1;
    
    return snprintf(buffer, 128, ":%s:PARM.%s,%s,%s,%s,%s{%d",
                   call_field, names[0], names[1], names[2], names[3], names[4], current_id);
}

size_t TelemetryBuilder::buildUnitPacket(const char* callsign, uint8_t ssid,
                                          const char* units[5], char* buffer) {
    // Format: :CALLSIGN-SS:UNIT.unit1,unit2,unit3,unit4,unit5{msgID
    char call_field[10];
    int len = snprintf(call_field, sizeof(call_field), "%s-%d", callsign, ssid);
    while (len < 9) {
        call_field[len++] = ' ';
    }
    call_field[9] = '\0';
    
    // Use static counter for message ID (wraps at 999)
    static uint16_t msg_id = 1;
    uint16_t current_id = msg_id;
    msg_id = (msg_id % 999) + 1;
    
    return snprintf(buffer, 128, ":%s:UNIT.%s,%s,%s,%s,%s{%d",
                   call_field, units[0], units[1], units[2], units[3], units[4], current_id);
}

size_t TelemetryBuilder::buildStandardParmPacket(const char* callsign, uint8_t ssid, char* buffer) {
    const char* names[5] = {
        "Battery",
        "Temp",
        "Pressure",
        "Humidity",
        "Altitude"
    };
    return buildParmPacket(callsign, ssid, names, buffer);
}

size_t TelemetryBuilder::buildStandardUnitPacket(const char* callsign, uint8_t ssid, char* buffer) {
    const char* units[5] = {
        "volts",
        "deg.C",
        "mbar",
        "%",
        "meters"
    };
    return buildUnitPacket(callsign, ssid, units, buffer);
}

int TelemetryBuilder::floatToTelemetryValue(float value, float min_val, float max_val) {
    // Clamp value to range
    if (value < min_val) value = min_val;
    if (value > max_val) value = max_val;
    
    // Scale to 0-999 range
    float scaled = ((value - min_val) / (max_val - min_val)) * 999.0f;
    
    return (int)(scaled + 0.5f);  // Round to nearest integer
}

} // namespace APRS
