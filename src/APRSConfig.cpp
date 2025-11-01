#include "APRSConfig.h"
#include "Settings.h"
#include "hardware_config.h"
#include <cstring>

/**
 * Get default APRS configuration
 */
APRSConfig getDefaultAPRSConfig()
{
    APRSConfig config;
    
    strncpy(config.callsign, DEFAULT_APRS_CALLSIGN, sizeof(config.callsign) - 1);
    config.callsign[sizeof(config.callsign) - 1] = '\0';
    
    config.ssid = APRS_SSID;
    config.symbol = DEFAULT_APRS_SYMBOL;
    config.symbol_table = DEFAULT_APRS_TABLE;
    
    strncpy(config.path1, "WIDE1", sizeof(config.path1) - 1);
    config.path1[sizeof(config.path1) - 1] = '\0';
    config.path1_ssid = 1;
    
    strncpy(config.path2, "WIDE2", sizeof(config.path2) - 1);
    config.path2[sizeof(config.path2) - 1] = '\0';
    config.path2_ssid = 2;
    
    config.frequency = RADIO_FREC;
    config.preamble_ms = DEFAULT_PREAMBLE_MS;
    config.tail_ms = DEFAULT_TAIL_MS;
    config.update_interval_min = APRS_TX_CYCLE_SECONDS / 60;  // Convert seconds to minutes
    
    return config;
}

/**
 * Check if APRS has been configured
 */
bool isAPRSConfigured()
{
    return settings_has_key("config_done") && settings_get_bool("config_done", false);
}

/**
 * Load APRS configuration from persistent storage
 */
APRSConfig loadAPRSConfig()
{
    APRSConfig config = getDefaultAPRSConfig();
    
    // Only load if configuration has been saved
    if (!isAPRSConfigured()) {
        return config;
    }
    
    // Load string values
    String callsign_str = settings_get_string("callsign", DEFAULT_APRS_CALLSIGN);
    strncpy(config.callsign, callsign_str.c_str(), sizeof(config.callsign) - 1);
    config.callsign[sizeof(config.callsign) - 1] = '\0';
    
    String path1_str = settings_get_string("path1", "WIDE1");
    strncpy(config.path1, path1_str.c_str(), sizeof(config.path1) - 1);
    config.path1[sizeof(config.path1) - 1] = '\0';
    
    String path2_str = settings_get_string("path2", "WIDE2");
    strncpy(config.path2, path2_str.c_str(), sizeof(config.path2) - 1);
    config.path2[sizeof(config.path2) - 1] = '\0';
    
    String symbol_str = settings_get_string("symbol", "n");
    if (symbol_str.length() > 0) {
        config.symbol = symbol_str[0];
    }
    
    String table_str = settings_get_string("symbol_table", "/");
    if (table_str.length() > 0) {
        config.symbol_table = table_str[0];
    }
    
    // Load numeric values
    config.ssid = settings_get_int("ssid", APRS_SSID);
    config.path1_ssid = settings_get_int("path1_ssid", 1);
    config.path2_ssid = settings_get_int("path2_ssid", 2);
    config.frequency = settings_get_float("frequency", RADIO_FREC);
    config.preamble_ms = settings_get_int("preamble_ms", DEFAULT_PREAMBLE_MS);
    config.tail_ms = settings_get_int("tail_ms", DEFAULT_TAIL_MS);
    config.update_interval_min = settings_get_int("update_interval_min", APRS_TX_CYCLE_SECONDS / 60);
    
    return config;
}

/**
 * Save APRS configuration to persistent storage
 */
void saveAPRSConfig(const APRSConfig& config)
{
    settings_put_string("callsign", config.callsign);
    settings_put_int("ssid", config.ssid);
    
    char symbol_buf[2] = {config.symbol, '\0'};
    settings_put_string("symbol", symbol_buf);
    
    char table_buf[2] = {config.symbol_table, '\0'};
    settings_put_string("symbol_table", table_buf);
    
    settings_put_string("path1", config.path1);
    settings_put_int("path1_ssid", config.path1_ssid);
    settings_put_string("path2", config.path2);
    settings_put_int("path2_ssid", config.path2_ssid);
    
    settings_put_float("frequency", config.frequency);
    settings_put_int("preamble_ms", config.preamble_ms);
    settings_put_int("tail_ms", config.tail_ms);
    settings_put_int("update_interval_min", config.update_interval_min);
    
    // Mark configuration as complete
    settings_put_bool("config_done", true);
}
