#include <HX711_ADC.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pins:
const int HX711_dout = 14; // MCU > HX711 dout pin
const int HX711_sck = 15;  // MCU > HX711 sck pin

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

// I2C LCD addresses (change if necessary):
LiquidCrystal_I2C lcd1(0x27, 16, 2); // LCD for Earth weight
LiquidCrystal_I2C lcd2(0x26, 16, 2); // LCD for Moon weight

// Calibration and smoothing parameters:
const float calibrationValue = 24.40; // Calibration value
const int numReadings = 10;           // Number of readings for averaging

// Variables for moving average filter:
float readings[numReadings]; // The readings from the analog input
int readIndex = 0;            // The index of the current reading
float total = 0;              // The running total
float average = 0;            // The average

// Timing variables:
unsigned long t = 0;

void setup() {
  Serial.begin(9600);
  delay(10);
  Serial.println();
  Serial.println("Starting...");

  // Initialize readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

  LoadCell.begin();
  unsigned long stabilizingtime = 2000; // Tare precision can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; // Set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  } else {
    LoadCell.setCalFactor(calibrationValue); // Set calibration factor (float)
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());
  Serial.print("Calibration value: ");
  Serial.println(LoadCell.getCalFactor());
  Serial.print("HX711 measured conversion time ms: ");
  Serial.println(LoadCell.getConversionTime());
  Serial.print("HX711 measured sampling rate HZ: ");
  Serial.println(LoadCell.getSPS());
  Serial.print("HX711 measured settlingtime ms: ");
  Serial.println(LoadCell.getSettlingTime());
  Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
  if (LoadCell.getSPS() < 7) {
    Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
  } else if (LoadCell.getSPS() > 100) {
    Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
  }

  // Initialize LCDs:
  lcd1.init();
  lcd1.backlight();
  lcd2.init();
  lcd2.backlight();
}

void loop() {
  static boolean newDataReady = 0;
  const int serialPrintInterval = 200; // Faster update interval

  // Check for new data/start next conversion:
  if (LoadCell.update()) newDataReady = true;

  // Get smoothed value from the dataset:
  if (newDataReady) {
    if (millis() > t + serialPrintInterval) {
      float i = LoadCell.getData();
      
      // Remove the oldest reading:
      total = total - readings[readIndex];
      // Add the new reading:
      readings[readIndex] = i;
      total = total + readings[readIndex];
      // Advance to the next position in the array:
      readIndex = readIndex + 1;

      // If we're at the end of the array:
      if (readIndex >= numReadings) {
        // ...wrap around to the beginning:
        readIndex = 0;
      }

      // Calculate the average:
      average = total / numReadings;

      // Convert grams to kilograms and round to the nearest whole number
      float weightOnEarthKg = round(average / 1000.0); // Convert grams to kilograms
      float weightOnMoonKg = round((average * 0.165) / 1000.0); // Convert grams to kilograms

      // Display weights on LCDs:
      lcd1.clear();
      lcd1.setCursor(0, 0);
      lcd1.print("Weight on Earth:");
      lcd1.setCursor(0, 1);
      lcd1.print(weightOnEarthKg); // Whole number

      lcd2.clear();
      lcd2.setCursor(0, 0);
      lcd2.print("Weight on Moon:");
      lcd2.setCursor(0, 1);
      lcd2.print(weightOnMoonKg); // Whole number

      // Display approximate weights:
      if (average > 200 && average <= 300) {
        Serial.println("Approximate Weight: 250 grams");
      } else if (average > 450 && average <= 550) {
        Serial.println("Approximate Weight: 500 grams");
      } else if (average > 700 && average <= 800) {
        Serial.println("Approximate Weight: 750 grams");
      }

      newDataReady = 0;
      t = millis();
    }
  }

  // Receive command from serial terminal, send 't' to initiate tare operation:
  if (Serial.available() > 0) {
    char inByte = Serial.read();
    if (inByte == 't') LoadCell.tareNoDelay();
  }

  // Check if last tare operation is complete:
  if (LoadCell.getTareStatus() == true) {
    Serial.println("Tare complete");
  }
}
