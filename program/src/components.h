#pragma once
#include <vector>
#include <stdint.h>

struct Time {
    int sec;
    int min;
    int hour;
    int dayDate; // Starts 1
    int month; // Jan = 1
    int year;
};

void SetupComponents();

int GetTemp();
bool GetWifiSwitchStatus();
void SetLED(bool state);

void SetTime(const Time &time);

void EnableSwitchPorts(std::vector<uint32_t> ports);
void DisableSwitchPorts(std::vector<uint32_t> ports);
void UpdateSwitchPorts(int bitWidth);

Time GetTimeNow();
void SetTime(const Time& time);

unsigned int GetTimeNowSeconds();
unsigned int ConvertTimeToSeconds(const Time &time);

