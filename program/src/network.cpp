#include "network.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <Preferences.h>
#include <ArduinoJson.h>

#include "components.h"
#include "routines.h"

String ssid = "RIS";
String password ="admin123";

IPAddress localIP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

Preferences networkPreferences; // Max Key Length: 15

void HandleRoot() {
    String response = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Auto Water Distribution System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Regulated Irrigation System</h1>    
        
        <a href="time"><button >Set Time</button></a>
        <a href="wifi"><button >Set Wifi Credentials</button></a>
        <a href="switch-ports"><button >Switch Ports</button></a>
        <a href="routines"><button >Routines</button></a>

        <hr>

        <p id="time">Time: ...</p>
        <p id="date">Date: ...</p>

        <p id="temp">Temperature: ...</p>

        <script>
            async function GetTemperature() {
                const response = await fetch(`/api/get-temp`);
                const temp = await response.json();

                document.getElementById('temp').textContent = `Temperature: ${temp}°C`;
            }
            async function GetTime() {
                const response = await fetch(`/api/get-time`);
                const time = await response.json();

                document.getElementById('time').textContent = `Time: ${time.hour}:${time.min}:${time.sec}`;
                document.getElementById('date').textContent = `Date: ${time.dayDate}/${time.month}/${time.year}`;
            }

            document.addEventListener("DOMContentLoaded", async () => {
                await GetTemperature();
                await GetTime();
            });
        </script>
    </body>
    </html>
    )";
    server.send(200, "text/html", response);
}
void HandleRoutines() {
    String response = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Routines - Regulated Irrigation System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Routines - Regulated Irrigation System</h1>

        <a href="/"><button>RIS</button></a>

        <hr>

        <a href="/add-routine"><button>Add Routine</button></a><br>
        
        <ul id="routineList"></ul>

        <script>
            function DisplayRoutines(routines) {
                const list = document.getElementById("routineList");
                list.innerHTML = "";

                routines.forEach((routine, index) => {
                    const li = document.createElement("li");

                    const text = document.createElement("span");
                    text.textContent = `${index}: ${routine.name}`;

                    const editBtn = document.createElement("button");
                    editBtn.textContent = "Edit";
                    editBtn.addEventListener("click", () => {
                        window.location.href = `/edit-routine?index=${index}`;
                    });

                    const removeBtn = document.createElement("button");
                    removeBtn.textContent = "Remove";
                    removeBtn.addEventListener("click", async () => {
                        await fetch(`/api/routine/remove?index=${index}`);

                        LoadRoutines();
                    });

                    li.appendChild(text);
                    li.appendChild(editBtn);
                    li.appendChild(removeBtn);

                    list.appendChild(li);
                });
            }
            
            async function LoadRoutines() {
                const response = await fetch("/api/routine/get-all");
                const routines = await response.json();

                DisplayRoutines(routines);
            }

            window.addEventListener("DOMContentLoaded", LoadRoutines);
        </script>
    </body>
    </html>
    )";
    server.send(200, "text/html", response);
}
void HandleAddRoutine() {
    String response = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Add Routine - Regulated Irrigation System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Add Routine - Regulated Irrigation System</h1>

        <a href="/"><button>RIS</button></a>
        <a href="/routines"><button>View Routines</button></a>

        <hr>

        <form id="routineForm">
            <label for="name">Name: </label>
            <input type="text" id="name" name="name" required><br><br>

            <label for="timeInterval">Time Interval:</label>
            <input type="number" id="timeInterval" name="timeInterval" required>
            <select id="timeIntervalUnit" name="timeIntervalUnit">
                <option value="seconds">Seconds</option>
                <option value="minutes">Minutes</option>
                <option value="hours">Hours</option>
                <option value="days">Days</option>
            </select>

            <h3>Switch Ports</h3>
            <h4>Pumps</h4>
            <button type="button" id="addPumpPort">Add Port</button>
            <button type="button" id="removePumpPort">Remove Port</button>

            <ul id="pumpPortsContainer"></ul>

            <h4>Valves</h4>

            <button type="button" id="addValvePort">Add Port</button>
            <button type="button" id="removeValvePort">Remove Port</button>

            <ul id="valvePortsContainer"></ul>
            <br>

            <h3>Durations (s) between Temperatures (°C)</h3>
            <div id="tempDurationsContainer">
                <label style="margin-left: 120px;"><= , Duration: </label>
                <input type="number" value=0 style="width: 50px;">
                <br>
            </div>
            <button type="button" id="addTempDuration">Add Temp Duration</button>
            <button type="button" id="removeTempDuration">Remove Temp Duration</button>
            <br>

            <h3>Start Time:</h3>
            <div id="specificTimeContainer">
                <label>Second: <input type="number" id="timeSec" min="0" max="59" value="0"></label><br>
                <label>Minute: <input type="number" id="timeMin" min="0" max="59" value="0"></label><br>
                <label>Hour:   <input type="number" id="timeHour" min="0" max="23" value="0"></label><br>
                <label>Date:   <input type="number" id="timeDayDate" min="1" max="31" value="1"></label><br>
                <label>Month:  <input type="number" id="timeMonth" min="1" max="12" value="1"></label><br>
                <label>Year:   <input type="number" id="timeYear" min="1970" max="2100" value="2026"></label><br>
            </div>

            <br>
            <button type="submit">Add Routine</button>
        </form>

        <script>
            const unitToSeconds = {
                seconds: 1,
                minutes: 60,
                hours: 3600,
                days: 86400
            };
            
            function CreateCheckbox(container, isChecked) {
                const checkbox = document.createElement("input");
                checkbox.type = "checkbox";
                checkbox.checked = isChecked;
                container.appendChild(checkbox);
            }

            function CreateTextInput(container) {
                const input = document.createElement("input");
                input.type = "number";
                input.style.width = "50px";
                container.appendChild(input);
            }

            function CreateLabel(container, text) {
                const label = document.createElement("label");
                label.innerText = text;
                container.appendChild(label);
            }

            function CreatePortSelection(jsonList, container) {
                const select = document.createElement("select");

                const placeholder = document.createElement("option");
                placeholder.value = "";
                placeholder.textContent = "-- Select Port --";
                placeholder.selected = true;
                placeholder.disabled = true;
                select.appendChild(placeholder);

                jsonList.forEach(item => {
                    const option = document.createElement("option");
                    option.value = item.index;
                    option.textContent = `${item.index}: ${item.current}A, ${item.voltage}V`;
                    select.appendChild(option);
                });

                container.appendChild(select);
            }

            const pumpPortsContainer = document.getElementById("pumpPortsContainer");
            const valvePortsContainer = document.getElementById("valvePortsContainer");

            let switchPorts = null;

            document.getElementById("addPumpPort").addEventListener("click", () => {
                const li = document.createElement("li");

                CreatePortSelection(switchPorts.pumps, li);

                pumpPortsContainer.appendChild(li);
            });
            document.getElementById("removePumpPort").addEventListener("click", () => {
                const last = pumpPortsContainer.lastElementChild;
                if (last) last.remove();
            });
            document.getElementById("addValvePort").addEventListener("click", () => {
                const li = document.createElement("li");

                CreatePortSelection(switchPorts.valves, li);

                valvePortsContainer.appendChild(li);
            });
            document.getElementById("removeValvePort").addEventListener("click", () => {
                const last = valvePortsContainer.lastElementChild;
                if (last) last.remove();
            });

            document.getElementById("addTempDuration").addEventListener("click", () => {
                const tempDurationsContainer = document.getElementById("tempDurationsContainer");

                CreateLabel(tempDurationsContainer, "< Temp: ");
                CreateTextInput(tempDurationsContainer);

                CreateLabel(tempDurationsContainer, " <= , Duration: ");
                CreateTextInput(tempDurationsContainer);
                
                tempDurationsContainer.appendChild(document.createElement("br"));
            });
            document.getElementById("removeTempDuration").addEventListener("click", () => {
                const tempDurationsContainer = document.getElementById("tempDurationsContainer");

                for (let i = 0; i < 5; i++) {
                    const last = tempDurationsContainer.lastElementChild;
                    if (last && tempDurationsContainer.children.length > 3) last.remove();
                    else break;
                }
            });

            document.getElementById("routineForm").addEventListener("submit", async (e) => {
                e.preventDefault();

                const name = document.getElementById("name").value;
                
                const timeValue = parseFloat(document.getElementById("timeInterval").value);
                const timeIntervalUnit = document.getElementById("timeIntervalUnit").value;
                const timeInterval = timeValue * (unitToSeconds[timeIntervalUnit] || 1);

                // Ports
                const pumpSelects = pumpPortsContainer.querySelectorAll("select");
                const pumpPorts = Array.from(pumpSelects).map(s => s.value).filter(v => v !== "");

                const valveSelects = valvePortsContainer.querySelectorAll("select");
                const valvePorts = Array.from(valveSelects).map(s => s.value).filter(v => v !== "");

                // Temp Range Durations
                const tempContainer = document.getElementById("tempDurationsContainer");
                const tempRangeDurations = Array.from(tempContainer.querySelectorAll("input[type=number]")).map(inp => parseInt(inp.value) || 0);

                // New time
                const time = {
                    sec: parseInt(document.getElementById("timeSec").value) || 0,
                    min: parseInt(document.getElementById("timeMin").value) || 0,
                    hour: parseInt(document.getElementById("timeHour").value) || 0,
                    dayDate: parseInt(document.getElementById("timeDayDate").value) || 1,
                    month: parseInt(document.getElementById("timeMonth").value) || 1,
                    year: parseInt(document.getElementById("timeYear").value) || 2026
                };

                const routineJson = {
                    name,
                    timeInterval,
                    pumpPorts,
                    valvePorts,
                    tempRangeDurations,
                    newTimeSec: time.sec,
                    newTimeMin: time.min,
                    newTimeHour: time.hour,
                    newTimeDayDate: time.dayDate,
                    newTimeMonth: time.month,
                    newTimeYear: time.year
                };

                const response = await fetch("/api/routine/add", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify(routineJson)
                });

                window.location.href = "/routines";
            });

            async function LoadSwitchPorts() {
                const response = await fetch("/api/switch-port/get-all");
                switchPorts = await response.json();
            }

            document.addEventListener("DOMContentLoaded", async () => {
                await LoadSwitchPorts();
            });
        </script>
    </body>
    </html>
    )";
    server.send(200, "text/html", response);
}
void HandleEditRoutine() {
    String response = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Edit Routine - Regulated Irrigation System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Edit Routine - Regulated Irrigation System</h1>

        <a href="/"><button>RIS</button></a>
        <a href="/routines"><button>View Routines</button></a>

        <hr>

        <form id="routineForm">
            <label for="name">Name: </label>
            <input type="text" id="name" name="name" required><br><br>

            <label for="timeInterval">Time Interval:</label>
            <input type="number" id="timeInterval" name="timeInterval" required>
            <select id="timeIntervalUnit" name="timeIntervalUnit">
                <option value="seconds">Seconds</option>
                <option value="minutes">Minutes</option>
                <option value="hours">Hours</option>
                <option value="days">Days</option>
            </select>

            <h3>Switch Ports</h3>
            <h4>Pumps</h4>
            <button type="button" id="addPumpPort">Add Port</button>
            <button type="button" id="removePumpPort">Remove Port</button>

            <ul id="pumpPortsContainer"></ul>

            <h4>Valves</h4>

            <button type="button" id="addValvePort">Add Port</button>
            <button type="button" id="removeValvePort">Remove Port</button>

            <ul id="valvePortsContainer"></ul>
            <br>

            <h3>Durations (s) between Temperatures (°C)</h3>
            <div id="tempDurationsContainer">
                <label style="margin-left: 120px;"><= , Duration: </label>
                <input type="number" value=0 style="width: 50px;"></input>
                <br>
            </div>
            <button type="button" id="addTempDuration">Add Temp Duration</button>
            <button type="button" id="removeTempDuration">Remove Temp Duration</button>
            <br>

            <h3>Start Time:</h3>
            <label for="setNewTime">Set New Start Time:</label>
            <input type="checkbox" id="setNewTime" name="setNewTime"/>
            <div id="newTimeContainer">
                <label>Second: <input type="number" id="timeSec" min="0" max="59" value="0"></label><br>
                <label>Minute: <input type="number" id="timeMin" min="0" max="59" value="0"></label><br>
                <label>Hour:   <input type="number" id="timeHour" min="0" max="23" value="0"></label><br>
                <label>Date:   <input type="number" id="timeDayDate" min="1" max="31" value="1"></label><br>
                <label>Month:  <input type="number" id="timeMonth" min="1" max="12" value="1"></label><br>
                <label>Year:   <input type="number" id="timeYear" min="1970" max="2100" value="2026"></label><br>
            </div>

            <br>
            <button type="submit">Edit Routine</button>
            <button type="button" id="restore">Restore Originals</button>
        </form>

        <script>
            const setNewTime = document.getElementById("setNewTime");
            const newTimeContainer = document.getElementById("newTimeContainer");
            newTimeContainer.style.display = setNewTime.checked ? "block" : "none";

            setNewTime.addEventListener("change", () => {
                if (setNewTime.checked) {
                    newTimeContainer.style.display = "block";
                } else {
                    newTimeContainer.style.display = "none";
                }
            });

            const unitToSeconds = {
                seconds: 1,
                minutes: 60,
                hours: 3600,
                days: 86400
            };
            
            function CreateCheckbox(container, isChecked) {
                const checkbox = document.createElement("input");
                checkbox.type = "checkbox";
                checkbox.checked = isChecked;
                container.appendChild(checkbox);
            }

            function CreateTextInput(container, text) {
                const input = document.createElement("input");
                input.type = "number";
                input.value = text;
                input.style.width = "50px";
                container.appendChild(input);
            }

            function CreateLabel(container, text) {
                const label = document.createElement("label");
                label.innerText = text;
                container.appendChild(label);
            }

            function CreatePortSelection(jsonList, container, valueSelected) {
                const select = document.createElement("select");

                const placeholder = document.createElement("option");
                placeholder.value = "";
                placeholder.textContent = "-- Select Port --";
                placeholder.selected = !valueSelected;
                placeholder.disabled = true;
                select.appendChild(placeholder);

                jsonList.forEach(item => {
                    const option = document.createElement("option");
                    option.value = item.index;
                    option.textContent = `${item.index}: ${item.current}A, ${item.voltage}V`;
                    select.appendChild(option);
                });

                if (valueSelected != null) { 
                    select.value = valueSelected.toString();
                }

                container.appendChild(select);
            }

            const pumpPortsContainer = document.getElementById("pumpPortsContainer");
            const valvePortsContainer = document.getElementById("valvePortsContainer");

            let switchPorts = null;

            document.getElementById("addPumpPort").addEventListener("click", () => {
                const li = document.createElement("li");

                CreatePortSelection(switchPorts.pumps, li, "");

                pumpPortsContainer.appendChild(li);
            });
            document.getElementById("removePumpPort").addEventListener("click", () => {
                const last = pumpPortsContainer.lastElementChild;
                if (last) last.remove();
            });
            document.getElementById("addValvePort").addEventListener("click", () => {
                const li = document.createElement("li");

                CreatePortSelection(switchPorts.valves, li, "");

                valvePortsContainer.appendChild(li);
            });
            document.getElementById("removeValvePort").addEventListener("click", () => {
                const last = valvePortsContainer.lastElementChild;
                if (last) last.remove();
            });

            document.getElementById("addTempDuration").addEventListener("click", () => {
                const tempDurationsContainer = document.getElementById("tempDurationsContainer");

                CreateLabel(tempDurationsContainer, "< Temp: ");
                CreateTextInput(tempDurationsContainer, "");

                CreateLabel(tempDurationsContainer, " <= , Duration: ");
                CreateTextInput(tempDurationsContainer, "");
                
                tempDurationsContainer.appendChild(document.createElement("br"));
            });
            document.getElementById("removeTempDuration").addEventListener("click", () => {
                const tempDurationsContainer = document.getElementById("tempDurationsContainer");

                for (let i = 0; i < 5; i++) {
                    const last = tempDurationsContainer.lastElementChild;
                    if (last && tempDurationsContainer.children.length > 3) last.remove();
                    else break;
                }
            });

            async function RestoreOriginals() {
                let params = new URLSearchParams(document.location.search);
                let index = params.get("index");

                const response = await fetch(`/api/routine/get?index=${index}`);
                const routine = await response.json();

                // Basic
                document.getElementById("name").value = routine.name;

                document.getElementById("timeInterval").value = routine.timeInterval;
                document.getElementById("timeIntervalUnit").value = "seconds";

                // Ports
                pumpPortsContainer.innerHTML = "";
                valvePortsContainer.innerHTML = "";

                for (let i = 0; i < routine.pumpPorts.length; i++) {
                    const li = document.createElement("li");

                    CreatePortSelection(switchPorts.pumps, li, routine.pumpPorts[i] + 1);

                    pumpPortsContainer.appendChild(li);
                }
                for (let i = 0; i < routine.valvePorts.length; i++) {
                    const li = document.createElement("li");

                    CreatePortSelection(switchPorts.valves, li, routine.valvePorts[i] + 1);

                    valvePortsContainer.appendChild(li);
                }

                // Temp Durations
                const tempContainer = document.getElementById("tempDurationsContainer");
                tempContainer.innerHTML = `<label style="margin-left: 120px;"><= , Duration: </label><input type="number" value=${routine.tempRangeDurations[0]} style="width: 50px;"></input><br>`;
            
                for (let i = 1; i < routine.tempRangeDurations.length; i += 2) {
                    CreateLabel(tempContainer, "< Temp: ");
                    CreateTextInput(tempContainer, routine.tempRangeDurations[i]);
            
                    CreateLabel(tempContainer, " <= , Duration: ");
                    CreateTextInput(tempContainer, routine.tempRangeDurations[i + 1]);
            
                    tempContainer.appendChild(document.createElement("br"));
                }
            }

            document.getElementById("restore").addEventListener("click", async () => {
                await RestoreOriginals();
            });

            document.getElementById("routineForm").addEventListener("submit", async (e) => {
                e.preventDefault();

                const name = document.getElementById("name").value;
                
                const timeValue = parseFloat(document.getElementById("timeInterval").value);
                const timeIntervalUnit = document.getElementById("timeIntervalUnit").value;
                const timeInterval = timeValue * (unitToSeconds[timeIntervalUnit] || 1);

                // Ports
                const pumpSelects = pumpPortsContainer.querySelectorAll("select");
                const pumpPorts = Array.from(pumpSelects).map(s => s.value).filter(v => v !== "");

                const valveSelects = valvePortsContainer.querySelectorAll("select");
                const valvePorts = Array.from(valveSelects).map(s => s.value).filter(v => v !== "");

                // Temp Range Durations
                const tempContainer = document.getElementById("tempDurationsContainer");
                const tempRangeDurations = Array.from(tempContainer.querySelectorAll("input[type=number]")).map(inp => parseInt(inp.value) || 0);

                // New time
                const time = {
                    sec: parseInt(document.getElementById("timeSec").value) || 0,
                    min: parseInt(document.getElementById("timeMin").value) || 0,
                    hour: parseInt(document.getElementById("timeHour").value) || 0,
                    dayDate: parseInt(document.getElementById("timeDayDate").value) || 1,
                    month: parseInt(document.getElementById("timeMonth").value) || 1,
                    year: parseInt(document.getElementById("timeYear").value) || 2026
                };

                const routineJson = {
                    name,
                    timeInterval,
                    newTimeSet: setNewTime.checked,
                    pumpPorts,
                    valvePorts,
                    tempRangeDurations,
                    newTimeSec: time.sec,
                    newTimeMin: time.min,
                    newTimeHour: time.hour,
                    newTimeDayDate: time.dayDate,
                    newTimeMonth: time.month,
                    newTimeYear: time.year
                };

                let params = new URLSearchParams(document.location.search);
                let index = params.get("index");

                const response = await fetch(`/api/routine/edit?index=${index}`, {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify(routineJson)
                });

                window.location.href = "/routines";
            });

            async function LoadSwitchPorts() {
                const response = await fetch("/api/switch-port/get-all");
                switchPorts = await response.json();
            }

            document.addEventListener("DOMContentLoaded", async () => {
                await LoadSwitchPorts();
                await RestoreOriginals();
            });
        </script>
    </body>
    </html>
    )";
    server.send(200, "text/html", response);
}
void HandleSwitchPorts() {
    String response = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Switch Ports - Regulated Irrigation System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Switch Ports - Regulated Irrigation System</h1>

        <a href="/"><button>RIS</button></a>

        <hr>

        <h3>Pump Ports: </h3>
        <button type="button" id="addPumpPort">Add</button>
        <button type="button" id="removePumpPort">Remove</button>
        
        <ul id="pumpList"></ul>

        <h3>Valve Ports: </h3>
        <button type="button" id="addValvePort">Add</button>
        <button type="button" id="removeValvePort">Remove</button>
        
        <ul id="valveList"></ul>

        <button type="button" id="save-all">Save All</button>

        <script>
            const pumpList = document.getElementById("pumpList");
            const valveList = document.getElementById("valveList");

            function GetPortList(portList) {
                const ports = [];

                for (const li of portList.children) {
                    const inputs = li.querySelectorAll("input");

                    ports.push({
                        index: Number(inputs[0].value),
                        current: Number(inputs[1].value),
                        voltage: Number(inputs[2].value),
                        externalPower: inputs[3].checked
                    });
                }
                return ports;
            }

            document.getElementById("save-all").addEventListener("click", async () => {
                const switchPortsJson = {
                    pumps: GetPortList(pumpList),
                    valves: GetPortList(valveList),
                };

                const response = await fetch("/api/switch-port/save-all", {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify(switchPortsJson)
                });

                window.location.href = "/";
            });

            function CreateCheckbox(container, isChecked) {
                const checkbox = document.createElement("input");
                checkbox.type = "checkbox";
                checkbox.checked = isChecked;
                container.appendChild(checkbox);
            }
            function CreateTextInput(container, value, width) {
                const input = document.createElement("input");
                input.value = value;

                input.style.width = width;
                container.appendChild(input);
            }
            function CreateNumberInput(container, value, width, min, max, step) {
                const input = document.createElement("input");
                input.value = value;

                input.type = "number";
                input.min = min;
                input.max = max;
                input.step = step;

                input.style.width = width;
                container.appendChild(input);
            }
            function CreateText(container, value) {
                const text = document.createElement("span");
                text.textContent = value;
                container.appendChild(text);
            }

            document.getElementById("addPumpPort").addEventListener("click", () => {
                const li = document.createElement("li");

                CreateText(li, "Index: ");
                CreateNumberInput(li, "", "30px", 1, "", 1);

                CreateText(li, ", Current (A): ");
                CreateNumberInput(li, 0.4, "30px", 0, 10, "");
                
                CreateText(li, ", Voltage (V): ");
                CreateNumberInput(li, 12, "30px", 0, "", "");

                CreateText(li, ", External Power: ");
                CreateCheckbox(li, false);

                pumpList.appendChild(li);
            });
            document.getElementById("addValvePort").addEventListener("click", () => {
                const li = document.createElement("li");

                CreateText(li, "Index: ");
                CreateNumberInput(li, "", "30px", 1, "", 1);

                CreateText(li, ", Current (A): ");
                CreateNumberInput(li, 0.4, "30px", 0, 10, "");

                CreateText(li, ", Voltage (V): ");
                CreateNumberInput(li, 12, "30px", 0, "", "");

                CreateText(li, ", External Power: ");
                CreateCheckbox(li, false);

                valveList.appendChild(li);
            });
            document.getElementById("removePumpPort").addEventListener("click", () => {
                const last = pumpList.lastElementChild;
                if (last) last.remove();
            });
            document.getElementById("removeValvePort").addEventListener("click", () => {
                const last = valveList.lastElementChild;
                if (last) last.remove();
            });

            function DisplayPorts(ports) {
                pumpList.innerHTML = "";
                valveList.innerHTML = "";
 
                ports.pumps.forEach((pump, index) => {
                    const li = document.createElement("li");

                    CreateText(li, "Index: ");
                    CreateNumberInput(li, pump.index, "30px", 1, "", 1);

                    CreateText(li, ", Current (A): ");
                    CreateNumberInput(li, pump.current, "30px", 0, 10, "");

                    CreateText(li, ", Voltage (V): ");
                    CreateNumberInput(li, pump.voltage, "30px", 0, "", "");

                    CreateText(li, ", External Power: ");
                    CreateCheckbox(li, pump.externalPower);

                    pumpList.appendChild(li);
                });
                ports.valves.forEach((valve, index) => {
                    const li = document.createElement("li");

                    CreateText(li, "Index: ");
                    CreateNumberInput(li, valve.index, "30px", 1, "", 1);

                    CreateText(li, ", Current (A): ");
                    CreateNumberInput(li, valve.current, "30px", 0, 10, "");

                    CreateText(li, ", Voltage (V): ");
                    CreateNumberInput(li, valve.voltage, "30px", 0, "", "");

                    CreateText(li, ", External Power: ");
                    CreateCheckbox(li, valve.externalPower);

                    valveList.appendChild(li);
                });
            }
            
            async function LoadPorts() {
                const response = await fetch("/api/switch-port/get-all");
                const ports = await response.json();

                DisplayPorts(ports);
            }
            
            document.addEventListener("DOMContentLoaded", async () => {
                await LoadPorts();
            });
        </script>
    </body>
    </html>
    )";
    server.send(200, "text/html", response);
}
void HandleTime() {
    String response = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Time - Regulated Irrigation System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Time - Regulated Irrigation System</h1>

        <a href="/"><button>RIS</button></a>

        <hr>

        <form id="timeForm">
            <h3>Set Time:</h3>
            <div id="timeContainer">
                <label>Second: <input type="number" id="timeSec" min="0" max="59" value="0"></label><br>
                <label>Minute: <input type="number" id="timeMin" min="0" max="59" value="0"></label><br>
                <label>Hour:   <input type="number" id="timeHour" min="0" max="23" value="0"></label><br>
                <label>Date:   <input type="number" id="timeDayDate" min="1" max="31" value="1"></label><br>
                <label>Month:  <input type="number" id="timeMonth" min="1" max="12" value="1"></label><br>
                <label>Year:   <input type="number" id="timeYear" min="1970" max="2100" value="2026"></label><br>
            </div>

            <button type="submit">Set Time</button>
        </form>

        <script>
            document.getElementById("timeForm").addEventListener("submit", async (e) => {
                e.preventDefault();

                const timeJson = {
                    sec: parseInt(document.getElementById("timeSec").value) || 0,
                    min: parseInt(document.getElementById("timeMin").value) || 0,
                    hour: parseInt(document.getElementById("timeHour").value) || 0,
                    dayDate: parseInt(document.getElementById("timeDayDate").value) || 1,
                    month: parseInt(document.getElementById("timeMonth").value) || 0,
                    year: parseInt(document.getElementById("timeYear").value) || 2026
                };

                const response = await fetch(`/api/set-time`, {
                    method: "POST",
                    headers: { "Content-Type": "application/json" },
                    body: JSON.stringify(timeJson)
                });

                window.location.href = "/";
            });
        </script>
    </body>
    </html>
    )";
    server.send(200, "text/html", response);
}
void HandleWifi() {
    String response = R"(
    <!DOCTYPE html>
    <html lang="en">
    <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>Wifi - Regulated Irrigation System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Wifi - Regulated Irrigation System</h1>

        <a href="/"><button>RIS</button></a>

        <hr>

        <form id="ssidForm">
            <label for="ssid">SSID</label>
            <input type="text" name="ssid">
            <input type="submit" value="Set SSID">
        </form>
        <form id="passwordForm">
            <label for="password">Password </label>
            <input type="password" name="password">
            <input type="submit" value="Set Password">
        </form>

        <script>
            const ssidForm = document.getElementById('ssidForm');
            const passwordForm = document.getElementById('passwordForm');

            ssidForm.addEventListener('submit', async (e) => {
                e.preventDefault();
                const formData = new FormData(ssidForm);

                ssidForm.reset();
                window.location.reload();

                await fetch("/api/set-wifi", {
                    method: 'POST',
                    body: formData
                });
            });
            passwordForm.addEventListener('submit', async (e) => {
                e.preventDefault();
                const formData = new FormData(passwordForm);

                passwordForm.reset();
                window.location.reload();

                await fetch("/api/set-wifi", {
                    method: 'POST',
                    body: formData
                });
            });
        </script>
    </body>
    </html>
    )";
    server.send(200, "text/html", response);
}
void Handle404() {
    server.send(404, "text/plain", "404 Not Found");
}

