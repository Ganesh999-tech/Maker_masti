#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Wi-Fi credentials
const char* ssid = "mad";  // Replace with your SSID
const char* password = "password";  // Replace with your Password

ESP8266WebServer server(80);

// Device structure
struct Device {
  String name;
  int pin; // Corresponding Arduino pin
  bool state; // ON or OFF
};

// Define devices
Device devices[] = {
  {"Living Room Light", 2, false},
  {"Bedroom Light", 3, false},
  {"Kitchen Light", 4, false},
  {"Bathroom Light", 5, false},
  {"Garage Door", 6, false},
  {"Garden Light", 7, false}
};

const int numDevices = sizeof(devices) / sizeof(devices[0]);

void setup() {
  Serial.begin(9600);  // Communication with Arduino Uno
  Serial.println("ESP8266 Initializing...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retries++;
    if (retries > 20) {
      Serial.println("Failed to connect to WiFi. Restarting...");
      ESP.restart();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Define server routes
  server.on("/", handleRoot);
  server.on("/set", handleSet); // Endpoint to set device state
  server.on("/status", handleStatus); // Endpoint to get device statuses

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle incoming serial data from Arduino
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    Serial.println("Received from Arduino: " + data);
    if (data.startsWith("STATUS")) {
      // Parse status data
      for (int i = 0; i < numDevices; i++) {
        String searchString = " " + String(devices[i].pin) + ":";
        int index = data.indexOf(searchString);
        if (index != -1) {
          char stateChar = data.charAt(index + searchString.length());
          devices[i].state = (stateChar == '1');
        }
      }
    }
  }

  server.handleClient();
}

// Handler for root "/"
void handleRoot() {
  String html = "<!DOCTYPE html>"
                "<html>"
                "<head>"
                "<meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<title>Home Automation Dashboard</title>"
                "<style>"
                "body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 20px; }"
                ".container { display: flex; flex-wrap: wrap; justify-content: center; }"
                ".tile { background-color: #fff; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); margin: 10px; padding: 20px; width: 200px; text-align: center; }"
                ".switch { position: relative; display: inline-block; width: 60px; height: 34px; }"
                ".switch input { opacity: 0; width: 0; height: 0; }"
                ".slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }"
                ".slider:before { position: absolute; content: ''; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: white; transition: .4s; border-radius: 50%; }"
                "input:checked + .slider { background-color: #66bb6a; }"
                "input:focus + .slider { box-shadow: 0 0 1px #66bb6a; }"
                "input:checked + .slider:before { transform: translateX(26px); }"
                ".onoff-label { margin-top: 10px; font-weight: bold; }"
                "</style>"
                "</head>"
                "<body>"
                "<h1>Home Automation Dashboard</h1>"
                "<div class='container'>";

  // Generate tiles for each device
  for (int i = 0; i < numDevices; i++) {
    String checked = devices[i].state ? "checked" : "";
    String stateText = devices[i].state ? "ON" : "OFF";
    html += "<div class='tile'>"
            "<h3>" + devices[i].name + "</h3>"
            "<label class='switch'>"
            "<input type='checkbox' id='switch" + String(i) + "' " + checked + " onclick='toggleSwitch(" + String(i) + ")'>"
            "<span class='slider'></span>"
            "</label>"
            "<div id='status" + String(i) + "' class='onoff-label'>" + stateText + "</div>"
            "</div>";
  }

  html += "</div>"
          "<script>"
          "function toggleSwitch(index) {"
          "  var checkbox = document.getElementById('switch' + index);"
          "  var state = checkbox.checked ? 1 : 0;"
          "  fetch('/set?device=' + index + '&state=' + state)"
          "    .then(response => response.text())"
          "    .then(data => {"
          "      console.log('Set device', index, 'to', state);"
          "      updateStatusLabel(index, state);"
          "    });"
          "}"
          "function updateStatusLabel(index, state) {"
          "  var statusLabel = document.getElementById('status' + index);"
          "  statusLabel.innerText = state == 1 ? 'ON' : 'OFF';"
          "}"
          "function fetchStatus() {"
          "  fetch('/status')"
          "    .then(response => response.json())"
          "    .then(data => {"
          "      data.forEach((state, index) => {"
          "        var checkbox = document.getElementById('switch' + index);"
          "        checkbox.checked = state;"
          "        updateStatusLabel(index, state ? 1 : 0);"
          "      });"
          "    });"
          "}"
          "window.onload = function() {"
          "  fetchStatus();"
          "  setInterval(fetchStatus, 5000);"
          "};"
          "</script>"
          "</body>"
          "</html>";

  server.send(200, "text/html", html);
}

// Handler for "/set" endpoint
void handleSet() {
  if (server.hasArg("device") && server.hasArg("state")) {
    int deviceIndex = server.arg("device").toInt();
    int state = server.arg("state").toInt();
    if (deviceIndex >= 0 && deviceIndex < numDevices && (state == 0 || state == 1)) {
      devices[deviceIndex].state = (state == 1);
      String command = "SET " + String(devices[deviceIndex].pin) + " " + String(state);
      Serial.println(command); // Send command to Arduino
      Serial.flush();
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid parameters");
    }
  } else {
    server.send(400, "text/plain", "Missing parameters");
  }
}

// Handler for "/status" endpoint
void handleStatus() {
  String json = "[";
  for (int i = 0; i < numDevices; i++) {
    json += devices[i].state ? "true" : "false";
    if (i < numDevices - 1) {
      json += ",";
    }
  }
  json += "]";
  server.send(200, "application/json", json);
}
