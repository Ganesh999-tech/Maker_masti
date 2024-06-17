#define CLK 8
#define DIO 9
#define BUTTON_PIN 2
#define BUZZER_PIN 3
#define IR_SENSOR_PIN A2 // Finish point
#define TOUCH_SENSOR_PIN A1 // Reset or Start point
#define WIN_LED_PIN 4
#define LOSE_LED_PIN 6
#define LED_STRIP_PIN 7
#define SERVO_PIN 5
#define NUM_LEDS 44

#include <TM1637Display.h>
#include <FastLED.h>
#include <Servo.h>

TM1637Display tm1637Display(CLK, DIO);
Servo myservo;  // create servo object to control a servo

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

CRGB leds[NUM_LEDS];

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(TOUCH_SENSOR_PIN, INPUT);
  
  pinMode(WIN_LED_PIN, OUTPUT);
  pinMode(LOSE_LED_PIN, OUTPUT);
  
  tm1637Display.setBrightness(0x0f);
  Serial.begin(9600);

  FastLED.addLeds<NEOPIXEL, LED_STRIP_PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  
  myservo.attach(SERVO_PIN);  // attaches the servo on pin D5 to the servo object
  myservo.write(90);  // initial position of the servo
}

void loop() {
  // Check touch sensor for immediate reset
  int touchSensorValue = digitalRead(TOUCH_SENSOR_PIN);
  if (touchSensorValue == HIGH) {
    resetSystem();
    delay(1000);
    resetLEDs();
    return; // Skip the rest of the loop to prioritize reset
  }

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

  if (irSensorState == HIGH) {
    if (counter < 10) {
      winGame();
    } else {
      loseGame();
    }
    delay(1000);
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
  resetLEDs(); // Reset LEDs when the system resets
  digitalWrite(WIN_LED_PIN, LOW); // Turn off the win LED when resetting
  digitalWrite(LOSE_LED_PIN, LOW); // Turn off the lose LED when resetting
  myservo.write(90); // Reset the servo position to 90 degrees
}

void updateLEDs(int count) {
  FastLED.clear();
  int ledsToLight = count * 4;  // Each hit lights up 4 LEDs

  // Ensure only the first 3 sets light up 11 LEDs each, the last set lights up 10 LEDs
  if (ledsToLight > 44) {
    ledsToLight = 44; // Cap the maximum number of LEDs to NUM_LEDS
  }

  for (int i = 0; i < ledsToLight; i++) {
    if (counter <= 9) {
      if (i < 11) {
        leds[i] = CRGB::Green; // Set first 11 LEDs to green
      } else if (i < 22) {
        leds[i] = CRGB::Yellow; // Set next 11 LEDs to yellow
      } else if (i < 33) {
        leds[i] = CRGB::Orange; // Set next 11 LEDs to orange
      } else {
        leds[i] = CRGB::Red; // Set remaining LEDs to red
      }
    } else {
      leds[i] = CRGB::Red; // Set all LEDs to red
    }
  }
  FastLED.show();
}

void resetLEDs() {
  FastLED.clear();
  FastLED.show();
}

void winGame() {
  tm1637Display.showNumberDecEx(0, 0b0000, true); // Clear the 7-segment display
  digitalWrite(WIN_LED_PIN, HIGH); // Keep the win LED on until reset
  myservo.write(0); // Set servo to 180 degrees when winning
}

void loseGame() {
  digitalWrite(LOSE_LED_PIN, HIGH); // Keep the lose LED on until reset
  myservo.write(180); // Set servo to 0 degrees when losing
}
