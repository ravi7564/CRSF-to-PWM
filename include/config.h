#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// CRSF & Battery Pins
#define VBAT_PIN        34
#define CRSF_RX_PIN     16
#define CRSF_TX_PIN     17

/**
 * 8 Pins Assignment (AETR + AUX)
 * CH1 (Aileron):  13
 * CH2 (Elevator): 14
 * CH3 (Throttle): 27
 * CH4 (Rudder):   18
 CH5-CH8 (AUX): GPIO 19, 21, 25, 26
 */
const int PWM_PINS[] = {13, 14, 27, 18, 19, 21, 25, 26};

#define TOTAL_CHANNELS  8
#define PWM_FREQ        50
#define PWM_RES         16
#define BATTERY_DIVIDER_RATIO 11.0
#define FAILSAFE_TIMEOUT_MS   500

inline int calculate_pwm_us(uint16_t crsf_val) {
    return (crsf_val * 1024 / 1639) + 881;
}

#endif