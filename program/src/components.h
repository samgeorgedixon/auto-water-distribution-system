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
void SetLEDColourRG(int r, int g);

void SetTime(const Time &time);

void EnableSwitchPorts(std::vector<uint32_t> ports);
void DisableSwitchPorts(std::vector<uint32_t> ports);
void UpdateSwitchPorts();

Time GetTimeNow();
void SetTime(const Time& time);

unsigned int GetTimeNowSeconds();
unsigned int ConvertTimeToSeconds(const Time &time);

