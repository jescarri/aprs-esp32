#ifndef APRS_PROTOCOL_H
#define APRS_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

namespace APRS {

// ============================================================================
// AFSK Constants
// ============================================================================
#define MARK_FREQ       1200    // Hz
#define SPACE_FREQ      2200    // Hz
#define BITRATE         1200    // bps
#define SAMPLERATE      105600  // 13200 * 8 oversampling
#define SAMPLESPERBIT   (SAMPLERATE / BITRATE)

// ============================================================================
// AX.25 Protocol Constants
// ============================================================================
#define AX25_CTRL_UI        0x03
#define AX25_PID_NOLAYER3   0xF0
#define HDLC_FLAG           0x7E
#define HDLC_RESET          0x7F
#define AX25_ESC            0x1B
#define BIT_STUFF_LEN       5

// ============================================================================
// AX.25 Call Structure
// ============================================================================
struct AX25Call {
    char call[7];       // 6 chars + null terminator
    uint8_t ssid;       // SSID (0-15)
};

// ============================================================================
// Protocol Configuration
// ============================================================================
struct ProtocolConfig {
    uint8_t ptt_pin;        // GPIO pin number
    uint16_t preamble_ms;   // Pre-transmission flags duration
    uint16_t tail_ms;       // Post-transmission flags duration
};

// ============================================================================
// AFSK/AX.25 Protocol Handler
// ============================================================================
class Protocol {
public:
    /**
     * Initialize the protocol layer
     * @param config Protocol configuration
     * @return true on success
     */
    bool begin(const ProtocolConfig& config);
    
    /**
     * Send an AX.25 packet via AFSK modulation
     * 
     * @param src Source callsign
     * @param dst Destination callsign
     * @param path Array of path callsigns
     * @param path_len Number of path elements (typically 2: WIDE1-1, WIDE2-2)
     * @param payload Packet payload
     * @param payload_len Payload length
     * @return true on success
     */
    bool sendPacket(const AX25Call& src,
                   const AX25Call& dst,
                   const AX25Call* path,
                   size_t path_len,
                   const uint8_t* payload,
                   size_t payload_len);
    
    /**
     * Check if transmission is in progress
     */
    bool isBusy() const { return _transmitting; }
    
    /**
     * Set PTT (Push-to-Talk) state
     */
    void setPTT(bool enable);
    
private:
    ProtocolConfig _config;
    bool _transmitting;
    
    // Internal transmission state
    uint16_t _phaseAcc;
    uint16_t _phaseInc;
    uint8_t _sampleIndex;
    uint8_t _currentOutputByte;
    uint8_t _txBit;
    uint8_t _bitstuffCount;
    bool _bitStuff;
    uint16_t _preambleLength;
    uint16_t _tailLength;
    uint16_t _crc;
    
    // FIFO buffer for packet data
    static const size_t FIFO_SIZE = 512;
    uint8_t _fifo[FIFO_SIZE];
    size_t _fifo_head;
    size_t _fifo_tail;
    
    // Helper methods
    void initI2S();
    void sendAFSK();
    uint8_t generateSample();
    void putByte(uint8_t byte);
    void sendCall(const AX25Call& call, bool last);
    uint16_t updateCRC(uint8_t byte, uint16_t crc);
    uint8_t sinSample(uint16_t phase);
    
    // FIFO operations
    bool fifoIsEmpty() const;
    bool fifoIsFull() const;
    void fifoPush(uint8_t byte);
    uint8_t fifoPop();
    void fifoFlush();
};

} // namespace APRS

#endif // APRS_PROTOCOL_H
