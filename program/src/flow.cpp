#include "flow.h"
#include <Arduino.h>

#include "components.h"
#include "routines.h"
#include "network.h"

void SetupProgram() {
    SetupComponents();
    
    ResetToPerminantSwitchPorts();
    ResetToAllPerminantRoutines();
}

bool wifiOn = false;

void UpdateProgram() {
    if (GetWifiSwitchStatus() && !wifiOn) {
        SetupNetwork();
        wifiOn = true;
        
        SetLED(true);
        Serial.println("Wifi On");
        
        delay(20);
    } else if (!GetWifiSwitchStatus() && wifiOn) {
        StopNetwork();
        wifiOn = false;
        
        SetLED(false);
        Serial.println("Wifi Off");
        
        delay(20);
    }
    UpdateSwitch(wifiOn);
    
    if (wifiOn) {
        UpdateNetwork();
    }
    else {
        esp_sleep_enable_timer_wakeup(200 * 1000ULL); // 200 ms
        esp_light_sleep_start();
    }
}
