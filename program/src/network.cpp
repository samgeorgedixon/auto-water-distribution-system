#include "network.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "components.h"
#include "routines.h"

#include "html-includes/index_html.h"
#include "html-includes/routines_html.h"
#include "html-includes/add_routine_html.h"
#include "html-includes/edit_routine_html.h"
#include "html-includes/switch_ports_html.h"
#include "html-includes/time_html.h"
#include "html-includes/wifi_html.h"

String ssid = "RIS";
String password ="admin123";

IPAddress localIP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

Preferences networkPreferences; // Max Key Length: 15

void HandleRoot() {
    server.send(200, "text/html", FPSTR(index_html));
}
void HandleRoutines() {
    server.send(200, "text/html", FPSTR(routines_html));
}
void HandleAddRoutine() {
    server.send(200, "text/html", FPSTR(add_routine_html));
}
void HandleEditRoutine() {
    server.send(200, "text/html", FPSTR(edit_routine_html));
}
void HandleSwitchPorts() {
    server.send(200, "text/html", FPSTR(switch_ports_html));
}
void HandleTime() {
    server.send(200, "text/html", FPSTR(time_html));
}
void HandleWifi() {
    server.send(200, "text/html", FPSTR(wifi_html));
}
void Handle404() {
    server.send(404, "text/plain", "404 Not Found");
}

void HandleGetTemp() {
    String temp = String(GetTemp());
    server.send(200, "text/plain", temp);
}
void HandleGetTime() {
    Time time = GetTime();

    WiFiClient client = server.client();

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();

    DynamicJsonDocument doc(100);
    
    doc["sec"] = time.sec;
    doc["min"] = time.min;
    doc["hour"] = time.hour;
    doc["dayDate"] = time.dayDate;
    doc["month"] = time.month;
    doc["year"] = time.year;

    serializeJson(doc, client);

    client.stop();

    Serial.println("Got Time");
}
void HandleSetTime() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Invalid Set Time Request");
        return;
    }
    
    String body = server.arg("plain");
    DynamicJsonDocument doc(100);
    deserializeJson(doc, body);
    
    Time time = {};
    time.sec = doc["sec"].as<int>();
    time.min = doc["min"].as<int>();
    time.hour = doc["hour"].as<int>();
    time.dayDate = doc["dayDate"].as<int>();
    time.month = doc["month"].as<int>();
    time.year = doc["year"].as<int>();
    
    SetTime(time);

    server.send(200, "text/plain", "Set Time");
}
void HandleSetWifi() {
    if (server.hasArg("ssid")) {
        ssid = server.arg("ssid");

        networkPreferences.begin("wifi", false);
        networkPreferences.putString("ssid", ssid);
        networkPreferences.end();

        StopNetwork();
        delay(100);
        SetupNetwork();

        Serial.println("Set SSID: " + ssid);
        server.send(200, "text/plain", "Set SSID");
    } else if (server.hasArg("password")) {
        password = server.arg("password");

        networkPreferences.begin("wifi", false);
        networkPreferences.putString("password", password);
        networkPreferences.end();
        
        StopNetwork();
        delay(100);
        SetupNetwork();

        Serial.println("Set Password: " + password);
        server.send(200, "text/plain", "Set Password");
    } else {
        Serial.println("Invalid /set Request");
        server.send(400, "text/plain", "Invalid /set Request");
    }
}

