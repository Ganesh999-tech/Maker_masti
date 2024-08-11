#include <SoftwareSerial.h>

// Define SoftwareSerial pins (e.g., Rx = 8, Tx = 9)
SoftwareSerial mySerial(8, 9); // RX, TX

// Device pins
const int devicePins[] = {2, 3, 4, 5, 6, 7};
const int numDevices = sizeof(devicePins) / sizeof(devicePins[0]);

void setup() {
  Serial.begin(9600); // For debugging
  mySerial.begin(9600);  // Communication with ESP8266
  Serial.println("Arduino Uno Initializing...");

  // Initialize device pins
  for (int i = 0; i < numDevices; i++) {
    pinMode(devicePins[i], OUTPUT);
    digitalWrite(devicePins[i], LOW);
  }
}

void loop() {
  if (mySerial.available()) {
    String command = mySerial.readStringUntil('\n');
    command.trim();
    Serial.println("Received command: " + command);
    executeCommand(command);
  }
}

// Function to execute received commands
void executeCommand(String command) {
  if (command.startsWith("SET")) {
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    if (firstSpace == -1 || secondSpace == -1) {
      Serial.println("Invalid command format");
      return;
    }

    int pin = command.substring(firstSpace + 1, secondSpace).toInt();
    int state = command.substring(secondSpace + 1).toInt();

    // Validate pin
    bool validPin = false;
    for (int i = 0; i < numDevices; i++) {
      if (devicePins[i] == pin) {
        validPin = true;
        break;
      }
    }

    if (!validPin) {
      Serial.println("Invalid pin: " + String(pin));
      return;
    }

    // Validate state
    if (state != 0 && state != 1) {
      Serial.println("Invalid state: " + String(state));
      return;
    }

    digitalWrite(pin, state == 1 ? HIGH : LOW);
    Serial.println("Set pin " + String(pin) + " to " + (state == 1 ? "HIGH" : "LOW"));
  } else if (command == "STATUS") {
    // Report the status of all device pins
    String status = "STATUS";
    for (int i = 0; i < numDevices; i++) {
      int pinState = digitalRead(devicePins[i]);
      status += " " + String(devicePins[i]) + ":" + String(pinState);
    }
    mySerial.println(status);
    Serial.println("Sent status: " + status);
  } else {
    Serial.println("Unknown command");
  }
}

