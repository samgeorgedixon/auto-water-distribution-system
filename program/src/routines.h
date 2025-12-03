#pragma once
#include <string>
#include <vector>
#include <stdint.h>

#include <Arduino.h>

#include "components.h"

struct SwitchRoutine {
    String name;
    int timeInterval; // Seconds

    std::vector<uint8_t> portStates; // Bool Byte
    std::vector<uint8_t> returnPortStates; // Bool Byte

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

void UpdateSwitch();
