#include "crsf.h"
#include "config.h"

CRSF::CRSF() {
    _lastPacketTime = 0;
    for(int i=0; i<16; i++) _channels[i] = 992;
}

void CRSF::begin(HardwareSerial &serial) {
    _serial = &serial;
}

uint8_t CRSF::_crc8(uint8_t *ptr, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= ptr[i];
        for (uint8_t j = 0; j < 8; j++) {
            crc = (crc & 0x80) ? (crc << 1) ^ 0xD5 : (crc << 1);
        }
    }
    return crc;
}

void CRSF::_unpackRC(uint8_t *p) {
    _channels[0]  = (p[0]       | p[1] << 8) & 0x07FF;
    _channels[1]  = (p[1] >> 3  | p[2] << 5) & 0x07FF;
    _channels[2]  = (p[2] >> 6  | p[3] << 2 | p[4] << 10) & 0x07FF;
    _channels[3]  = (p[4] >> 1  | p[5] << 7) & 0x07FF;
    _channels[4]  = (p[5] >> 4  | p[6] << 4) & 0x07FF;
    _channels[5]  = (p[6] >> 7  | p[7] << 1 | p[8] << 9) & 0x07FF;
    _channels[6]  = (p[8] >> 2  | p[9] << 6) & 0x07FF;
    _channels[7]  = (p[9] >> 5  | p[10] << 3) & 0x07FF;
    _channels[8]  = (p[11]      | p[12] << 8) & 0x07FF;
    _channels[9]  = (p[12] >> 3 | p[13] << 5) & 0x07FF;
    _channels[10] = (p[13] >> 6 | p[14] << 2 | p[15] << 10) & 0x07FF;
    _channels[11] = (p[15] >> 1 | p[16] << 7) & 0x07FF;
    _channels[12] = (p[16] >> 4 | p[17] << 4) & 0x07FF;
    _channels[13] = (p[17] >> 7 | p[18] << 1 | p[19] << 9) & 0x07FF;
    _channels[14] = (p[19] >> 2 | p[20] << 6) & 0x07FF;
    _channels[15] = (p[20] >> 5 | p[21] << 3) & 0x07FF;
}

bool CRSF::update() {
    static uint8_t idx = 0;
    while (_serial->available()) {
        uint8_t b = _serial->read();
        if (idx == 0 && b != CRSF_ADDRESS_FC) continue;
        _rxBuffer[idx++] = b;
        if (idx >= 2) {
            uint8_t len = _rxBuffer[1];
            if (idx == len + 2) {
                uint8_t calcCrc = _crc8(&_rxBuffer[2], len - 1);
                if (calcCrc == _rxBuffer[idx - 1]) {
                    if (_rxBuffer[2] == CRSF_FRAMETYPE_RC_CHANNELS) {
                        _unpackRC(&_rxBuffer[3]);
                        _lastPacketTime = millis();
                        idx = 0;
                        return true;
                    }
                }
                idx = 0;
            }
        }
    }
    return false;
}

uint16_t CRSF::getChannel(uint8_t ch) { return (ch < 16) ? _channels[ch] : 992; }
bool CRSF::isLinkUp() { return (millis() - _lastPacketTime < FAILSAFE_TIMEOUT_MS); }

void CRSF::sendBattery(float volts, float amps, uint32_t mah, uint8_t percent) {
    uint16_t v = (uint16_t)(volts * 10);
    uint16_t a = (uint16_t)(amps * 10);
    uint8_t frame[12] = { CRSF_ADDRESS_RADIO, 10, CRSF_FRAMETYPE_BATTERY_SENSOR,
                          (uint8_t)(v >> 8), (uint8_t)(v & 0xFF), (uint8_t)(a >> 8), (uint8_t)(a & 0xFF),
                          (uint8_t)(mah >> 16), (uint8_t)(mah >> 8), (uint8_t)(mah & 0xFF), percent, 0 };
    frame[11] = _crc8(&frame[2], 9);
    _serial->write(frame, 12);
}