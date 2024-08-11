#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "mad";  // Change to your SSID
const char* password = "password";  // Change to your password

ESP8266WebServer server(80);

String temperature = "0";
String humidity = "0";

// Arrays to store temperature and humidity values over time
float tempData[10] = {0};
int dataIndex = 0;

void setup() {
  Serial.begin(9600);  // Communication with Arduino Mega
  Serial.println("ESP8266 Initializing...");

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

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    int tempIndex = data.indexOf("Temperature:");
    int humidIndex = data.indexOf("Humidity:");
    if (tempIndex != -1 && humidIndex != -1) {
      temperature = data.substring(tempIndex + 12, data.indexOf(",", tempIndex));
      humidity = data.substring(humidIndex + 9);

      // Store the values in the arrays
      tempData[dataIndex] = temperature.toFloat();

      dataIndex = (dataIndex + 1) % 10; // Keep the array index within bounds (0-9)
    }
  }
  server.handleClient();
}

void handleRoot() {
  String tempString = "[";

  for (int i = 0; i < 10; i++) {
    tempString += String(tempData[i]);
    if (i < 9) {
      tempString += ",";
    }
  }

  tempString += "]";

  String html = "<html>"
                "<head>"
                "<style>"
                "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; margin: 0; padding: 0; }"
                ".container { margin-top: 50px; }"
                "#tempGraph { width: 100%; max-width: 600px; margin: 0 auto; }"
                "#humidityGauge { width: 100%; max-width: 300px; height: 150px; margin: 0 auto; }"
                "</style>"
                "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>"
                "<script src='https://cdnjs.cloudflare.com/ajax/libs/justgage/1.2.9/justgage.min.js'></script>"
                "<script src='https://cdnjs.cloudflare.com/ajax/libs/raphael/2.1.4/raphael-min.js'></script>"
                "</head>"
                "<body>"
                "<div class='container'>"
                "<h1>Temperature and Humidity Dashboard</h1>"
                "<canvas id='tempGraph'></canvas>"
                "<div id='humidityGauge'></div>"
                "</div>"
                "<script>"
                "const ctx = document.getElementById('tempGraph').getContext('2d');"
                "const tempData = {"
                "    labels: Array.from({length: 10}, (_, i) => i + 1),"
                "    datasets: [{"
                "        label: 'Temperature (°C)',"
                "        data: " + tempString + ","
                "        fill: false,"
                "        borderColor: 'rgba(75, 192, 192, 1)',"
                "        tension: 0.1"
                "    }]"
                "};"
                "const tempChart = new Chart(ctx, {"
                "    type: 'line',"
                "    data: tempData,"
                "    options: {"
                "        scales: {"
                "            x: { beginAtZero: true },"
                "            y: { beginAtZero: true }"
                "        }"
                "    }"
                "});"
                "var gauge = new JustGage({"
                "    id: 'humidityGauge',"
                "    value: " + humidity + ","
                "    min: 0,"
                "    max: 100,"
                "    title: 'Humidity (%)'"
                "});"
                "</script>"
                "</body>"
                "</html>";
  server.send(200, "text/html", html);
}
