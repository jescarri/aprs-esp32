#ifndef RADIOMANAGER_H
#define RADIOMANAGER_H

#include <Arduino.h>
#include <dra818.h>
#include "hardware_config.h"

/**
 * RadioManager - Clean wrapper for DRA818 radio module
 * 
 * Handles initialization and configuration of the DRA818 VHF transceiver
 * Provides simple interface for power control and basic settings
 */
class RadioManager {
public:
    struct RadioConfig {
        float frequency = DEFAULT_RADIO_FREQ;
        uint8_t squelch_level = DEFAULT_RADIO_SQUELCH;
        uint8_t volume = DEFAULT_RADIO_VOLUME;
        uint8_t mic_gain = DEFAULT_RADIO_MIC_GAIN;
        bool narrow_band = false;
        bool rx_enable = true;
        bool tx_enable = true;
    };
    
    RadioManager();
    
    /**
     * Initialize the radio module
     * 
     * @param serial Serial port for AT commands
     * @param pd_pin Power down control pin
     * @param ptt_pin PTT control pin
     * @param tx_pow_pin TX power selection pin
     * @param config Radio configuration
     * @return true on success
     */
    bool begin(HardwareSerial* serial,
               gpio_num_t pd_pin,
               gpio_num_t ptt_pin,
               gpio_num_t tx_pow_pin,
               const RadioConfig& config);
    
    /**
     * Configure radio parameters
     */
    bool configure(const RadioConfig& config);
    
    /**
     * Power down the radio module
     */
    void setPowerDown(bool powerdown);
    
    /**
     * Set TX power level
     * @param high_power true = 1W, false = 0.5W
     */
    void setTxPower(bool high_power);
    
    /**
     * Get initialization status
     */
    bool isInitialized() const { return _initialized; }
    
private:
    dra818 _radio;
    HardwareSerial* _serial;
    RadioConfig _config;
    gpio_num_t _pd_pin;
    gpio_num_t _ptt_pin;
    gpio_num_t _tx_pow_pin;
    bool _initialized;
};

#endif // RADIOMANAGER_H
