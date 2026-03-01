#include "components.h"

#include <stdint.h>

#include <Arduino.h>
#include <DHT.h>
#include <RTClib.h>
#include <Wire.h>
#include <esp_sleep.h>

#include "core.h"

DHT dht(TEMP_SENSOR_PIN, DHT22);

RTC_DS3231 rtcExternal;
RTC_Millis rtcInternal;
bool externalRTC = true;

std::vector<uint8_t> portStates;
std::vector<uint8_t> currentPortStates;

void SetupComponents() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(WIFI_SWITCH_PIN, INPUT_PULLDOWN);
    
    pinMode(SWITCH_DATA_PIN, OUTPUT);
    pinMode(SWITCH_SRCLK_PIN, OUTPUT);
    pinMode(SWITCH_RCLK_PIN, OUTPUT);
    
    dht.begin();
    Wire.begin();

    portStates = {};
    currentPortStates = {};

    if (!rtcExternal.begin()) {
        LOGf("External RTC Not Found - Using Internal RTC\n");

        rtcInternal.begin(DateTime(2000, 1, 1, 0, 0, 0));

        externalRTC = false;
    }

	LOGf("Components Setup\n");
}

void LightSleep(uint32_t seconds) {
    if (seconds == 0) {
        return;
    }

    LOGf("Light Sleep: %d\n", seconds);
    
    pinMode(WIFI_SWITCH_PIN, INPUT);

    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)WIFI_SWITCH_PIN, 1);
    esp_light_sleep_start();

    pinMode(WIFI_SWITCH_PIN, INPUT_PULLDOWN);
    
    LOGf("Exit Light Sleep\n");
}

int GetTemp() {
    int temp = (int)dht.readTemperature();
    
    if (temp < -40 || temp > 80) {
        temp = 0;
    }
    LOGf("Temp: %d\n", temp);

    return temp;
}
bool GetWifiSwitchStatus() {
    return digitalRead(WIFI_SWITCH_PIN);
    //return true;
}
void SetLED(bool state) {
    digitalWrite(LED_PIN, state);

    LOGf("LED State: %d\n", state);
}

Time GetTime() {
    DateTime now;
    if (externalRTC) {
        now = rtcExternal.now();
    }
    else {
        now = rtcInternal.now();
    }

    Time time = {};
    time.sec = now.second();
    time.min = now.minute();
    time.hour = now.hour();
    time.dayDate = now.day();
    time.month = now.month();
    time.year = now.year();

    return time;
}
void SetTime(const Time& time) {
    DateTime newTime(time.year, time.month, time.dayDate, time.hour, time.min, time.sec);
    
    if (externalRTC) {
        rtcExternal.adjust(newTime);
    }
    else {
        rtcInternal.adjust(newTime);
    }
}

uint32_t GetTimeSeconds() {
    DateTime now;
    if (externalRTC) {
        now = rtcExternal.now();
    }
    else {
        now = rtcInternal.now();
    }
    uint32_t timeSeconds = now.unixtime();
    
    return timeSeconds;
}
void SetTimeSeconds(uint32_t seconds) {
    if (externalRTC) {
        rtcExternal.adjust(DateTime(seconds));
    }
    else {
        rtcInternal.adjust(DateTime(seconds));
    }
}

uint32_t ConvertTimeToSeconds(const Time& time) {
    DateTime newTime(time.year, time.month, time.dayDate, time.hour, time.min, time.sec);

    return newTime.unixtime();
}

void EnableSwitchPorts(std::vector<uint32_t> ports) {
    for (int i = 0; i < ports.size(); i++) {
        while (portStates.size() <= ports[i]) {
            portStates.push_back(0);
        }
        LOGf("Enable Port: %d\n", ports[i]);
        portStates[ports[i]] = 1;
        LOGf("... Port: %d\n", portStates[ports[i]]);
    }
}
void DisableSwitchPorts(std::vector<uint32_t> ports) {
    for (int i = 0; i < ports.size(); i++) {
        while (portStates.size() <= ports[i]) {
            portStates.push_back(0);
        }
        LOGf("Disable Port: %d\n", ports[i]);
        portStates[ports[i]] = 0;
    }
}

void UpdateSwitchPorts(int bitWidth) {
    if (currentPortStates == portStates || portStates.size() == 0 || bitWidth <= 0 || bitWidth > 8) { // Check Changed
        return;
    }

    digitalWrite(SWITCH_DATA_PIN, LOW);
    digitalWrite(SWITCH_SRCLK_PIN, LOW);
	digitalWrite(SWITCH_RCLK_PIN, LOW);
    
    // Write Internally to 74HC595 Shift Register
    int remainingPorts = portStates.size();
    int portIndex = portStates.size() - 1;

    while (remainingPorts > 0) {
        int registerDataBits = (remainingPorts >= bitWidth) ? bitWidth : remainingPorts;
        
        LOGf("Padding: ");
        
        // Padding
        if (registerDataBits == bitWidth) {
            for (int i = 0; i < (8 - bitWidth); i++) {
                digitalWrite(SWITCH_DATA_PIN, LOW);
                
                digitalWrite(SWITCH_SRCLK_PIN, HIGH);
                digitalWrite(SWITCH_SRCLK_PIN, LOW);
                
                LOGf("0, ");
            }
        }
        LOGf("\n");

        LOGf("Update Port Row: ");
        
        // Shift Data Bits
        for (int i = 0; i < registerDataBits; i++) {
            digitalWrite(SWITCH_DATA_PIN, portStates[portIndex]);

            digitalWrite(SWITCH_SRCLK_PIN, HIGH);
            digitalWrite(SWITCH_SRCLK_PIN, LOW);

            LOGf("%d-%d ", portStates[portIndex], portIndex);

            portIndex--;
        }
        LOGf("\n");

        remainingPorts -= registerDataBits;
    }
    
    // Output 74HC595 Shift Register Internals
    digitalWrite(SWITCH_RCLK_PIN, HIGH);
    digitalWrite(SWITCH_RCLK_PIN, LOW);

    digitalWrite(SWITCH_DATA_PIN, LOW);

    currentPortStates = portStates;
}
