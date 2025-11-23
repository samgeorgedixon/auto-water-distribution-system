#pragma once
#include <vector>

void SetupComponents();

int GetTemp();
bool GetWifiSwitchStatus();
void SetLEDColourRG(int r, int g);

void UpdateSwitchPorts(std::vector<bool> portStates);
void UpdateSwitch();
