#include "APRS.h"
#include <string.h>

namespace APRS {

bool APRSClient::begin(const Config& config) {
    _config = config;
    
    ProtocolConfig pconfig = {
        .ptt_pin = _config.ptt_pin,
        .preamble_ms = _config.preamble_ms,
        .tail_ms = _config.tail_ms
    };
    
    return _protocol.begin(pconfig);
}

AX25Call APRSClient::makeCall(const char* callsign, uint8_t ssid) {
    AX25Call call = {0};
    strncpy(call.call, callsign, 6);
    call.call[6] = '\0';
    call.ssid = ssid;
    return call;
}

void APRSClient::buildPath(AX25Call* path, size_t& path_len) {
    path_len = 0;
    if (_config.path1 && _config.path1[0]) {
        path[path_len++] = makeCall(_config.path1, _config.path1_ssid);
    }
    if (_config.path2 && _config.path2[0]) {
        path[path_len++] = makeCall(_config.path2, _config.path2_ssid);
    }
}

bool APRSClient::sendPosition(float lat, float lon, 
                             const char* comment,
                             uint8_t power,
                             uint8_t height,
                             uint8_t gain,
                             uint8_t directivity) {
    // Convert coordinates to APRS format
    char lat_str[9];
    char lon_str[10];
    if (!convertLatitude(lat, lat_str) || !convertLongitude(lon, lon_str)) {
        return false;
    }
    
    // Build payload:
    // =DDMM.MMN/DDDMM.MMLsPHGphgd<comment>
    // where s is symbol table, L is symbol, PHG is optional
    char payload[120];
    size_t idx = 0;
    
    payload[idx++] = '=';  // Position without timestamp
    
    // Latitude
    memcpy(&payload[idx], lat_str, 8); idx += 8;
    
    // Symbol table
    payload[idx++] = _config.symbol_table;
    
    // Longitude
    memcpy(&payload[idx], lon_str, 9); idx += 9;
    
    // Symbol
    payload[idx++] = _config.symbol;
    
    // Optional PHG (if all values < 10)
    if (power < 10 && height < 10 && gain < 10 && directivity <= 9) {
        payload[idx++] = 'P';
        payload[idx++] = 'H';
        payload[idx++] = 'G';
        payload[idx++] = char('0' + power);
        payload[idx++] = char('0' + height);
        payload[idx++] = char('0' + gain);
        payload[idx++] = char('0' + directivity);
    }
    
    // Optional comment
    if (comment && comment[0]) {
        size_t comment_len = strnlen(comment, 60);
        memcpy(&payload[idx], comment, comment_len);
        idx += comment_len;
    }
    
    // Build path and send
    AX25Call src = makeCall(_config.callsign, _config.ssid);
    AX25Call dst = makeCall("APZMDR", 0);  // Open Source MDroid TOCALL
    AX25Call path[2]; size_t path_len;
    buildPath(path, path_len);
    
    return _protocol.sendPacket(src, dst, path, path_len,
                                reinterpret_cast<uint8_t*>(payload), idx);
}

bool APRSClient::sendTelemetry(const TelemetryData& data, bool auto_increment) {
    char buffer[128];
    size_t len = TelemetryBuilder::buildDataPacket(_telemetry_seq, data, buffer);
    
    if (auto_increment) {
        _telemetry_seq = (_telemetry_seq + 1) % 1000;
    }
    
    AX25Call src = makeCall(_config.callsign, _config.ssid);
    AX25Call dst = makeCall("APZMDR", 0);  // Open Source MDroid TOCALL
    AX25Call path[2]; size_t path_len;
    buildPath(path, path_len);
    
    return _protocol.sendPacket(src, dst, path, path_len,
                                reinterpret_cast<uint8_t*>(buffer), len);
}

bool APRSClient::sendTelemetryDefinitions() {
    char parm[128];
    char unit[128];
    size_t len_parm = TelemetryBuilder::buildStandardParmPacket(_config.callsign, _config.ssid, parm);
    size_t len_unit = TelemetryBuilder::buildStandardUnitPacket(_config.callsign, _config.ssid, unit);
    
    AX25Call src = makeCall(_config.callsign, _config.ssid);
    AX25Call dst = makeCall("APZMDR", 0);  // Open Source MDroid TOCALL
    AX25Call path[2]; size_t path_len;
    buildPath(path, path_len);
    
    bool ok1 = _protocol.sendPacket(src, dst, path, path_len,
                                    reinterpret_cast<uint8_t*>(parm), len_parm);
    bool ok2 = _protocol.sendPacket(src, dst, path, path_len,
                                    reinterpret_cast<uint8_t*>(unit), len_unit);
    return ok1 && ok2;
}

bool APRSClient::sendMessage(const char* message) {
    if (!message || !message[0]) return false;
    
    // APRS message format: :CALLSIGN-SSID:message
    // For simplicity here we send a plain text payload
    const uint8_t* payload = reinterpret_cast<const uint8_t*>(message);
    size_t length = strnlen(message, 255);
    
    AX25Call src = makeCall(_config.callsign, _config.ssid);
    AX25Call dst = makeCall("APZMDR", 0);  // Open Source MDroid TOCALL
    AX25Call path[2]; size_t path_len;
    buildPath(path, path_len);
    
    return _protocol.sendPacket(src, dst, path, path_len, payload, length);
}

bool APRSClient::sendRawPacket(const uint8_t* payload, size_t length) {
    if (!payload || length == 0) return false;
    
    AX25Call src = makeCall(_config.callsign, _config.ssid);
    AX25Call dst = makeCall("APZMDR", 0);  // Open Source MDroid TOCALL
    AX25Call path[2]; size_t path_len;
    buildPath(path, path_len);
    
    return _protocol.sendPacket(src, dst, path, path_len, payload, length);
}

} // namespace APRS
