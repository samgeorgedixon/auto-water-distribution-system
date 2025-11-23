#include "network.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <esp_wifi.h>
#include <Preferences.h>

#include "components.h"

String ssid = "AWDS";
String password ="admin123";

IPAddress localIP(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

Preferences preferences;

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
        <h1>Auto Water Distribution System</h1>

        <p id="time">Time: ...</p>
        <p id="temp">Temperature: ...</p>
        
        <a href="time"><button >Set Time</button></a>
        <a href="wifi"><button >Set Wifi Credentials</button></a>

        <a href="routines"><button >View Routines</button></a>
        
        <script>
            fetch('/get/temp')
                .then(res => res.text())
                .then(temp => {
                    document.getElementById('temp').textContent = "Temperature: " + temp;
                });
            fetch('/get/time')
                .then(res => res.text())
                .then(time => {
                    document.getElementById('time').textContent = "Time: " + time;
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
        <title>Routines - Auto Water Distribution System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Routines - Auto Water Distribution System</h1>

        <a href=""><button >Add Routine</button></a>
        <a href=""><button >Edit Routine... 1</button></a>

        <p>Routine List...</p>
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
        <title>Time - Auto Water Distribution System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Time - Auto Water Distribution System</h1>

        <a href=""><button >Set Time</button></a>
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
        <title>Wifi - Auto Water Distribution System</title>

        <style>
            html {
                font-family: Segoe UI;
            }
        </style>
    </head>
    <body>
        <h1>Wifi - Auto Water Distribution System</h1>

        <form id="ssidForm" action="/set" target="hidden_iframe">
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
            const form = document.getElementById('passwordForm');

            form.addEventListener('submit', async (e) => {
                e.preventDefault();
                const formData = new FormData(form);

                await fetch("/set", {
                    method: 'POST',
                    body: formData
                });

                window.location.replace("/"); 
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
    server.send(200, "text/plain", "0");
}
void HandleSet() {
    if (server.hasArg("ssid")) {
        ssid = server.arg("ssid");

        preferences.begin("wifi", false);
        preferences.putString("ssid", ssid);
        preferences.end();

        StopNetwork();
        delay(100);
        SetupNetwork();

        Serial.println("Set SSID: " + ssid);
        server.send(200, "text/plain", "Set SSID");
    } else if (server.hasArg("password")) {
        password = server.arg("password");

        preferences.begin("wifi", false);
        preferences.putString("password", password);
        preferences.end();
        
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

void SetupServerHandles() {
    server.on("/", HandleRoot);
    server.on("/routines", HandleRoutines);
    server.on("/time", HandleTime);
    server.on("/wifi", HandleWifi);
    
    server.on("/get/temp", HandleGetTemp);
    server.on("/get/time", HandleGetTime);
    
    server.on("/set", HandleSet);
    
    server.onNotFound(Handle404);
}

void SetupNetwork() {
    preferences.begin("wifi", false);
    
    ssid = preferences.getString("ssid", ssid);
    password = preferences.getString("password", password);

    preferences.end();

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
