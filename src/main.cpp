#include <Arduino.h>
#include "config.h"
#include "crsf.h"
#include <WiFi.h>
#include <esp_bt.h>

// Global Objects
CRSF crsf;
TaskHandle_t FlightTaskHandle;
TaskHandle_t TelemetryTaskHandle;

// Shared variables for cross-core communication
volatile int actual_output_us[TOTAL_CHANNELS];
volatile bool isLinkActive = false;

// --- CORE 0: Real-time PWM Generation & Failsafe ---
void FlightTask(void *pvParameters) {
    for (;;) {
        // Update CRSF parser and check link status
        crsf.update();
        isLinkActive = crsf.isLinkUp();

        for (int i = 0; i < TOTAL_CHANNELS; i++) {
            int pwmUs;

            if (isLinkActive) {
                // CASE 1: Link is OK - Direct passthrough
                pwmUs = calculate_pwm_us(crsf.getChannel(i));
            } else {
                // CASE 2: Link is DOWN - Trigger Failsafe
                // CH3 (Throttle - Index 2) -> Minimum (988us) to stop motors
                // All other channels -> Neutral (1500us) for safe gliding/center
                pwmUs = (i == 2) ? 988 : 1500;
            }

            // Record the value for Serial Monitoring
            actual_output_us[i] = pwmUs;

            // Convert Microseconds to 16-bit Duty Cycle
            // Formula: (Time_us / Period_us) * (2^Resolution - 1)
            // Period for 50Hz is 20,000us. Resolution is 65535 (16-bit).
            uint32_t duty = (pwmUs * 65535) / 20000;
            ledcWrite(i, duty);
        }

        // Very small delay to prevent watchdog triggers,
        // but high enough for 500Hz internal processing.
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

// --- CORE 1: Serial Monitoring & Battery Telemetry ---
void TelemetryTask(void *pvParameters) {
    for (;;) {
        // Clear screen logic could be added here, but simple line print is better for debugging
        if (isLinkActive) {
            Serial.print("[LINK: OK]   ");
        } else {
            Serial.print("[LINK: LOST] ");
        }

        // Print what is ACTUALLY going out to the pins
        Serial.print("Outputs: ");
        for (int i = 0; i < TOTAL_CHANNELS; i++) {
            Serial.printf("P%d:%4d ", i + 1, actual_output_us[i]);
            if (i == 3) Serial.print("| "); // Visual break after AETR
        }

        // Add Battery Voltage
        float vbat = (analogRead(VBAT_PIN) * 3.3 / 4095.0) * BATTERY_DIVIDER_RATIO;
        Serial.printf("| Bat:%.2fV\n", vbat);

        crsf.sendBattery(vbat, 0, 0, 0);
        // 10Hz refresh rate is plenty for the Serial Monitor
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup() {
    // 1. Radio Clean-up for maximum stability and power saving
    WiFi.mode(WIFI_OFF);
    btStop();
    esp_bt_controller_disable();

    // 2. Communications
    Serial.begin(115200);
    Serial2.begin(CRSF_BAUDRATE, SERIAL_8N1, CRSF_RX_PIN, CRSF_TX_PIN);
    crsf.begin(Serial2);

    // 3. Hardware PWM Setup (8 Channels)
    for (int i = 0; i < TOTAL_CHANNELS; i++) {
        ledcSetup(i, PWM_FREQ, PWM_RES);
        ledcAttachPin(PWM_PINS[i], i);
        actual_output_us[i] = (i == 2) ? 988 : 1500; // Initialize with safe values
    }

    // 4. Multi-threading Task Creation
    // Core 0 handles the critical flight/PWM loop
    xTaskCreatePinnedToCore(FlightTask, "FlightLoop", 4096, NULL, 3, &FlightTaskHandle, 0);

    // Core 1 handles Serial/Telemetry and non-critical tasks
    xTaskCreatePinnedToCore(TelemetryTask, "Monitor", 4096, NULL, 1, &TelemetryTaskHandle, 1);

    Serial.println("CRSF to PWM Converter Ready.");
}

void loop() {
    vTaskDelete(NULL);
}