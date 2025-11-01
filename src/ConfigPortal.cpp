#include "ConfigPortal.h"
#include "APRSConfig.h"
#include "Settings.h"
#include <WiFiManager.h>

// WiFiManagerParameter pointers - must persist until callback completes
static WiFiManagerParameter* paramCallsign = nullptr;
static WiFiManagerParameter* paramSsid = nullptr;
static WiFiManagerParameter* paramSymbol = nullptr;
static WiFiManagerParameter* paramSymbolTable = nullptr;
static WiFiManagerParameter* paramPath1 = nullptr;
static WiFiManagerParameter* paramPath1Ssid = nullptr;
static WiFiManagerParameter* paramPath2 = nullptr;
static WiFiManagerParameter* paramPath2Ssid = nullptr;
static WiFiManagerParameter* paramFrequency = nullptr;
static WiFiManagerParameter* paramPreamble = nullptr;
static WiFiManagerParameter* paramTail = nullptr;
static WiFiManagerParameter* paramUpdateInterval = nullptr;

// Buffer storage for form field initial values
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
    
    // Get values from WiFiManager parameters
    const char* callsign_input = paramCallsign->getValue();
    const char* ssid_input = paramSsid->getValue();
    
    // Debug: Show form data from getValue()
    Serial.printf("[DEBUG] Form data from getValue():\n");
    Serial.printf("  callsign: '%s'\n", callsign_input);
    Serial.printf("  ssid: '%s'\n", ssid_input);
    
    const char* sym = paramSymbol->getValue();
    Serial.printf("  symbol: len=%d", strlen(sym));
    if (strlen(sym) > 0) {
        Serial.printf(" hex=0x%02X\n", (uint8_t)sym[0]);
    } else {
        Serial.println(" (empty)");
    }
    
    const char* tbl = paramSymbolTable->getValue();
    Serial.printf("  symbol_table: len=%d", strlen(tbl));
    if (strlen(tbl) > 0) {
        Serial.printf(" hex=0x%02X\n", (uint8_t)tbl[0]);
    } else {
        Serial.println(" (empty)");
    }
    
    // Parse and save APRS configuration
    APRSConfig config;
    
    // Callsign - convert to uppercase and validate
    strncpy(config.callsign, callsign_input, sizeof(config.callsign) - 1);
    config.callsign[sizeof(config.callsign) - 1] = '\0';
    
    // Convert to uppercase
    for (int i = 0; config.callsign[i]; i++) {
        config.callsign[i] = toupper(config.callsign[i]);
    }
    
    // If empty, set to NOCALL
    if (strlen(config.callsign) == 0) {
        strcpy(config.callsign, "NOCALL");
    }
    
    config.ssid = atoi(ssid_input);
    if (config.ssid > 15) config.ssid = 15;
    
    // Symbol
    const char* symbol_input = paramSymbol->getValue();
    if (strlen(symbol_input) > 0) {
        config.symbol = symbol_input[0];
    } else {
        config.symbol = 'n';  // Default: car
    }
    
    // Symbol table
    const char* table_input = paramSymbolTable->getValue();
    if (strlen(table_input) > 0) {
        config.symbol_table = table_input[0];
    } else {
        config.symbol_table = '/';  // Default: primary table
    }
    
    // Paths
    strncpy(config.path1, paramPath1->getValue(), sizeof(config.path1) - 1);
    config.path1[sizeof(config.path1) - 1] = '\0';
    config.path1_ssid = atoi(paramPath1Ssid->getValue());
    if (config.path1_ssid > 7) config.path1_ssid = 7;
    if (config.path1_ssid < 1) config.path1_ssid = 1;
    
    strncpy(config.path2, paramPath2->getValue(), sizeof(config.path2) - 1);
    config.path2[sizeof(config.path2) - 1] = '\0';
    config.path2_ssid = atoi(paramPath2Ssid->getValue());
    if (config.path2_ssid > 7) config.path2_ssid = 7;
    if (config.path2_ssid < 1) config.path2_ssid = 1;
    
    // Frequency
    config.frequency = atof(paramFrequency->getValue());
    if (config.frequency < 144.0 || config.frequency > 148.0) {
        config.frequency = 144.990;  // Default
    }
    
    // Timing
    config.preamble_ms = atoi(paramPreamble->getValue());
    if (config.preamble_ms < 100) config.preamble_ms = 100;
    if (config.preamble_ms > 1000) config.preamble_ms = 1000;
    
    config.tail_ms = atoi(paramTail->getValue());
    if (config.tail_ms < 10) config.tail_ms = 10;
    if (config.tail_ms > 500) config.tail_ms = 500;
    
    config.update_interval_min = atoi(paramUpdateInterval->getValue());
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
    
    // Clean up old parameters if they exist
    delete paramCallsign;
    delete paramSsid;
    delete paramSymbol;
    delete paramSymbolTable;
    delete paramPath1;
    delete paramPath1Ssid;
    delete paramPath2;
    delete paramPath2Ssid;
    delete paramFrequency;
    delete paramPreamble;
    delete paramTail;
    delete paramUpdateInterval;
    
    // Add custom parameters with helpful placeholders and patterns
    WiFiManagerParameter customHeading("<h2>APRS Configuration</h2>");
    wm.addParameter(&customHeading);
    
    paramCallsign = new WiFiManagerParameter("callsign", "Callsign (max 6 chars)", callsignBuf, 10, 
                                       "maxlength='6' pattern='[A-Z0-9]{1,6}' style='text-transform:uppercase'");
    
    paramSsid = new WiFiManagerParameter("ssid", "SSID (0-15, see APRS spec)", ssidBuf, 8,
                                       "type='number' min='0' max='15'");
    
    paramSymbol = new WiFiManagerParameter("symbol", "Symbol (n=car, /=dot, etc)", symbolBuf, 4,
                                       "maxlength='1' placeholder='n'");
    
    paramSymbolTable = new WiFiManagerParameter("symbol_table", "Symbol Table (/ or \\\\)", symbolTableBuf, 4,
                                       "maxlength='1' placeholder='/'");
    
    WiFiManagerParameter pathHeading("<h3>Digipeater Path</h3>");
    wm.addParameter(&pathHeading);
    
    paramPath1 = new WiFiManagerParameter("path1", "Path 1 (e.g., WIDE1)", path1Buf, 10);
    paramPath1Ssid = new WiFiManagerParameter("path1_ssid", "Path 1 SSID (1-7)", path1SsidBuf, 8,
                                       "type='number' min='1' max='7'");
    paramPath2 = new WiFiManagerParameter("path2", "Path 2 (e.g., WIDE2)", path2Buf, 10);
    paramPath2Ssid = new WiFiManagerParameter("path2_ssid", "Path 2 SSID (1-7)", path2SsidBuf, 8,
                                       "type='number' min='1' max='7'");
    
    WiFiManagerParameter radioHeading("<h3>Radio Settings</h3>");
    wm.addParameter(&radioHeading);
    
    paramFrequency = new WiFiManagerParameter("frequency", "Frequency (MHz, e.g., 144.9900)", frequencyBuf, 16,
                                       "type='number' step='0.0001' min='144' max='148'");
    paramPreamble = new WiFiManagerParameter("preamble", "PTT Preamble (ms, 100-1000)", preambleBuf, 8,
                                      "type='number' min='100' max='1000'");
    paramTail = new WiFiManagerParameter("tail", "PTT Tail (ms, 10-500)", tailBuf, 8,
                                   "type='number' min='10' max='500'");
    paramUpdateInterval = new WiFiManagerParameter("update_interval", "Update Interval (minutes, 1-60)", updateIntervalBuf, 8,
                                             "type='number' min='1' max='60'");
    
    // Add all parameters
    wm.addParameter(paramCallsign);
    wm.addParameter(paramSsid);
    wm.addParameter(paramSymbol);
    wm.addParameter(paramSymbolTable);
    wm.addParameter(paramPath1);
    wm.addParameter(paramPath1Ssid);
    wm.addParameter(paramPath2);
    wm.addParameter(paramPath2Ssid);
    wm.addParameter(paramFrequency);
    wm.addParameter(paramPreamble);
    wm.addParameter(paramTail);
    wm.addParameter(paramUpdateInterval);
    
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
