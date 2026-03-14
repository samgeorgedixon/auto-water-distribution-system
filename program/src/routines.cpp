#include "routines.h"

#include <Arduino.h>
#include <Preferences.h>

#include "core.h"

Preferences routinePreferences; // Max Key Length: 15

std::vector<SwitchRoutine> switchRoutines = {};

SwitchPorts switchPorts = { {}, {} };

int GetCurrentTimeDuration(const SwitchRoutine& switchRoutine) {
    if (switchRoutine.tempRangeDurations.size() == 0) {
        return 0;
    }
    int temp = GetTemp();

    for (int i = 0; i < switchRoutine.tempRangeDurations.size(); i += 2) {
        if (temp < switchRoutine.tempRangeDurations[i]) {
            return switchRoutine.tempRangeDurations[i + 1];
        }
    }
    return switchRoutine.tempRangeDurations[switchRoutine.tempRangeDurations.size() - 1];
}

void SetPerminantRoutine(int index, SwitchRoutine& routine) {
    std::string routineName = "routine-" + std::to_string(index);

    LOGf("SetPerminantRoutine: %s\n", routineName.c_str());

    routinePreferences.begin(routineName.c_str(), false);
    routinePreferences.putString("name", routine.name.c_str());
    routinePreferences.putFloat("timeInterval", routine.timeInterval);
    routinePreferences.putString("tiUnit", routine.timeIntervalUnit);
    routinePreferences.putBool("staggerValves", routine.staggerValves);

    routinePreferences.putInt("ppCount", routine.pumpPorts.size());
    routinePreferences.putInt("vpCount", routine.valvePorts.size());
    routinePreferences.putInt("trdCount", routine.tempRangeDurations.size());
    
    routinePreferences.putBytes("pumpPorts", &routine.pumpPorts[0], routine.pumpPorts.size() * sizeof(uint32_t));
    routinePreferences.putBytes("valvePorts", &routine.valvePorts[0], routine.valvePorts.size() * sizeof(uint32_t));

    routinePreferences.putBytes("tempRangeDur", &routine.tempRangeDurations[0], routine.tempRangeDurations.size() * sizeof(int)); // Int32: 4 Char
    
    routinePreferences.putInt("newTime", routine.newTime);

    routinePreferences.end();
}
void RemovePerminantRoutine(int index) {
    std::string routineName = "routine-" + std::to_string(index);
    routinePreferences.begin(routineName.c_str(), false);

    routinePreferences.clear();

    routinePreferences.end();
}
SwitchRoutine GetPerminantRoutine(int index) {
    SwitchRoutine routine = {};

    std::string routineName = "routine-" + std::to_string(index);

    routinePreferences.begin(routineName.c_str(), false);
    routine.name = routinePreferences.getString("name", "");
    routine.timeInterval = routinePreferences.getFloat("timeInterval", 0);
    routine.timeIntervalUnit = routinePreferences.getString("tiUnit", "sec");
    routine.staggerValves = routinePreferences.getBool("staggerValves", false);
    
    int pumpPortsCount = routinePreferences.getInt("ppCount", 0);
    int valvePortsCount = routinePreferences.getInt("vpCount", 0);
    int tempRangeDurationsCount = routinePreferences.getInt("trdCount", 0);

    routine.pumpPorts.resize(pumpPortsCount);
    routine.valvePorts.resize(valvePortsCount);
    routine.tempRangeDurations.resize(tempRangeDurationsCount);

    routinePreferences.getBytes("pumpPorts", &routine.pumpPorts[0], pumpPortsCount * sizeof(uint32_t));
    routinePreferences.getBytes("valvePorts", &routine.valvePorts[0], valvePortsCount * sizeof(uint32_t));

    routinePreferences.getBytes("tempRangeDur", &routine.tempRangeDurations[0], tempRangeDurationsCount * sizeof(int)); // Int32: 4 Char
    
    // Work Out Correct routine.newTime
    routine.newTime = routinePreferences.getInt("newTime", GetTimeSeconds());
    routine.timeDuration = GetCurrentTimeDuration(routine);

    if (routine.timeDuration <= 0 && routine.timeInterval <= 0) {
        routine.newTime = GetTimeSeconds();
    }

    if (routine.newTime < GetTimeSeconds()) {
        uint32_t cycle = routine.timeDuration + routine.timeInterval;
        uint32_t divisions = (GetTimeSeconds() - routine.newTime) / cycle;

        routine.newTime += cycle * (divisions + 1);
    }

    routinePreferences.putInt("newTime", routine.newTime);

    routine.done = true;
    routinePreferences.end();

    return routine;
}
void ResetToAllPerminantRoutines() {
    routinePreferences.begin("routines", false);
    int count = routinePreferences.getInt("count", 0);
    routinePreferences.end();

    switchRoutines.clear();
    for (int i = 0; i < count; i++) {
        SwitchRoutine routine = GetPerminantRoutine(i);
        switchRoutines.push_back(routine);
    }
}

