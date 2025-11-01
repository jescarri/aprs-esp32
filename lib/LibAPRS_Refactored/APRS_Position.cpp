#include "APRS_Position.h"
#include <math.h>
#include <stdio.h>

namespace APRS {

bool convertLatitude(float lat, char* buffer) {
    if (!isValidLatitude(lat)) {
        return false;
    }
    
    // Determine hemisphere
    char hemisphere = (lat >= 0) ? 'N' : 'S';
    lat = fabs(lat);
    
    // Extract degrees
    int degrees = (int)lat;
    
    // Extract minutes (fractional part * 60)
    float minutes = (lat - degrees) * 60.0f;
    
    // Format: DDMM.MMH where H is N or S
    // Leading zeros are important for APRS
    snprintf(buffer, 9, "%02d%05.2f%c", degrees, minutes, hemisphere);
    
    // Ensure null termination
    buffer[8] = '\0';
    
    return true;
}

bool convertLongitude(float lon, char* buffer) {
    if (!isValidLongitude(lon)) {
        return false;
    }
    
    // Determine hemisphere
    char hemisphere = (lon >= 0) ? 'E' : 'W';
    lon = fabs(lon);
    
    // Extract degrees
    int degrees = (int)lon;
    
    // Extract minutes (fractional part * 60)
    float minutes = (lon - degrees) * 60.0f;
    
    // Format: DDDMM.MMH where H is E or W
    // Leading zeros are important for APRS
    snprintf(buffer, 10, "%03d%05.2f%c", degrees, minutes, hemisphere);
    
    // Ensure null termination
    buffer[9] = '\0';
    
    return true;
}

} // namespace APRS
