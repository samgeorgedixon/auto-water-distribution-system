#pragma once
#include <string>
#include <vector>
#include <stdint.h>

#include <Arduino.h>

#include "components.h"

struct __attribute__((packed)) SwitchPort {
    int32_t index;
    float current;
    float voltage;
    int32_t externalPower; // Bool
};
struct SwitchPorts {
    std::vector<SwitchPort> pumps;
    std::vector<SwitchPort> valves;
};

struct SwitchRoutine {
    String name;
    float timeInterval; // Seconds
    String timeIntervalUnit;

    std::vector<uint32_t> pumpPorts;
    std::vector<uint32_t> valvePorts;

    bool staggerValves;

    std::vector<int> tempRangeDurations;
    
    uint32_t newTime;

    int currentStaggerValveIndex;
    bool done;
    uint32_t timeDuration;
};

void ResetToAllPerminantRoutines();
const std::vector<SwitchRoutine> &GetRoutines();
const SwitchRoutine& GetRoutine(int index);

void AddRoutine(SwitchRoutine routine, Time time);
void EditRoutine(int index, SwitchRoutine newRoutine, Time time, bool newTimeSet);
void RemoveRoutine(int index);

void ResetToPerminantSwitchPorts();
const SwitchPorts& GetSwitchPorts();
void SetSwitchPorts(SwitchPorts switchPorts);

void UpdateSwitch(bool wifiOn);
