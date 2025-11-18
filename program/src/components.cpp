#include "components.h"

#include <Arduino.h>
#include "DHT.h"

#define LED_R_PIN 16            // Out
#define LED_G_PIN 17            // Out
#define WIFI_SWITCH_PIN 18      // In
#define TEMP_SENSOR_PIN 19      // In

// Outputs to 74HC595 Chip
#define SWITCH_DATA_PIN 21
#define SWITCH_SRCLK_PIN 22     // WriteInternalClock
#define SWITCH_RCLK_PIN 23      // OutputClock

DHT dht(TEMP_SENSOR_PIN, DHT22);

std::vector<bool> currentPortStates;

void SetupComponents() {
    pinMode(LED_R_PIN, OUTPUT);
    pinMode(LED_G_PIN, OUTPUT);
    pinMode(WIFI_SWITCH_PIN, INPUT);
    dht.begin();

    pinMode(SWITCH_DATA_PIN, OUTPUT);
    pinMode(SWITCH_SRCLK_PIN, OUTPUT);
    pinMode(SWITCH_RCLK_PIN, OUTPUT);
}

float GetTemp() {
    return dht.readTemperature();
}

bool GetWifiSwitchStatus() {
    return digitalRead(WIFI_SWITCH_PIN);
}

void SetLEDColourRG(int r, int g) {
    analogWrite(LED_R_PIN, r);
    analogWrite(LED_G_PIN, g);
}

void UpdateSwitchPorts(std::vector<bool> portStates) {
    if (currentPortStates == portStates) { // Check Changed
        return;
    }

    // Write Internally to 74HC595 Shift Register
    for (int i = portStates.size(); i >= 0; i--) {
        digitalWrite(SWITCH_DATA_PIN, portStates[i]);
        delay(2);
        digitalWrite(SWITCH_SRCLK_PIN, HIGH);
        delay(2);
        digitalWrite(SWITCH_SRCLK_PIN, LOW);
        delay(2);
    }
    digitalWrite(SWITCH_DATA_PIN, LOW);
    delay(2);
    
    // Output 74HC595 Shift Register Internals
    digitalWrite(SWITCH_RCLK_PIN, HIGH);
    delay(2);
    digitalWrite(SWITCH_RCLK_PIN, LOW);

    currentPortStates = portStates;
}
