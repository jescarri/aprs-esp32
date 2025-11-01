#ifndef APRS_POSITION_H
#define APRS_POSITION_H

#include <stdint.h>

/**
 * APRS Position Utilities
 * 
 * Provides functions to convert decimal degrees (float) coordinates
 * to APRS position format strings.
 * 
 * APRS Format:
 * - Latitude:  DDMM.MMN (8 chars) e.g., "4906.14N" = 49.1023° North
 * - Longitude: DDDMM.MML (9 chars) e.g., "12238.19W" = -122.6365° West
 */

namespace APRS {

/**
 * Convert decimal degrees latitude to APRS format
 * 
 * @param lat Latitude in decimal degrees (-90 to +90)
 *            Positive = North, Negative = South
 * @param buffer Output buffer (must be at least 9 bytes)
 * @return true on success, false if lat is out of range
 * 
 * Example: 49.1023 -> "4906.14N"
 */
bool convertLatitude(float lat, char* buffer);

/**
 * Convert decimal degrees longitude to APRS format
 * 
 * @param lon Longitude in decimal degrees (-180 to +180)
 *            Positive = East, Negative = West
 * @param buffer Output buffer (must be at least 10 bytes)
 * @return true on success, false if lon is out of range
 * 
 * Example: -122.6365 -> "12238.19W"
 */
bool convertLongitude(float lon, char* buffer);

/**
 * Validate if latitude is in valid range
 */
inline bool isValidLatitude(float lat) {
    return lat >= -90.0f && lat <= 90.0f;
}

/**
 * Validate if longitude is in valid range
 */
inline bool isValidLongitude(float lon) {
    return lon >= -180.0f && lon <= 180.0f;
}

} // namespace APRS

#endif // APRS_POSITION_H
