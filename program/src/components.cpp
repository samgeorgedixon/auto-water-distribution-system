#include "components.h"

#include <stdint.h>

#include <Arduino.h>
#include <DHT.h>
#include <time.h>

// #define _PIN 16
#define LED_PIN 17            // Out
#define WIFI_SWITCH_PIN 18      // In
#define TEMP_SENSOR_PIN 19      // In

// Outputs to 74HC595 Chip
#define SWITCH_DATA_PIN 21
#define SWITCH_SRCLK_PIN 22     // WriteInternalClock
#define SWITCH_RCLK_PIN 23      // OutputClock

DHT dht(TEMP_SENSOR_PIN, DHT22);

std::vector<uint8_t> currentPortStates;

void SetupComponents() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(WIFI_SWITCH_PIN, INPUT_PULLDOWN);
    dht.begin();

    pinMode(SWITCH_DATA_PIN, OUTPUT);
    pinMode(SWITCH_SRCLK_PIN, OUTPUT);
    pinMode(SWITCH_RCLK_PIN, OUTPUT);

	Serial.println("Components Setup");
}

int GetTemp() {
    int temp = (int)dht.readTemperature();
	Serial.printf("Temp: %d\n", temp);

    return temp;
}
bool GetWifiSwitchStatus() {
    //return digitalRead(WIFI_SWITCH_PIN);
    return true;
}
void SetLEDColourRG(int r, int g) {
    analogWrite(LED_R_PIN, r);
    analogWrite(LED_G_PIN, g);

    Serial.printf("LED Colour (RG): %d, %d\n", r, g);
}

Time GetTimeNow() {
    time_t now = time(NULL);
    tm *t = localtime(&now);

    Time time;
    time.sec = t->tm_sec;
    time.min = t->tm_min;
    time.hour = t->tm_hour;
    time.dayDate = t->tm_mday;
    time.month = t->tm_mon + 1;
    time.year = t->tm_year + 1900;

    return time;
}
void SetTime(const Time& time) {
    tm timeInfo = {};
    timeInfo.tm_sec  = time.sec;
    timeInfo.tm_min  = time.min;
    timeInfo.tm_hour = time.hour;
    timeInfo.tm_mday = time.dayDate;
    timeInfo.tm_mon  = time.month - 1;
    timeInfo.tm_year = time.year - 1900;

    time_t t = mktime(&timeInfo);
    timeval now = { .tv_sec = t };
    settimeofday(&now, NULL);
}

unsigned int GetTimeNowSeconds() {
    time_t now = time(NULL);
    return (unsigned int)now;
}
unsigned int ConvertTimeToSeconds(const Time& time) {
    tm timeInfo = {};
    timeInfo.tm_sec  = time.sec;
    timeInfo.tm_min  = time.min;
    timeInfo.tm_hour = time.hour;
    timeInfo.tm_mday = time.dayDate;
    timeInfo.tm_mon  = time.month - 1;
    timeInfo.tm_year = time.year - 1900;

    time_t now = mktime(&timeInfo);
    return (unsigned int)now;
}

void UpdateSwitchPorts(std::vector<uint8_t> portStates) {
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
