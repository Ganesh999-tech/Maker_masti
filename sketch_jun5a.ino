#define CLK 8
#define DIO 9
#define CS_PIN 10
#define MAX_DEVICES 4
#define BUTTON_PIN 2
#define BUZZER_PIN 3
#define IR_SENSOR_PIN A2 // Finish point
#define TOUCH_SENSOR_PIN A1 // Reset or Start point
#define LED_PIN_1 4
#define LED_PIN_2 5
#define LED_PIN_3 6
#define LED_PIN_4 7
#define LED_PIN_5 A0
//Din =11,cs=10,clk=13 only for uno 

#include <TM1637Display.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

MD_Parola matrixDisplay = MD_Parola(MD_MAX72XX::FC16_HW, CS_PIN, MAX_DEVICES);
TM1637Display tm1637Display(CLK, DIO);

int buttonState = 0;
int lastButtonState = 0;
int counter = 0;
bool gameOver = false;
unsigned long buzzerPreviousMillis = 0;
unsigned long hitTime = 0;
unsigned long beepStartMillis = 0;
bool isBeeping = false;
int beepDuration = 0;

unsigned long lastDebounceTime = 0; 
const long debounceDelay = 20; 

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(TOUCH_SENSOR_PIN, INPUT);
  
  pinMode(LED_PIN_1, OUTPUT);
  pinMode(LED_PIN_2, OUTPUT);
  pinMode(LED_PIN_3, OUTPUT);
  pinMode(LED_PIN_4, OUTPUT);
  pinMode(LED_PIN_5, OUTPUT);
  
  tm1637Display.setBrightness(0x0f);
  matrixDisplay.begin();
  matrixDisplay.setIntensity(0);
  matrixDisplay.displayClear();
  Serial.begin(9600);
}

void loop() {
  int reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW && counter < 10 && !gameOver) {
        counter++;
        tm1637Display.showNumberDecEx(counter, 0b1111, true);
        Serial.println(counter);
        updateLEDs(counter);
        hitTime = millis();

        if (counter >= 1 && counter < 5) {
          beepForDurationNonBlocking(100);
        } else if (counter >= 5 && counter < 10) {
          beepForDurationNonBlocking(500);
        }

        if (counter == 10) {
          beepForever();
          loseGame();
          gameOver = true;
        }
      }
    }
  }

  if (isBeeping && (millis() - beepStartMillis >= beepDuration)) {
    digitalWrite(BUZZER_PIN, LOW);
    isBeeping = false;
  }

  int irSensorState = digitalRead(IR_SENSOR_PIN);
  int touchSensorValue = digitalRead(TOUCH_SENSOR_PIN);

  if (irSensorState == HIGH) {
    if (counter < 10) {
      winGame();
    } else {
      loseGame();
    }
    delay(1000);
  }

  if (touchSensorValue == HIGH) {
    resetSystem();
    delay(1000);
    resetLEDs();
  }

  lastButtonState = reading;
}

void beepForDurationNonBlocking(int duration) {
  digitalWrite(BUZZER_PIN, HIGH);
  beepStartMillis = millis();
  beepDuration = duration;
  isBeeping = true;
}

void beepForever() {
  digitalWrite(BUZZER_PIN, HIGH);
}

void resetSystem() {
  counter = 0;
  tm1637Display.showNumberDecEx(0, 0b1111, true);
  digitalWrite(BUZZER_PIN, LOW);
  gameOver = false;
  lastButtonState = digitalRead(BUTTON_PIN);
  matrixDisplay.displayClear();
  resetLEDs(); // Reset LEDs when the system resets
}

void updateLEDs(int count) {
  digitalWrite(LED_PIN_1, count >= 2);
  digitalWrite(LED_PIN_2, count >= 4);
  digitalWrite(LED_PIN_3, count >= 6);
  digitalWrite(LED_PIN_4, count >= 8);
  digitalWrite(LED_PIN_5, count >= 10);
}

void resetLEDs() {
  digitalWrite(LED_PIN_1, LOW);
  digitalWrite(LED_PIN_2, LOW);
  digitalWrite(LED_PIN_3, LOW);
  digitalWrite(LED_PIN_4, LOW);
  digitalWrite(LED_PIN_5, LOW);
}

void winGame() {
  tm1637Display.showNumberDecEx(0, 0b0000, true); // Clear the 7-segment display
  displayMessage("WIN");
  delay(2000);
  matrixDisplay.displayClear();
}

void loseGame() {
  displayMessage("LOSE");
  delay(2000);
  matrixDisplay.displayClear();
}

void displayMessage(String message) {
  matrixDisplay.displayText(message.c_str(), PA_CENTER, 100, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  while (!matrixDisplay.displayAnimate());
}