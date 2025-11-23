#include <Arduino.h>

#include "flow.h"

void setup() {
	Serial.begin(115200);
	SetupProgram();

	Serial.println("ESP32 Setup");
}

void loop() {
	UpdateProgram();
}
