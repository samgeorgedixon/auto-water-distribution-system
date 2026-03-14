#include <Arduino.h>

#include "core.h"
#include "flow.h"

void setup() {
	Serial.begin(SERIAL_BAUD_RATE);
	SetupProgram();

	LOGf("ESP32 Setup\n");
}

void loop() {
	UpdateProgram();
}
