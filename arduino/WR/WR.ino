#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Define Arduino pins
const int massSensorPin = A0;
const int saveBPin = 8;
const int zeroPin = 9;
const int blueLedPin = 6;
const int greenLedPin = 7;
const int redLedPin = 5;

// Initialize variables
int mass = 0;
bool saveB = false;
bool zero = false;
bool accuracy_minus = false;
float mass_set = 0.0;
int accuracy = 0;
float accuracy_high = 0.0;
float accuracy_low = 0.0;

// Define LCD properties
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust the address (0x27) to your LCD module

void setup() {
  // Setup pins
  pinMode(saveBPin, OUTPUT);
  pinMode(zeroPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  // Initialize Serial communication
  Serial.begin(9600);

  // Initialize I2C communication
  Wire.begin();

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
  lcd.print("Ready!");
  delay(1000);
  lcd.clear();
}

void loop() {
  // Check if data is available from the PC
  if (Serial.available() > 0) {
    String receivedData = Serial.readStringUntil('\n');
    processReceivedData(receivedData);
  }

  // Calculate mass and accuracy limits
  if (zero) {
    zero = false;
    zero = analogRead(zeroPin);
  }

  mass = analogRead(massSensorPin) + zero;
  accuracy_high = mass_set + accuracy / 1000.0;

  if (accuracy_minus) {
    accuracy_low = mass_set - accuracy / 1000.0;
    if (accuracy_low < 0) {
      accuracy_low = 0;
    }
  }

  // Control indication lights based on conditions
  digitalWrite(blueLedPin, mass < accuracy_high && mass > accuracy_low);
  digitalWrite(greenLedPin, mass >= accuracy_low && mass <= accuracy_high);
  digitalWrite(redLedPin, mass > accuracy_high);

  // Update LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("M" + String(mass));
  lcd.setCursor(5, 0);
  lcd.print("R" + String(digitalRead(redLedPin)));
  lcd.setCursor(7, 0);
  lcd.print("G" + String(digitalRead(greenLedPin)));
  lcd.setCursor(9, 0);
  lcd.print("B" + String(digitalRead(blueLedPin)));
  lcd.setCursor(11, 0);
  lcd.print("S" + String(saveB));
  lcd.setCursor(13, 0);
  lcd.print("Z" + String(zero));
  lcd.setCursor(0, 1);
  lcd.print("W" + String(mass_set));
  lcd.setCursor(3, 1);
  lcd.print("A" + String(accuracy));
  lcd.setCursor(9, 1);
  lcd.print("AR" + String(accuracy_minus));
  delay(400);
}

void processReceivedData(String data) {
  // Process received data from PC and set corresponding variables
  int separatorIndex = data.indexOf(':');
  String key = data.substring(0, separatorIndex);
  String value = data.substring(separatorIndex + 1);

  if (key == "mass_set") {
    mass_set = value.toFloat();
  } else if (key == "accuracy") {
    accuracy = value.toInt();
  } else if (key == "accuracy_minus") {
    accuracy_minus = value.toInt();
  }
}
