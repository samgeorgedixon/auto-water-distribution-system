#pragma once

//#define DEBUG

#define SERIAL_BAUD_RATE 115200
#define SWITCH_BIT_WIDTH 6

// Wi-Fi
#define DEFAULT_SSID "ris"
#define DEFAULT_PASSWORD "admin123"

// Pins
#define LED_PIN 19              // Out
#define TEMP_SENSOR_PIN 23      // In/Out
#define WIFI_SWITCH_PIN 25      // In

// GPIO 21  (I²C SDA)
// GPIO 22  (I²C SCL)

// Outputs to 74HC595 Chip
#define SWITCH_DATA_PIN 18      // Out
#define SWITCH_SRCLK_PIN 17     // Out: Write Internal Clock
#define SWITCH_RCLK_PIN 16      // Out: Output Clock
