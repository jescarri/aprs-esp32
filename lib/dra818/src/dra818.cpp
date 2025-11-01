#include "dra818.h"
#include "Arduino.h"
#include <Stream.h>
#include <stdio.h>

dra818::dra818(uint8_t ptt_pin, uint8_t pd_pin, uint8_t txt_pow_pin)
{
    this->ptt_pin = ptt_pin;
    this->pd_pin = pd_pin;
    this->txt_pow_pin = txt_pow_pin;
}

uint8_t dra818::configure(Stream* serial, float rx_freq, float tx_freq, uint8_t tx_ctcss, uint8_t rx_ctcss, uint8_t sql, uint8_t out_vol, uint8_t mic_vol, bool preemph, bool highpass, bool lowpass)
{
    this->serial = serial;
    //Initialize the PTT and PD pins
    pinMode(this->ptt_pin, OUTPUT);
    pinMode(this->pd_pin, OUTPUT);
    pinMode(this->txt_pow_pin, OUTPUT);

    // ToDo: Add better validation of tx/rx frequency, tx and rx should be in the same band.
    if ((tx_freq > 136.0 && tx_freq < 174.00) || (tx_freq > 410.0 && tx_freq < 480.000)) {
	this->tx_freq = tx_freq;
    } else {
	return DRA818_ERR_FREC;
    }
    if ((rx_freq > 136.0 && rx_freq < 174.00) || (rx_freq > 410.0 && rx_freq < 480.000)) {
	this->rx_freq = rx_freq;
    } else {
	return DRA818_ERR_FREC;
    }
    if (tx_ctcss <= 38) {
	this->tx_ctcss = tx_ctcss;
    } else {
	return DRA818_ERR_CTCSS;
    }
    if (rx_ctcss <= 38) {
	this->rx_ctcss = rx_ctcss;
    } else {
	return DRA818_ERR_CTCSS;
    }
    if (sql <= 8) {
	this->squelch = sql;
    } else {
	return DRA818_ERR_SQL;
    }
    if (out_vol >= 1 || out_vol <= 8) {
	this->out_vol = out_vol;
    } else {
	return DRA818_ERR_VOL;
    }
    if (mic_vol >= 1 || mic_vol <= 8) {
	this->mic_vol = mic_vol;
    } else {
	return DRA818_ERR_VOL;
    }
    this->preemph = preemph ? 0 : 1;
    this->highpass = highpass ? 0 : 1;
    this->lowpass = lowpass ? 0 : 1;
    return DRA818_CONF_OK;
}

uint8_t dra818::begin()
{
    setModulePowerState(DRA818_ON);
    delay_ms(1000);

    if (!modulePresent()) {
	return DRA818_ERR_NOT_PRESENT;
    }
    if (!writeFreq()) {
	return DRA818_ERR_FREC;
    }
    if (!setFilters()) {
	return DRA818_ERR_FILTER_SET;
    }
    if (!setOutputVolume()) {
	return DRA818_ERR_VOL;
    }
    return DRA818_CONF_OK;
}

void dra818::ptt(uint8_t state)
{
    digitalWrite(this->ptt_pin, state);
}

void dra818::setModulePowerState(uint8_t power_state)
{
    digitalWrite(this->pd_pin, power_state);
    if (power_state == DRA818_ON_STATE) {
	delay_ms(500);
    }
}

void dra818::setTxtPower(uint8_t power)
{
    digitalWrite(this->txt_pow_pin, power);
}

bool dra818::modulePresent()
{
    char buffer[READ_BUFF_LEN];
    sprintf(buffer, "AT+DMOCONNECT\r\n");
    return sendCommand(buffer, "+DMOCONNECT:0");
}

bool dra818::writeFreq()
{
    char rx_freq_buff[10];
    char tx_freq_buff[10];
    char buffer[READ_BUFF_LEN];
    dtostrf(this->rx_freq, 8, 4, rx_freq_buff);
    dtostrf(this->tx_freq, 8, 4, tx_freq_buff);
    sprintf(buffer, "AT+DMOSETGROUP=0,%s,%s,%04d,%1d,%04d\r\n", tx_freq_buff, rx_freq_buff, this->tx_ctcss, this->squelch, this->rx_ctcss);
    return sendCommand(buffer, "+DMOSETGROUP:0");
}

bool dra818::setOutputVolume()
{
    char buffer[READ_BUFF_LEN];
    sprintf(buffer, "AT+DMOSETVOLUME=%1d\r\n", this->out_vol);
    return sendCommand(buffer, "+DMOSETVOLUME:0");
}

bool dra818::setMicVolume()
{
    char buffer[READ_BUFF_LEN];
    sprintf(buffer, "AT+DMOSETMIC=%1d,0\r\n", this->mic_vol);
    return sendCommand(buffer, "+DMOSETMIC");
}

bool dra818::setFilters()
{
    char buffer[READ_BUFF_LEN];
    sprintf(buffer, "AT+SETFILTER=%1d,%1d,%1d\r\n", this->preemph, this->highpass, this->lowpass);
    return sendCommand(buffer, "+DMOSETFILTER:0");
}

bool dra818::sendCommand(char* command, char* reply)
{
    char readBuffer[READ_BUFF_LEN];
    unsigned long timer;
    ptt(PTT_OFF);
    delay_ms(250);
    this->serial->flush();
    while (this->serial->available()) {
	serial->read();
    }
    this->serial->write(command);
    delay_ms(100);
    uint8_t idx = 0;
    timer = millis();
    while (millis() - timer < SERIAL_READ_TIMEOUT_MS) {
	if (this->serial->available() > 0) {
	    readBuffer[idx] = this->serial->read();
	    idx++;
	    readBuffer[idx] = '\0';
	    if (strcmp(readBuffer, reply) == 0) {
		return true;
	    }
	    if (idx == READ_BUFF_LEN) {
		return false;
	    }
	}
    }
    return false;
}

void dra818::delay_ms(int delay_ms)
{
    unsigned long x = 0;
    while (x < delay_ms) {
	delay(1);
	x += 1;
    }
}
