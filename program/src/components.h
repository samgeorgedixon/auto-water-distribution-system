#pragma once
#include <vector>
#include <stdint.h>

struct Time {
    int sec;
    int min;
    int hour;
    int dayDate; // 1st = 1
    int month; // Jan = 1
    int year;
};

void SetupComponents();

void LightSleep(uint32_t seconds);

int GetTemp();
bool GetWifiSwitchStatus();
void SetLED(bool state);

void SetTime(const Time &time);

void EnableSwitchPorts(std::vector<uint32_t> ports);
void DisableSwitchPorts(std::vector<uint32_t> ports);
void UpdateSwitchPorts(int bitWidth);

Time GetTime();
void SetTime(const Time& time);

uint32_t GetTimeSeconds();
uint32_t SetTimeSeconds();

uint32_t ConvertTimeToSeconds(const Time &time);
