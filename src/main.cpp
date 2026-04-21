#include <Arduino.h>
#include "config.h"
#include "crsf.h"

CRSF crsf;
TaskHandle_t FlightTaskHandle;
TaskHandle_t TelemetryTaskHandle;

// CORE 0: Flight Logic
void FlightTask(void *pvParameters) {
    for (;;) {
        crsf.update();

        for (int i = 0; i < TOTAL_CHANNELS; i++) {
            int pwmUs;
            if (crsf.isLinkUp()) {
                pwmUs = calculate_pwm_us(crsf.getChannel(i));
            } else {
                // Failsafe Logic:
                // CH3 (Throttle) is Index 2 in AETR.
                // Set to 988us (Min) to stop motor.
                pwmUs = (i == 2) ? 988 : 1500;
            }
            uint32_t duty = (pwmUs * 65535) / 20000;
            ledcWrite(i, duty);
        }
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

// CORE 1: Telemetry & Monitoring
void TelemetryTask(void *pvParameters) {
    for (;;) {
        if (crsf.isLinkUp()) {
            float vbat = (analogRead(VBAT_PIN) * 3.3 / 4095.0) * BATTERY_DIVIDER_RATIO;

            Serial.print("[AETR] ");
            for(int i=0; i<4; i++) Serial.printf("CH%d:%-4d ", i+1, crsf.getChannel(i));
            Serial.printf("| AUX: %d %d | V:%.2fV\n", crsf.getChannel(4), crsf.getChannel(5), vbat);

            crsf.sendBattery(vbat, 0.0, 0, 100);
        } else {
            Serial.println("[WARN] NO SIGNAL - FAILSAFE ENGAGED");
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup() {
    Serial.begin(115200);
    Serial2.begin(CRSF_BAUDRATE, SERIAL_8N1, CRSF_RX_PIN, CRSF_TX_PIN);
    crsf.begin(Serial2);

    for (int i = 0; i < TOTAL_CHANNELS; i++) {
        ledcSetup(i, PWM_FREQ, PWM_RES);
        ledcAttachPin(PWM_PINS[i], i);
    }

    xTaskCreatePinnedToCore(FlightTask, "Flight", 4096, NULL, 3, &FlightTaskHandle, 0);
    xTaskCreatePinnedToCore(TelemetryTask, "Tele", 4096, NULL, 1, &TelemetryTaskHandle, 1);
}

void loop() { vTaskDelete(NULL); }