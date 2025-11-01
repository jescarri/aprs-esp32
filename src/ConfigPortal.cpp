#include "ConfigPortal.h"
#include "APRSConfig.h"
#include "Settings.h"
#include <WiFiManager.h>

// Storage for form field values
static char callsignBuf[10];
static char ssidBuf[8];
static char symbolBuf[4];
static char symbolTableBuf[4];
static char path1Buf[10];
static char path1SsidBuf[8];
static char path2Buf[10];
static char path2SsidBuf[8];
static char frequencyBuf[16];
static char preambleBuf[8];
static char tailBuf[8];
static char updateIntervalBuf[8];

/**
 * Save callback - called by WiFiManager when user submits form
 */
static void saveConfigCallback()
{
    Serial.println("\n[ConfigPortal] Saving configuration...");
    
    // Parse and save APRS configuration
    APRSConfig config;
    
    // Callsign and SSID
    strncpy(config.callsign, callsignBuf, sizeof(config.callsign) - 1);
    config.callsign[sizeof(config.callsign) - 1] = '\0';
    config.ssid = atoi(ssidBuf);
    if (config.ssid > 15) config.ssid = 15;
    
    // Symbol
    if (strlen(symbolBuf) > 0) {
        config.symbol = symbolBuf[0];
    } else {
        config.symbol = 'n';  // Default: car
    }
    
    // Symbol table
    if (strlen(symbolTableBuf) > 0) {
        config.symbol_table = symbolTableBuf[0];
    } else {
        config.symbol_table = '/';  // Default: primary table
    }
    
    // Paths
    strncpy(config.path1, path1Buf, sizeof(config.path1) - 1);
    config.path1[sizeof(config.path1) - 1] = '\0';
    config.path1_ssid = atoi(path1SsidBuf);
    if (config.path1_ssid > 7) config.path1_ssid = 7;
    if (config.path1_ssid < 1) config.path1_ssid = 1;
    
    strncpy(config.path2, path2Buf, sizeof(config.path2) - 1);
    config.path2[sizeof(config.path2) - 1] = '\0';
    config.path2_ssid = atoi(path2SsidBuf);
    if (config.path2_ssid > 7) config.path2_ssid = 7;
    if (config.path2_ssid < 1) config.path2_ssid = 1;
    
    // Frequency
    config.frequency = atof(frequencyBuf);
    if (config.frequency < 144.0 || config.frequency > 148.0) {
        config.frequency = 144.990;  // Default
    }
    
    // Timing
    config.preamble_ms = atoi(preambleBuf);
    if (config.preamble_ms < 100) config.preamble_ms = 100;
    if (config.preamble_ms > 1000) config.preamble_ms = 1000;
    
    config.tail_ms = atoi(tailBuf);
    if (config.tail_ms < 10) config.tail_ms = 10;
    if (config.tail_ms > 500) config.tail_ms = 500;
    
    config.update_interval_min = atoi(updateIntervalBuf);
    if (config.update_interval_min < 1) config.update_interval_min = 1;
    if (config.update_interval_min > 60) config.update_interval_min = 60;
    
    // Save to persistent storage
    saveAPRSConfig(config);
    
    Serial.println("[ConfigPortal] Configuration saved!");
    Serial.printf("  Callsign: %s-%d\n", config.callsign, config.ssid);
    Serial.printf("  Symbol: %c (table %c)\n", config.symbol, config.symbol_table);
    Serial.printf("  Path: %s-%d,%s-%d\n", config.path1, config.path1_ssid, config.path2, config.path2_ssid);
    Serial.printf("  Frequency: %.4f MHz\n", config.frequency);
    Serial.printf("  Timing: preamble=%dms tail=%dms\n", config.preamble_ms, config.tail_ms);
    Serial.printf("  Update interval: %d minutes\n", config.update_interval_min);
}

/**
 * Start the WiFi configuration portal
 */
