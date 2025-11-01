#ifndef dra818_h
#define dra818_h

#include "Arduino.h"
#include <Stream.h>

// ERROR DEFINITION

#define DRA818_CONF_OK 0
#define DRA818_ERR_FREC 1
#define DRA818_ERR_VOL 2
#define DRA818_ERR_CTCSS 3
#define DRA818_ERR_SQL 4
#define DRA818_ERR_NOT_PRESENT 5
#define DRA818_ERR_FILTER_SET 6

#define default_timeout_ms 250
#define DRA818_ON HIGH
#define DRA818_OF LOW
#define PTT_ON LOW
#define PTT_OFF HIGH

// TXT POW PIN LOGIC IS ACTIVE ON LOW.
#define DRA818_LOW_POW HIGH
#define DRA818_HIGH_POW LOW
// ON State is 1 on pd_pin
#define DRA818_ON_STATE HIGH
#define DRA818_OFF_STATE LOW

#define READ_BUFF_LEN 255
#define SERIAL_READ_TIMEOUT_MS 500

class dra818 {
public:
    dra818(uint8_t ptt_pin, uint8_t pd_pin, uint8_t txt_pow_pin);

    // Initialize Internal values
    uint8_t configure(Stream* serial, float rx_freq, float tx_freq, uint8_t tx_ctcss, uint8_t rx_ctcss, uint8_t sql, uint8_t out_vol, uint8_t mic_vol, bool preemph, bool highpass, bool lowpass);

    uint8_t begin();
    void ptt(uint8_t state);
    void setModulePowerState(uint8_t power_state);
    void setTxtPower(uint8_t power);
    bool modulePresent();
    bool writeFreq();
    bool setOutputVolume();
    bool setMicVolume();
    bool setFilters();

private:
    Stream* serial;
    uint8_t ptt_pin;
    uint8_t pd_pin;
    uint8_t txt_pow_pin;
    uint8_t tx_ctcss;
    uint8_t rx_ctcss;
    float tx_freq;
    float rx_freq;
    uint8_t out_vol;
    uint8_t mic_vol;
    uint8_t squelch;
    uint8_t preemph;
    uint8_t highpass;
    uint8_t lowpass;
    void delay_ms(int delay_ms);

    bool sendCommand(char* command, char* reply);
};
#endif
