#include <dra818.h>

#define RADIO_AUDIO_OUT 25
#define RADIO_AUDIO_IN 36
#define RADIO_AUDIO_TRIGGER 32
#define RADIO_PTT 33
#define RADIO_PD 23
#define RADIO_TX_POW_PIN 26
#define RADIO_RX 18
#define RADIO_TX 19
// Radio TX POWER 0 LOW 1 HIGH
#define RADIO_TX_POW 0
#define RADIO_FREC 144.500
#define RADIO_SQUELCH_LEVEL 8
#define RADIO_AUDIO_OUTPUT_VOLUME 8
#define RADIO_MIC_VOLUME 8

dra818 radio(RADIO_PTT, RADIO_PD, RADIO_TX_POW_PIN);

void setup()
{
    Serial.begin(115200);
    Serial.println("HELLO!!");
    delay(500);
    Serial2.begin(9600, SERIAL_8N1, RADIO_RX, RADIO_TX);
    while (!Serial2) {
    }
    Serial2.flush();
    delay(500);
    uint8_t init_ok, conf_ok;
    conf_ok = radio.configure(&Serial2, RADIO_FREC, RADIO_FREC, 0, 0, RADIO_SQUELCH_LEVEL, RADIO_AUDIO_OUTPUT_VOLUME, 8, true, true, true);
    Serial.print("Radio Config check: ");
    Serial.println(conf_ok);
    init_ok = radio.begin();
    Serial.print("Radio Begin Code: ");
    Serial.println(init_ok);
    if (init_ok == DRA818_CONF_OK) {
	for (int i = 0; i < 5; i++) {
	    radio.ptt(PTT_ON);
	    delay(1000);
	    radio.ptt(PTT_OFF);
	    delay(1000);
	}
    }
}

void loop()
{
}
