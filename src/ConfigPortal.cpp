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
    
    // SSID Dropdown with descriptions
    char ssidHtml[1024];
    snprintf(ssidHtml, sizeof(ssidHtml),
        "<label for='ssid'>SSID (Station Type)</label>"
        "<select name='ssid' id='ssid'>"
        "<option value='0'%s>0 - 🏠 Primary station (usually home)</option>"
        "<option value='7'%s>7 - 📱 Handheld radio</option>"
        "<option value='9'%s>9 - 🚗 Mobile (car/truck)</option>"
        "<option value='1'%s>1 - 📡 Generic/Digipeater</option>"
        "<option value='5'%s>5 - 🌐 Other networks</option>"
        "<option value='8'%s>8 - ⛵ Boats/ships/aircraft</option>"
        "<option value='10'%s>10 - 💻 Internet/D-STAR</option>"
        "<option value='11'%s>11 - 🎈 Balloons</option>"
        "<option value='12'%s>12 - 🎒 Portable (camping)</option>"
        "<option value='13'%s>13 - 🌦️ Weather station</option>"
        "<option value='14'%s>14 - 🚚 Trucking</option>"
        "<option value='15'%s>15 - ➕ Generic additional</option>"
        "</select>",
        atoi(ssidBuf)==0 ? " selected" : "",
        atoi(ssidBuf)==7 ? " selected" : "",
        atoi(ssidBuf)==9 ? " selected" : "",
        atoi(ssidBuf)==1 ? " selected" : "",
        atoi(ssidBuf)==5 ? " selected" : "",
        atoi(ssidBuf)==8 ? " selected" : "",
        atoi(ssidBuf)==10 ? " selected" : "",
        atoi(ssidBuf)==11 ? " selected" : "",
        atoi(ssidBuf)==12 ? " selected" : "",
        atoi(ssidBuf)==13 ? " selected" : "",
        atoi(ssidBuf)==14 ? " selected" : "",
        atoi(ssidBuf)==15 ? " selected" : "");
    WiFiManagerParameter paramSsidDropdown(ssidHtml);
    
    // Symbol Dropdown with Unicode icons
    char symbolHtml[2048];
    String currentSymbol = String(symbolBuf[0]);
    snprintf(symbolHtml, sizeof(symbolHtml),
        "<label for='symbol'>APRS Symbol (Map Icon)</label>"
        "<select name='symbol' id='symbol'>"
        "<option value='n'%s>🚗 Car/Truck (n)</option>"
        "<option value='>'%s>🚙 Small car (>)</option>"
        "<option value='v'%s>🚐 Van (v)</option>"
        "<option value='b'%s>🚲 Bicycle (b)</option>"
        "<option value='k'%s>🚢 Ship/boat (k)</option>"
        "<option value='s'%s>⛵ Sailboat (s)</option>"
        "<option value='\''%s>✈️ Small aircraft (')</option>"
        "<option value='^'%s>🛩️ Large aircraft (^)</option>"
        "<option value='['%s>👤 Person (jogger) ([)</option>"
        "<option value='-'%s>🏠 House (-)</option>"
        "<option value='_'%s>🌦️ Weather station (_)</option>"
        "<option value='!'%s>🚨 Police/Fire (!)</option>"
        "<option value='a'%s>🚑 Ambulance (a)</option>"
        "<option value='f'%s>🚒 Fire truck (f)</option>"
        "<option value='j'%s>🚙 Jeep (j)</option>"
        "<option value='u'%s>🚚 Truck (u)</option>"
        "<option value='O'%s>🎈 Balloon (O)</option>"
        "<option value='/'%s>⚫ Dot (/)</option>"
        "<option value='R'%s>🚘 Recreational vehicle (R)</option>"
        "<option value='Y'%s>⛵ Yacht (Y)</option>"
        "</select>"
        "<br><small>Symbol appears as icon on APRS maps</small>",
        currentSymbol=="n" ? " selected" : "",
        currentSymbol==">" ? " selected" : "",
        currentSymbol=="v" ? " selected" : "",
        currentSymbol=="b" ? " selected" : "",
        currentSymbol=="k" ? " selected" : "",
        currentSymbol=="s" ? " selected" : "",
        currentSymbol=="'" ? " selected" : "",
        currentSymbol=="^" ? " selected" : "",
        currentSymbol=="[" ? " selected" : "",
        currentSymbol=="-" ? " selected" : "",
        currentSymbol=="_" ? " selected" : "",
        currentSymbol=="!" ? " selected" : "",
        currentSymbol=="a" ? " selected" : "",
        currentSymbol=="f" ? " selected" : "",
        currentSymbol=="j" ? " selected" : "",
        currentSymbol=="u" ? " selected" : "",
        currentSymbol=="O" ? " selected" : "",
        currentSymbol=="/" ? " selected" : "",
        currentSymbol=="R" ? " selected" : "",
        currentSymbol=="Y" ? " selected" : "");
    WiFiManagerParameter paramSymbolDropdown(symbolHtml);
    
    // Symbol Table (Primary / or Alternate \)
    char symbolTableHtml[512];
    String currentTable = String(symbolTableBuf[0]);
    snprintf(symbolTableHtml, sizeof(symbolTableHtml),
        "<label for='symbol_table'>Symbol Table</label>"
        "<select name='symbol_table' id='symbol_table'>"
        "<option value='/'%s>/ - Primary (most common)</option>"
        "<option value='\\'%s>\\ - Alternate (special)</option>"
        "</select>"
        "<br><small>Primary (/) has standard icons, Alternate (\\) has variations</small>",
        currentTable=="/" ? " selected" : "",
        currentTable=="\\" ? " selected" : "");
    WiFiManagerParameter paramSymbolTableDropdown(symbolTableHtml);
    
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
    wm.addParameter(&paramSsidDropdown);
    wm.addParameter(&paramSymbolDropdown);
    wm.addParameter(&paramSymbolTableDropdown);
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
