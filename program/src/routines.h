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
    int timeInterval; // Seconds

    std::vector<uint32_t> pumpPorts;
    std::vector<uint32_t> valvePorts;

    std::vector<int> tempRangeDurations; // Duration,  < Temp <=, Duration,  < Temp <=, Duration
    
    unsigned int newTime;

    bool done;
    int timeDuration;
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

void UpdateSwitch();
