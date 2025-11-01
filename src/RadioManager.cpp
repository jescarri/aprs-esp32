#include "RadioManager.h"

RadioManager::RadioManager() 
    : _radio(RADIO_PTT, RADIO_PD),
      _serial(nullptr),
      _pd_pin((gpio_num_t)RADIO_PD),
      _ptt_pin((gpio_num_t)RADIO_PTT),
      _initialized(false) {
}

bool RadioManager::begin(HardwareSerial* serial,
                        gpio_num_t pd_pin,
                        gpio_num_t ptt_pin,
                        const RadioConfig& config) {
    _serial = serial;
    _pd_pin = pd_pin;
    _ptt_pin = ptt_pin;
    _config = config;
    
    if (!_serial) {
        return false;
    }
    
    // Note: PD pin power-up is handled in main.cpp with proper timing
    // (2s delay after serial flush, then PD=HIGH, then 1s delay)
    
    // Configure the radio
    uint8_t conf_result = _radio.configure(
        _serial,
        _config.frequency,
        _config.frequency,  // Same freq for RX/TX
        0,                  // CTCSS/DCS off
        0,                  // CTCSS/DCS off
        _config.squelch_level,
        _config.volume,
        _config.mic_gain,
        _config.rx_enable,
        _config.tx_enable,
        !_config.narrow_band  // Wideband = !narrowband
    );
    
    Serial.printf("Radio configure result: %d\n", conf_result);
    
    // Begin communication
    uint8_t begin_result = _radio.begin();
    Serial.printf("Radio begin result: %d\n", begin_result);
    
    if (begin_result == DRA818_CONF_OK) {
        // Test PTT
        _radio.ptt(PTT_ON);
        delay(500);
        _radio.ptt(PTT_OFF);
        
        _initialized = true;
        Serial.println("Radio initialized successfully");
        return true;
    }
    
    Serial.println("Radio initialization failed");
    return false;
}

bool RadioManager::configure(const RadioConfig& config) {
    if (!_initialized || !_serial) {
        return false;
    }
    
    _config = config;
    
    uint8_t result = _radio.configure(
        _serial,
        _config.frequency,
        _config.frequency,
        0, 0,
        _config.squelch_level,
        _config.volume,
        _config.mic_gain,
        _config.rx_enable,
        _config.tx_enable,
        !_config.narrow_band
    );
    
    return (result == DRA818_CONF_OK);
}

void RadioManager::setPowerDown(bool powerdown) {
    _radio.setModulePowerState(powerdown ? LOW : HIGH);
}

bool RadioManager::setMicVolume() {
    return _radio.setMicVolume();
}
