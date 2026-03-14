#pragma once
#include <Arduino.h>

const char switch_ports_html[] PROGMEM = R"rawliteral(
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

)rawliteral";
