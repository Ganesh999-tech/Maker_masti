#include <HX711_ADC.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Pins:
const int HX711_dout = 9; // MCU > HX711 dout pin
const int HX711_sck = 8;  // MCU > HX711 sck pin

// HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

// I2C LCD addresses (change if necessary):
LiquidCrystal_I2C lcdEarth(0x27, 16, 2);   // LCD for Earth weight
LiquidCrystal_I2C lcdMercury(0x26, 16, 2); // LCD for Mercury weight
LiquidCrystal_I2C lcdVenus(0x25, 16, 2);   // LCD for Venus weight
LiquidCrystal_I2C lcdMars(0x24, 16, 2);    // LCD for Mars weight
LiquidCrystal_I2C lcdJupiter(0x23, 16, 2); // LCD for Jupiter weight
LiquidCrystal_I2C lcdSaturn(0x22, 16, 2);  // LCD for Saturn weight
LiquidCrystal_I2C lcdUranus(0x21, 16, 2);  // LCD for Uranus weight
LiquidCrystal_I2C lcdNeptune(0x20, 16, 2); // LCD for Neptune weight

// Calibration and smoothing parameters:
const float calibrationValue = -26.45; // Calibration value
const int numReadings = 10;           // Number of readings for averaging

// Gravity constants relative to Earth
const float gravityEarth = 9.81; // m/s^2
const float gravityMercury = 0.38 * gravityEarth;
const float gravityVenus = 0.91 * gravityEarth;
const float gravityMars = 0.38 * gravityEarth;
const float gravityJupiter = 2.34 * gravityEarth;
const float gravitySaturn = 1.06 * gravityEarth;
const float gravityUranus = 0.92 * gravityEarth;
const float gravityNeptune = 1.19 * gravityEarth;

// Variables for moving average filter:
float readings[numReadings]; // The readings from the analog input
int readIndex = 0;           // The index of the current reading
float total = 0;             // The running total
float average = 0;           // The average

// Timing variables:
unsigned long t = 0;

// Scrolling text buffers
String earthText, mercuryText, venusText, marsText, jupiterText, saturnText, uranusText, neptuneText;

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
  lcdEarth.init();
  lcdEarth.backlight();
  lcdMercury.init();
  lcdMercury.backlight();
  lcdVenus.init();
  lcdVenus.backlight();
  lcdMars.init();
  lcdMars.backlight();
  lcdJupiter.init();
  lcdJupiter.backlight();
  lcdSaturn.init();
  lcdSaturn.backlight();
  lcdUranus.init();
  lcdUranus.backlight();
  lcdNeptune.init();
  lcdNeptune.backlight();
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

      // Convert grams to kilograms
      float weightOnEarthKg = average / 1000.0;
      float weightOnMercuryKg = weightOnEarthKg * gravityMercury / gravityEarth;
      float weightOnVenusKg = weightOnEarthKg * gravityVenus / gravityEarth;
      float weightOnMarsKg = weightOnEarthKg * gravityMars / gravityEarth;
      float weightOnJupiterKg = weightOnEarthKg * gravityJupiter / gravityEarth;
      float weightOnSaturnKg = weightOnEarthKg * gravitySaturn / gravityEarth;
      float weightOnUranusKg = weightOnEarthKg * gravityUranus / gravityEarth;
      float weightOnNeptuneKg = weightOnEarthKg * gravityNeptune / gravityEarth;

      // Prepare display text
      earthText = "Earth " + String(weightOnEarthKg, 2) + " kgf";
      mercuryText = "Mercury " + String(weightOnMercuryKg, 2) + " kgf";
      venusText = "Venus " + String(weightOnVenusKg, 2) + " kgf";
      marsText = "Mars " + String(weightOnMarsKg, 2) + " kgf";
      jupiterText = "Jupiter " + String(weightOnJupiterKg, 2) + " kgf";
      saturnText = "Saturn " + String(weightOnSaturnKg, 2) + " kgf";
      uranusText = "Uranus " + String(weightOnUranusKg, 2) + " kgf";
      neptuneText = "Neptune " + String(weightOnNeptuneKg, 2) + " kgf";

      // Display the weight for each planet on respective LCDs
      displayWeight(lcdEarth, earthText, weightOnEarthKg);
      displayWeight(lcdMercury, mercuryText, weightOnMercuryKg);
      displayWeight(lcdVenus, venusText, weightOnVenusKg);
      displayWeight(lcdMars, marsText, weightOnMarsKg);
      displayWeight(lcdJupiter, jupiterText, weightOnJupiterKg);
      displayWeight(lcdSaturn, saturnText, weightOnSaturnKg);
      displayWeight(lcdUranus, uranusText, weightOnUranusKg);
      displayWeight(lcdNeptune, neptuneText, weightOnNeptuneKg);

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

void displayWeight(LiquidCrystal_I2C &lcd, const String &planetText, float weightKg) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(planetText); // Planet name and weight in kgf
  lcd.setCursor(0, 1);
  lcd.print(String(weightKg * gravityEarth, 2) + " N"); // Weight in Newtons
}
