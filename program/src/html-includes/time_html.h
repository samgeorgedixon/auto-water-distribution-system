#pragma once
#include <Arduino.h>

const char time_html[] PROGMEM = R"rawliteral(
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
            <label>Time: <input type="time" required id="time" step="1"></label><br>
            <label>Date: <input type="date" required id="date" min="2000-01-01" max="2100-01-01"></label><br>
        </div>

        <button type="submit">Set Time</button>
    </form>

    <script>
        document.getElementById("timeForm").addEventListener("submit", async (e) => {
            e.preventDefault();

            const timeInput = document.getElementById("time").value;
            const dateInput = document.getElementById("date").value;
            
            const [h, m, s = "0"] = timeInput.split(":");
            const [y, mm, d] = dateInput.split("-");

            const timeJson = {
                sec: parseInt(s) || 0,
                min: parseInt(m) || 0,
                hour: parseInt(h) || 0,
                dayDate: parseInt(d) || 1,
                month: parseInt(mm) || 1,
                year: parseInt(y) || 2000
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

)rawliteral";