void HandleAddRoutineAPI() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Invalid Add Routine Request");
        return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument doc(1500);
    deserializeJson(doc, body);

    SwitchRoutine routine;
    routine.newTime = 0;
    routine.done = true;
    routine.timeDuration = 0;

    routine.pumpPorts = {};
    routine.valvePorts = {};
    routine.staggerValves = false;
    routine.tempRangeDurations = {};
    
    routine.name = doc["name"].as<String>().c_str();
    routine.timeInterval = doc["timeInterval"].as<float>();
    routine.timeIntervalUnit = doc["timeIntervalUnit"].as<String>();

    JsonArray pumpPorts = doc["pumpPorts"].as<JsonArray>();
    for (uint32_t b : pumpPorts) routine.pumpPorts.push_back(b - 1);

    JsonArray valvePorts = doc["valvePorts"].as<JsonArray>();
    for (uint32_t b : valvePorts) routine.valvePorts.push_back(b - 1);

    routine.staggerValves = doc["staggerValves"].as<bool>();
    
    JsonArray temps = doc["tempRangeDurations"].as<JsonArray>();
    for (int t : temps) routine.tempRangeDurations.push_back(t);

    Time time;
    time.sec = doc["newTimeSec"].as<int>();
    time.min = doc["newTimeMin"].as<int>();
    time.hour = doc["newTimeHour"].as<int>();
    time.dayDate = doc["newTimeDayDate"].as<int>();
    time.month = doc["newTimeMonth"].as<int>();
    time.year = doc["newTimeYear"].as<int>();

    AddRoutine(routine, time);

    server.send(200, "text/plain", "Added Routine");
}
void HandleEditRoutineAPI() {
    if (!server.hasArg("index") || !server.hasArg("plain")) {
        server.send(400, "text/plain", "Invalid Edit Routine Request");
        return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument doc(1500);
    deserializeJson(doc, body);

    SwitchRoutine routine;
    routine.newTime = 0;
    routine.done = true;
    routine.timeDuration = 0;

    routine.pumpPorts = {};
    routine.valvePorts = {};
    routine.staggerValves = false;
    routine.tempRangeDurations = {};
    
    routine.name = doc["name"].as<String>().c_str();
    routine.timeInterval = doc["timeInterval"].as<float>();
    routine.timeIntervalUnit = doc["timeIntervalUnit"].as<String>();

    JsonArray pumpPorts = doc["pumpPorts"].as<JsonArray>();
    for (uint32_t b : pumpPorts) routine.pumpPorts.push_back(b - 1);

    JsonArray valvePorts = doc["valvePorts"].as<JsonArray>();
    for (uint32_t b : valvePorts) routine.valvePorts.push_back(b - 1);

    routine.staggerValves = doc["staggerValves"].as<bool>();
    
    JsonArray temps = doc["tempRangeDurations"].as<JsonArray>();
    for (int t : temps) routine.tempRangeDurations.push_back(t);
    
    Time time;
    time.sec = doc["newTimeSec"].as<int>();
    time.min = doc["newTimeMin"].as<int>();
    time.hour = doc["newTimeHour"].as<int>();
    time.dayDate = doc["newTimeDayDate"].as<int>();
    time.month = doc["newTimeMonth"].as<int>();
    time.year = doc["newTimeYear"].as<int>();
    
    bool newTimeSet = doc["newTimeSet"].as<bool>();
    
    EditRoutine(server.arg("index").toInt(), routine, time, newTimeSet);

    server.send(200, "text/plain", "Edited Routine");
}
void HandleRemoveRoutineAPI() {
    if (server.hasArg("index")) {
        int index = server.arg("index").toInt();

        RemoveRoutine(index);

        String response = "Removed Routine: " + String(index);
        Serial.println(response);
        server.send(200, "text/plain", response);
    } else {
        Serial.println("Invalid Remove Routine Request");
        server.send(400, "text/plain", "Invalid Remove Routine Request");
    }
}

void RoutineToJson(const SwitchRoutine &routine, DynamicJsonDocument& doc) {
    doc["name"] = routine.name.c_str();
    doc["timeInterval"] = routine.timeInterval;
    doc["timeIntervalUnit"] = routine.timeIntervalUnit;

    JsonArray pumpPortsJson = doc.createNestedArray("pumpPorts");
    for (int i = 0; i < routine.pumpPorts.size(); i++) {
        pumpPortsJson.add(routine.pumpPorts[i]);
    }
    JsonArray valvePortsJson = doc.createNestedArray("valvePorts");
    for (int i = 0; i < routine.valvePorts.size(); i++) {
        valvePortsJson.add(routine.valvePorts[i]);
    }

    doc["staggerValves"] = routine.staggerValves;

    JsonArray tempRangeDurationsJson = doc.createNestedArray("tempRangeDurations");
    for (int i = 0; i < routine.tempRangeDurations.size(); i++) {
        tempRangeDurationsJson.add(routine.tempRangeDurations[i]);
    }
}

void HandleGetRoutineAPI() {
    if (server.hasArg("index")) {
        int index = server.arg("index").toInt();
        const SwitchRoutine& routine = GetRoutine(index);

        WiFiClient client = server.client();

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
        
        DynamicJsonDocument doc(1500);
        RoutineToJson(routine, doc);
        serializeJson(doc, client);

        client.stop();

        Serial.printf("Got Routine: %d\n", index);
    } else {
        Serial.println("Invalid Get Routine Request");
        server.send(400, "text/plain", "Invalid Get Routine Request");
    }
}
void HandleGetAllRoutineAPI() {
    const std::vector<SwitchRoutine>& routines = GetRoutines();

    WiFiClient client = server.client();

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    
    client.print("[");

    for (int i = 0; i < routines.size(); i++) {
        if (i > 0) client.print(",");

        DynamicJsonDocument doc(1500);
        RoutineToJson(routines[i], doc);

        serializeJson(doc, client);
    }

    client.print("]");
    client.stop();

    Serial.println("Got Routines");
}

void SwitchPortToJson(const SwitchPort &switchPort, DynamicJsonDocument& doc) {
    doc["index"] = switchPort.index;
    doc["current"] = switchPort.current;
    doc["voltage"] = switchPort.voltage;
    doc["externalPower"] = (switchPort.externalPower ? true: false);
}

void HandleGetAllSwitchPortAPI() {
    const SwitchPorts& ports = GetSwitchPorts();

    WiFiClient client = server.client();

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    
    client.print("{");
    client.print("\"pumps\": [");

    for (int i = 0; i < ports.pumps.size(); i++) {
        if (i > 0) client.print(",");

        DynamicJsonDocument doc(128);
        SwitchPortToJson(ports.pumps[i], doc);

        serializeJson(doc, client);
    }

    client.print("],");
    client.print("\"valves\": [");

    for (int i = 0; i < ports.valves.size(); i++) {
        if (i > 0) client.print(",");

        DynamicJsonDocument doc(128);
        SwitchPortToJson(ports.valves[i], doc);

        serializeJson(doc, client);
    }

    client.print("]");
    client.print("}");
    client.stop();

    Serial.println("Got Switch Ports");
}
void HandleSaveAllSwitchPortAPI() {
    if (!server.hasArg("plain")) {
        server.send(400, "text/plain", "Invalid Save Switch Ports Request");
        return;
    }

    String body = server.arg("plain");
    DynamicJsonDocument doc(1500);
    deserializeJson(doc, body);

    SwitchPorts newPorts;
    newPorts.pumps.clear();
    newPorts.valves.clear();

    JsonArray pumps = doc["pumps"].as<JsonArray>();
    for (JsonObject p : pumps) {
        SwitchPort port;
        port.index = p["index"].as<int>();
        port.current = p["current"].as<float>();
        port.voltage = p["voltage"].as<float>();
        port.externalPower = p["externalPower"].as<bool>();

        newPorts.pumps.push_back(port);
    }

    JsonArray valves = doc["valves"].as<JsonArray>();
    for (JsonObject v : valves) {
        SwitchPort port;
        port.index = v["index"].as<int>();
        port.current = v["current"].as<float>();
        port.voltage = v["voltage"].as<float>();
        port.externalPower = v["externalPower"].as<bool>();

        newPorts.valves.push_back(port);
    }

    SetSwitchPorts(newPorts);

    Serial.println("Saved Switch Ports");
    server.send(200, "text/plain", "Saved Switch Ports");
}

void SetupServerHandles() {
    server.on("/", HandleRoot);
    server.on("/routines", HandleRoutines);
    server.on("/add-routine", HandleAddRoutine);
    server.on("/edit-routine", HandleEditRoutine);
    server.on("/switch-ports", HandleSwitchPorts);
    server.on("/time", HandleTime);
    server.on("/wifi", HandleWifi);
    
    server.on("/api/get-temp", HandleGetTemp);
    server.on("/api/get-time", HandleGetTime);
    server.on("/api/set-time", HandleSetTime);
    server.on("/api/set-wifi", HandleSetWifi);

    server.on("/api/routine/add", HandleAddRoutineAPI);
    server.on("/api/routine/edit", HandleEditRoutineAPI);
    server.on("/api/routine/remove", HandleRemoveRoutineAPI);

    server.on("/api/routine/get-all", HandleGetAllRoutineAPI);
    server.on("/api/routine/get", HandleGetRoutineAPI);

    server.on("/api/switch-port/get-all", HandleGetAllSwitchPortAPI);
    server.on("/api/switch-port/save-all", HandleSaveAllSwitchPortAPI);
    
    server.onNotFound(Handle404);
}

void SetupNetwork() {
    networkPreferences.begin("wifi", false);
    
    ssid = networkPreferences.getString("ssid", ssid);
    password = networkPreferences.getString("password", password);

    networkPreferences.end();

    WiFi.mode(WIFI_AP);

    delay(100);

    WiFi.softAPConfig(localIP, gateway, subnet);
    WiFi.softAP(ssid.c_str(), password.c_str());

    SetupServerHandles();
    server.begin();

    Serial.println("Network Setup");
}

void StopNetwork() {
    server.stop();

    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    esp_wifi_stop();
    esp_wifi_deinit();

    Serial.println("Network Stopped");
}

void UpdateNetwork() {
    server.handleClient();
}
