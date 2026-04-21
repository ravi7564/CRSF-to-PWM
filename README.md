## ESP32 High-Performance CRSF-to-PWM Converter (8-Channel)

This is a professional, dual-core optimized firmware for converting TBS Crossfire or ELRS (ExpressLRS) protocols into 8-channel high-precision PWM signals. Built for RC aircraft and drones requiring maximum stability and ultra-low latency.

## ✨ Features

- **Dual-Core Execution:** * Core 0: Dedicated to high-speed CRSF packet decoding and PWM signal generation
- **Core 1:** Handles telemetry (V-Bat) and real-time monitoring via the Serial Terminal.
- **AETR Channel Mapping:** Pre-configured with the standard Aileron, Elevator, Throttle, and Rudder sequence.
- **8-Channel "Safe" GPIOs:** Specifically uses pins that do not interfere with the ESP32 boot process (strapping pins avoided).
- **High Precision PWM:** Utilizes 16-bit resolution (0-65535) for surgical precision in servo movements.
- **Safety Failsafe:** Instantly forces Throttle (CH3) to minimum and flight surfaces to neutral if the radio link is lost.

## 🔌 Connection Diagram:

| Component Pin | ESP32 Pin | Description |
| :--- | :--- | :--- |
| **CRSF TX (Receiver)** | **GPIO 16 (RX2)** | Serial Data Input to ESP32|
| **CRSF RX (Receiver)** | **GPIO 17 (TX2)** | Serial Data Output for Telemetry|
| **VCC (Receiver)** | **V**IN / 5V** | Power (Ensure Receiver supports 5V)|
| **GND** | **GND** | Common Ground (Must be shared with ESC)|
| **VBAT (Battery +)** | **GPIO 34** | Voltage Sensing (Via 1:11 Divider)|

## 🕹️ PWM Output (AETR Sequence):

| Channel | Function |GPIO Pin
| :--- | :--- | :--- |
| **CH1** | **Aileron** | 13
| **CH2** | **Elevator** | 14
| **CH3** | **Throttle** | 27
| **CH4** | **Rudder** | 18
| **CH5** | **AUX 1** | 19
| **CH6** | **AUX 2** | 21
| **CH7** | **AUX 3** | 25
| **CH8** | **AUX 4** | 26

## 📁 Project Structure (PlatformIO)
```
.
├── include/
│   └── config.h          // Global constants and Pin maps
├── lib/
│   └── CRSF/
│       ├── crsf.h        // Library Interface
│       └── crsf.cpp      // Protocol Parser & Bit-unpacking logic
├── src/
│   └── main.cpp          // FreeRTOS Task Management & PWM output
└── platformio.ini        // Build flags and Serial Monitor settings

```

## 🛠️ Installation & Build

1. Environment:Ensure you have PlatformIO installed (VS Code extension).
2. Configuration: Update your platformio.ini with the following flags:
monitor_speed = 115200
build_flags = -I include
3. Upload: Connect your ESP32 to your laptop and click the Upload button.
4. Monitor: Open the Serial Monitor (115200 baud) to view live channel data  and battery status.

## ⚠️ Important Safety Warnings

1. Failsafe Check: Always remove propellers before testing. Turn off your transmitter to ensure the code triggers the failsafe (Throttle should drop to ~988us).
2. Voltage Divider: Never connect a LiPo battery directly to GPIO 34. Use a voltage divider (e.g., 100kΩ and 10kΩ resistors) to scale the voltage down to < 3.3V.
3. Ground Loop: Ensure a thick ground wire connects the ESC to the ESP32 to prevent signal jitter.