bool startConfigPortal(int timeoutSeconds)
{
    Serial.println("\n========================================");
    Serial.println("  APRS CONFIGURATION PORTAL");
    Serial.println("========================================");
    
    // Load current configuration (or defaults)
    APRSConfig config = loadAPRSConfig();
    
    // Populate form fields with current values
    strncpy(callsignBuf, config.callsign, sizeof(callsignBuf) - 1);
    snprintf(ssidBuf, sizeof(ssidBuf), "%d", config.ssid);
    snprintf(symbolBuf, sizeof(symbolBuf), "%c", config.symbol);
    snprintf(symbolTableBuf, sizeof(symbolTableBuf), "%c", config.symbol_table);
    strncpy(path1Buf, config.path1, sizeof(path1Buf) - 1);
    snprintf(path1SsidBuf, sizeof(path1SsidBuf), "%d", config.path1_ssid);
    strncpy(path2Buf, config.path2, sizeof(path2Buf) - 1);
    snprintf(path2SsidBuf, sizeof(path2SsidBuf), "%d", config.path2_ssid);
    snprintf(frequencyBuf, sizeof(frequencyBuf), "%.4f", config.frequency);
    snprintf(preambleBuf, sizeof(preambleBuf), "%d", config.preamble_ms);
    snprintf(tailBuf, sizeof(tailBuf), "%d", config.tail_ms);
    snprintf(updateIntervalBuf, sizeof(updateIntervalBuf), "%d", config.update_interval_min);
    
    // Create WiFiManager instance
    WiFiManager wm;
    wm.setConfigPortalBlocking(true);
    wm.setSaveConfigCallback(saveConfigCallback);
    
    // Add custom parameters with helpful placeholders and patterns
    WiFiManagerParameter customHeading("<h2>APRS Configuration</h2>");
    wm.addParameter(&customHeading);
    
    WiFiManagerParameter paramCallsign("callsign", "Callsign (max 6 chars)", callsignBuf, 10, 
                                       "maxlength='6' pattern='[A-Z0-9]{1,6}' style='text-transform:uppercase'");
    WiFiManagerParameter paramSsid("ssid", "SSID (0-15, see aprs.org/aprs11/SSIDs.txt)", ssidBuf, 8,
                                   "type='number' min='0' max='15'");
    WiFiManagerParameter paramSymbol("symbol", "Symbol (one char, see aprs.org/symbols.html)", symbolBuf, 4,
                                     "maxlength='1'");
    WiFiManagerParameter paramSymbolTable("symbol_table", "Symbol Table (/ or \\)", symbolTableBuf, 4,
                                         "maxlength='1' pattern='[/\\\\]'");
    
    WiFiManagerParameter pathHeading("<h3>Digipeater Path</h3>");
    wm.addParameter(&pathHeading);
    
    WiFiManagerParameter paramPath1("path1", "Path 1 (e.g., WIDE1)", path1Buf, 10);
    WiFiManagerParameter paramPath1Ssid("path1_ssid", "Path 1 SSID (1-7)", path1SsidBuf, 8,
                                       "type='number' min='1' max='7'");
    WiFiManagerParameter paramPath2("path2", "Path 2 (e.g., WIDE2)", path2Buf, 10);
    WiFiManagerParameter paramPath2Ssid("path2_ssid", "Path 2 SSID (1-7)", path2SsidBuf, 8,
                                       "type='number' min='1' max='7'");
    
    WiFiManagerParameter radioHeading("<h3>Radio Settings</h3>");
    wm.addParameter(&radioHeading);
    
    WiFiManagerParameter paramFrequency("frequency", "Frequency (MHz, e.g., 144.9900)", frequencyBuf, 16,
                                       "type='number' step='0.0001' min='144' max='148'");
    WiFiManagerParameter paramPreamble("preamble", "PTT Preamble (ms, 100-1000)", preambleBuf, 8,
                                      "type='number' min='100' max='1000'");
    WiFiManagerParameter paramTail("tail", "PTT Tail (ms, 10-500)", tailBuf, 8,
                                   "type='number' min='10' max='500'");
    WiFiManagerParameter paramUpdateInterval("update_interval", "Update Interval (minutes, 1-60)", updateIntervalBuf, 8,
                                             "type='number' min='1' max='60'");
    
    // Add all parameters
    wm.addParameter(&paramCallsign);
    wm.addParameter(&paramSsid);
    wm.addParameter(&paramSymbol);
    wm.addParameter(&paramSymbolTable);
    wm.addParameter(&paramPath1);
    wm.addParameter(&paramPath1Ssid);
    wm.addParameter(&paramPath2);
    wm.addParameter(&paramPath2Ssid);
    wm.addParameter(&paramFrequency);
    wm.addParameter(&paramPreamble);
    wm.addParameter(&paramTail);
    wm.addParameter(&paramUpdateInterval);
    
    // Generate portal SSID
    String portalSSID;
    if (strlen(config.callsign) > 0 && strcmp(config.callsign, "NOCALL") != 0) {
        portalSSID = "APRS-" + String(config.callsign);
    } else {
        portalSSID = "APRS-NODE";
    }
    
    Serial.printf("\n[ConfigPortal] Starting WiFi portal...\n");
    Serial.printf("  AP SSID: %s\n", portalSSID.c_str());
    Serial.printf("  AP IP: 192.168.4.1\n");
    Serial.printf("  URL: http://192.168.4.1\n");
    
    if (timeoutSeconds > 0) {
        Serial.printf("  Timeout: %d seconds\n", timeoutSeconds);
        wm.setConfigPortalTimeout(timeoutSeconds);
    } else {
        Serial.println("  Timeout: None (will wait indefinitely)");
    }
    
    Serial.println("\nConnect to the AP and open the URL to configure.");
    Serial.println("========================================\n");
    
    // Start portal (blocking)
    bool result = wm.startConfigPortal(portalSSID.c_str());
    
    if (result) {
        Serial.println("\n[ConfigPortal] Configuration completed successfully!");
        Serial.println("Restarting...\n");
        delay(1000);
        ESP.restart();
    } else {
        Serial.println("\n[ConfigPortal] Portal timeout or user cancelled.");
    }
    
    return result;
}
