#ifndef CRSF_H
#define CRSF_H

#include <Arduino.h>

// Protocol Definitions
#define CRSF_ADDRESS_FC                 0xC8
#define CRSF_ADDRESS_RADIO              0xEA
#define CRSF_BAUDRATE                   420000
#define CRSF_FRAMETYPE_RC_CHANNELS      0x16
#define CRSF_FRAMETYPE_BATTERY_SENSOR   0x08

class CRSF {
public:
    CRSF();
    void begin(HardwareSerial &serial);
    bool update();
    uint16_t getChannel(uint8_t ch);
    bool isLinkUp();
    void sendBattery(float volts, float amps, uint32_t mah, uint8_t percent);

private:
    HardwareSerial* _serial;
    uint16_t _channels[16];
    uint8_t _rxBuffer[64];
    unsigned long _lastPacketTime;
    uint8_t _crc8(uint8_t *ptr, uint8_t len);
    void _unpackRC(uint8_t *payload);
};

#endif