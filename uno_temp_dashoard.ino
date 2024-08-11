#include <DHT11.h>
#include <SoftwareSerial.h>

// Define SoftwareSerial pins (e.g., Rx = 10, Tx = 11)
SoftwareSerial mySerial(8, 9); // RX, TX

DHT11 dht11(A0);

void setup() {
    mySerial.begin(9600);  // Communication with ESP8266 via SoftwareSerial
    Serial.begin(9600);    // For debugging via USB Serial
    Serial.println("Arduino Mega Initializing...");
}

void loop() {
    int temperature = 0;
    int humidity = 0;

    int result = dht11.readTemperatureHumidity(temperature, humidity);

    if (result == 0) {
        String data = "Temperature:" + String(temperature) + ",Humidity:" + String(humidity);
        mySerial.println(data);  // Send data to ESP8266 via SoftwareSerial
        Serial.println(data);    // Also print data to Serial Monitor for debugging
    } else {
        Serial.println(DHT11::getErrorString(result));
    }

    delay(2000);  // Adjust as needed
}
