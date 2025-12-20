#include "routines.h"

#include <Arduino.h>
#include <Preferences.h>

#define SWITCH_BIT_WIDTH 6

Preferences routinePreferences; // Max Key Length: 15

std::vector<SwitchRoutine> switchRoutines = {};

SwitchPorts switchPorts = { {}, {} };

void SetPerminantRoutine(int index, SwitchRoutine& routine) {
    std::string routineName = "routine-" + std::to_string(index);

    Serial.printf("SetPerminantRoutine: %s\n", routineName.c_str());

    routinePreferences.begin(routineName.c_str(), false);
    routinePreferences.putString("name", routine.name.c_str());
    routinePreferences.putInt("timeInterval", routine.timeInterval);

    routinePreferences.putInt("ppCount", routine.pumpPorts.size());
    routinePreferences.putInt("vpCount", routine.valvePorts.size());
    routinePreferences.putInt("trdCount", routine.tempRangeDurations.size());
    
    routinePreferences.putBytes("pumpPorts", &routine.pumpPorts[0], routine.pumpPorts.size() * sizeof(uint32_t));
    routinePreferences.putBytes("valvePorts", &routine.valvePorts[0], routine.valvePorts.size() * sizeof(uint32_t));

    routinePreferences.putBytes("tempRangeDur", &routine.tempRangeDurations[0], routine.tempRangeDurations.size() * sizeof(int)); // Int32: 4 Char
    
    //routinePreferences.putInt("newTime", routine.newTime);
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
    routine.timeInterval = routinePreferences.getInt("timeInterval", 0);
    
    int pumpPortsCount = routinePreferences.getInt("ppCount", 0);
    int valvePortsCount = routinePreferences.getInt("vpCount", 0);
    int tempRangeDurationsCount = routinePreferences.getInt("trdCount", 0);

    routine.pumpPorts.resize(pumpPortsCount);
    routine.valvePorts.resize(valvePortsCount);
    routine.tempRangeDurations.resize(tempRangeDurationsCount);

    routinePreferences.getBytes("pumpPorts", &routine.pumpPorts[0], pumpPortsCount * sizeof(uint32_t));
    routinePreferences.getBytes("valvePorts", &routine.valvePorts[0], valvePortsCount * sizeof(uint32_t));

    routinePreferences.getBytes("tempRangeDur", &routine.tempRangeDurations[0], tempRangeDurationsCount * sizeof(int)); // Int32: 4 Char
    
    //routine.newTime = routinePreferences.getInt("newTime", GetTimeNowSeconds());
    routine.newTime = GetTimeNowSeconds();

    routine.done = true;
    routine.timeDuration = 0;
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
void AddRoutine(SwitchRoutine routine, Time time) {
    routine.newTime = ConvertTimeToSeconds(time);
    routine.done = true;
    routine.timeDuration = 0;

    switchRoutines.push_back(routine);

    SetPerminantRoutine(switchRoutines.size() - 1, routine);

    routinePreferences.begin("routines", false);
    routinePreferences.putInt("count", switchRoutines.size());
    routinePreferences.end();

    Serial.printf("Added Routine: %d\n", switchRoutines.size() - 1);
}
void EditRoutine(int index, SwitchRoutine newRoutine, Time time, bool newTimeSet) {
    if (newTimeSet) {
        newRoutine.newTime = ConvertTimeToSeconds(time);
    } else {
        newRoutine.newTime = switchRoutines[index].newTime;
    }
    newRoutine.done = switchRoutines[index].done;
    newRoutine.timeDuration = switchRoutines[index].timeDuration;

    switchRoutines[index] = newRoutine;

    SetPerminantRoutine(index, newRoutine);

    Serial.printf("Edited Routine: %d\n", index);
}
void RemoveRoutine(int index) {
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

    Serial.printf("Removed Routine: %d\n", index);
}

void ResetToPerminantSwitchPorts() {
    routinePreferences.begin("switchPorts", false);

    int pumpsCount = routinePreferences.getInt("pumpsCount", 0);
    int valvesCount = routinePreferences.getInt("valvesCount", 0);

    
    switchPorts.pumps.resize(pumpsCount);
    switchPorts.valves.resize(valvesCount);
    
    routinePreferences.getBytes("pumps", &switchPorts.pumps[0], pumpsCount * sizeof(SwitchPort));
    routinePreferences.getBytes("valves", &switchPorts.valves[0], valvesCount * sizeof(SwitchPort));
    
    Serial.printf("pumpsCount: %d\n", switchPorts.pumps[0].index);
    Serial.printf("valvesCount: %d\n", switchPorts.valves.size());

    routinePreferences.end();
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

int GetCurrentTimeDuration(const SwitchRoutine& switchRoutine) {
    int temp = GetTemp();

    for (int i = 1; i < switchRoutine.tempRangeDurations.size(); i += 2) {
        if (temp < switchRoutine.tempRangeDurations[i]) {
            return switchRoutine.tempRangeDurations[i - 1];
        }
    }

    return switchRoutine.tempRangeDurations[switchRoutine.tempRangeDurations.size() - 1];
}

void UpdateSwitch() {
    for (int i = 0; i < switchRoutines.size(); i++) {
        if (GetTimeNowSeconds() >= switchRoutines[i].newTime && switchRoutines[i].done) {
            switchRoutines[i].timeDuration = GetCurrentTimeDuration(switchRoutines[i]);
            
            if (switchRoutines[i].timeDuration != 0) {
                EnableSwitchPorts(switchRoutines[i].pumpPorts);
                EnableSwitchPorts(switchRoutines[i].valvePorts);
                UpdateSwitchPorts(SWITCH_BIT_WIDTH);
            }
            
            switchRoutines[i].done = false;
        }
        if (GetTimeNowSeconds() - switchRoutines[i].newTime >= switchRoutines[i].timeDuration && !switchRoutines[i].done) {
            if (switchRoutines[i].timeDuration != 0) {
                DisableSwitchPorts(switchRoutines[i].pumpPorts);
                DisableSwitchPorts(switchRoutines[i].valvePorts);
                UpdateSwitchPorts(SWITCH_BIT_WIDTH);
            }
            
            switchRoutines[i].newTime += switchRoutines[i].timeInterval;
            
            switchRoutines[i].done = true;
        }
    }
}
