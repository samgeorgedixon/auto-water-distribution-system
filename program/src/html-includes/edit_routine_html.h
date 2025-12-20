#pragma once
#include <Arduino.h>

const char edit_routine_html[] PROGMEM = R"rawliteral(
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

)rawliteral";