const std::vector<SwitchRoutine>& GetRoutines() {
    return switchRoutines;
}
const SwitchRoutine& GetRoutine(int index) {
    if (index >= switchRoutines.size()) {
        return {};
    }

    return switchRoutines[index];
}

void DisableAllSwitchPorts() {
    if (switchPorts.pumps.size() == 0 && switchPorts.valves.size() == 0) {
        return;
    }

    uint32_t highestIndex = 0;

    for (int i = 0; i < switchPorts.pumps.size(); i++) {
        if (switchPorts.pumps[i].index > highestIndex) {
            highestIndex = switchPorts.pumps[i].index;
        }
    }
    for (int i = 0; i < switchPorts.valves.size(); i++) {
        if (switchPorts.valves[i].index > highestIndex) {
            highestIndex = switchPorts.valves[i].index;
        }
    }

    DisableSwitchPorts({ highestIndex });
}

void ResetToPerminantSwitchPorts() {
    routinePreferences.begin("switchPorts", false);

    int pumpsCount = routinePreferences.getInt("pumpsCount", 0);
    int valvesCount = routinePreferences.getInt("valvesCount", 0);

    switchPorts.pumps.resize(pumpsCount);
    switchPorts.valves.resize(valvesCount);
    
    routinePreferences.getBytes("pumps", &switchPorts.pumps[0], pumpsCount * sizeof(SwitchPort));
    routinePreferences.getBytes("valves", &switchPorts.valves[0], valvesCount * sizeof(SwitchPort));
    
    routinePreferences.end();
    
    DisableAllSwitchPorts();
}

const SwitchPorts& GetSwitchPorts() {
    return switchPorts;
}
void SetSwitchPorts(SwitchPorts ports) {
    switchPorts = ports;

    routinePreferences.begin("switchPorts", false);

    routinePreferences.putInt("pumpsCount", switchPorts.pumps.size());
    routinePreferences.putInt("valvesCount", switchPorts.valves.size());

    routinePreferences.putBytes("pumps", &switchPorts.pumps[0], switchPorts.pumps.size() * sizeof(SwitchPort));
    routinePreferences.putBytes("valves", &switchPorts.valves[0], switchPorts.valves.size() * sizeof(SwitchPort));
    
    routinePreferences.end();
}

void StartRoutine(SwitchRoutine& switchRoutine) {
    switchRoutine.timeDuration = GetCurrentTimeDuration(switchRoutine);
    
    if (switchRoutine.timeDuration != 0) {
        EnableSwitchPorts(switchRoutine.pumpPorts);

        if (switchRoutine.staggerValves) {
            switchRoutine.currentStaggerValveIndex = 0;
            EnableSwitchPorts({ switchRoutine.valvePorts[switchRoutine.currentStaggerValveIndex] });
        }
        else {
            EnableSwitchPorts(switchRoutine.valvePorts);
        }
        UpdateSwitchPorts(SWITCH_BIT_WIDTH);
    }
    switchRoutine.done = false;
}
void StopRoutine(SwitchRoutine& switchRoutine) {
    switchRoutine.newTime = int(switchRoutine.timeInterval) + GetTimeSeconds();

    if (switchRoutine.timeDuration != 0) {
        if (switchRoutine.staggerValves && switchRoutine.currentStaggerValveIndex < switchRoutine.valvePorts.size() - 1) {
            DisableSwitchPorts({ switchRoutine.valvePorts[switchRoutine.currentStaggerValveIndex] });

            switchRoutine.currentStaggerValveIndex++;

            EnableSwitchPorts({ switchRoutine.valvePorts[switchRoutine.currentStaggerValveIndex] });

            UpdateSwitchPorts(SWITCH_BIT_WIDTH);

            return;
        }
        else {
            DisableSwitchPorts(switchRoutine.pumpPorts);
            DisableSwitchPorts(switchRoutine.valvePorts);
            
            UpdateSwitchPorts(SWITCH_BIT_WIDTH);
        }
    }
    
    switchRoutine.done = true;
}

