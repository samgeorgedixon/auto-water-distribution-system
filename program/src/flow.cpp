#include "flow.h"
#include <Arduino.h>

#include "components.h"
#include "routines.h"
#include "network.h"

void SetupProgram() {
    SetupComponents();
    ResetToAllPerminantRoutines();
}

bool WifiOn = false;

void UpdateProgram() {
    UpdateSwitch();

    if (GetWifiSwitchStatus() && !WifiOn) {
        SetupNetwork();
        WifiOn = true;

        Serial.println("Wifi On");
        delay(20);
    } else if (!GetWifiSwitchStatus() && WifiOn) {
        StopNetwork();
        WifiOn = false;

        Serial.println("Wifi Off");
        delay(20);
    }

    if (WifiOn) {
        UpdateNetwork();
    }
}