void HandleGetTemp() {
    String temp = String(GetTemp());
    server.send(200, "text/plain", temp);
}
void HandleGetTime() {
    Time time = GetTimeNow();

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
    routine.tempRangeDurations = {};
    
    routine.name = doc["name"].as<String>().c_str();
    routine.timeInterval = doc["timeInterval"].as<int>();

    JsonArray pumpPorts = doc["pumpPorts"].as<JsonArray>();
    for (uint32_t b : pumpPorts) routine.pumpPorts.push_back(b - 1);

    JsonArray valvePorts = doc["valvePorts"].as<JsonArray>();
    for (uint32_t b : valvePorts) routine.valvePorts.push_back(b - 1);
    
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
    routine.tempRangeDurations = {};
    
    routine.name = doc["name"].as<String>().c_str();
    routine.timeInterval = doc["timeInterval"].as<int>();

    JsonArray pumpPorts = doc["pumpPorts"].as<JsonArray>();
    for (uint32_t b : pumpPorts) routine.pumpPorts.push_back(b - 1);

    JsonArray valvePorts = doc["valvePorts"].as<JsonArray>();
    for (uint32_t b : valvePorts) routine.valvePorts.push_back(b - 1);
    
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

    JsonArray pumpPortsJson = doc.createNestedArray("pumpPorts");
    for (int i = 0; i < routine.pumpPorts.size(); i++) {
        pumpPortsJson.add(routine.pumpPorts[i]);
    }
    JsonArray valvePortsJson = doc.createNestedArray("valvePorts");
    for (int i = 0; i < routine.valvePorts.size(); i++) {
        valvePortsJson.add(routine.valvePorts[i]);
    }

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
    WiFi.softAP(ssid, password);

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