void UpdateSwitch(bool wifiOn) {
    uint32_t now = GetTimeSeconds();
    uint32_t smallestNewTime = 0xffffffff; // Seconds

    for (int i = 0; i < switchRoutines.size(); i++) {
        if (now >= switchRoutines[i].newTime && switchRoutines[i].done) {
            StartRoutine(switchRoutines[i]);
        }
        if (now - switchRoutines[i].newTime >= switchRoutines[i].timeDuration && !switchRoutines[i].done) {
            StopRoutine(switchRoutines[i]);
        }

        if (switchRoutines[i].newTime < smallestNewTime) {
            smallestNewTime = switchRoutines[i].newTime;
        }
    }

    if (smallestNewTime == 0xffffffff || wifiOn) {
        return;
    }

    uint32_t smallestTimeGap = smallestNewTime - now;
    if (smallestTimeGap > 2628000) { // > ~1 Month
        LightSleep(smallestTimeGap - 1800); // Wakeup 30mins Before.
    }
    else if (smallestTimeGap > 86400) { // > 1 Day
        LightSleep(smallestTimeGap - 120); // Wakeup 2mins Before.
    }
    else if (smallestTimeGap > 3600) { // > 1 Hour
        LightSleep(smallestTimeGap - 10); // Wakeup 10secs Before.
    }
    else if (smallestTimeGap > 4) { // > 1 Sec
        LightSleep(smallestTimeGap - 2); // Wakeup 1sec Before.
    }
}

void AddRoutine(SwitchRoutine routine, Time time) {
    routine.newTime = ConvertTimeToSeconds(time);
    routine.done = true;
    routine.timeDuration = 0;

    switchRoutines.push_back(routine);

    SetPerminantRoutine(switchRoutines.size() - 1, routine);

    routinePreferences.begin("routines", false);
    routinePreferences.putInt("count", switchRoutines.size());
    routinePreferences.end();

    LOGf("Added Routine: %d\n", switchRoutines.size() - 1);
}
void EditRoutine(int index, SwitchRoutine newRoutine, Time time, bool newTimeSet) {
    if (newTimeSet) {
        StopRoutine(switchRoutines[index]);

        newRoutine.newTime = ConvertTimeToSeconds(time);
    } else {
        newRoutine.newTime = switchRoutines[index].newTime;
    }
    
    newRoutine.done = switchRoutines[index].done;
    
    if (!switchRoutines[index].done) { // If Running
        newRoutine.timeDuration = switchRoutines[index].timeDuration;
    }
    else {
        newRoutine.timeDuration = 0;
    }

    switchRoutines[index] = newRoutine;

    SetPerminantRoutine(index, newRoutine);

    LOGf("Edited Routine: %d\n", index);
}
void RemoveRoutine(int index) {
    if (!switchRoutines[index].done) { // If Running
        StopRoutine(switchRoutines[index]);
    }

    for (int i = index; i < switchRoutines.size(); i++) {
        RemovePerminantRoutine(i);

        if (i == index) {
            continue;
        }

        SetPerminantRoutine(i - 1, switchRoutines[i]);
    }

    switchRoutines.erase(switchRoutines.begin() + index);

    routinePreferences.begin("routines", false);
    routinePreferences.putInt("count", switchRoutines.size());
    routinePreferences.end();

    LOGf("Removed Routine: %d\n", index);
}
