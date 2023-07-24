#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Parser.h"
#include "AsyncStream.h"
AsyncStream<50> serial(&Serial, ';');

// Define Arduino pins
const int massSensorPin = A0;
const int saveBPin = 8;
const int zeroBPin = 9;
const int blueLedPin = 6;
const int greenLedPin = 7;
const int redLedPin = 5;

// Initialize variables
float mass;
bool saveB;
bool zeroB;
float mass_set;
int accuracy;
bool accuracy_minus;
int zero;

// Define LCD properties
LiquidCrystal_I2C lcd(0x27, 16, 2);

bool setZeroBOnceFlag = false;  // Flag to indicate whether to set zeroBPin to HIGH only once

void setup() {
  pinMode(saveBPin, OUTPUT);
  pinMode(zeroBPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);

  Serial.begin(115200);
  Wire.begin();

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
  lcd.print("Ready!");
  delay(1000);
  lcd.clear();
}

void loop() {
  parsing();

  // Calculate mass and accuracy limits
  if ((digitalRead(zeroBPin) == HIGH) || setZeroBOnceFlag) {
    zero = map(analogRead(massSensorPin), 0, 1023, 0.0, 52.0);
    setZeroBOnceFlag = false;  // Reset the flag to avoid entering this block again
  }

  mass = map(analogRead(massSensorPin), 0, 1023, 0.0, 52.0) - zero;
  float accuracy_high = mass_set + accuracy / 1000.0;

  if (accuracy_minus) {
    float accuracy_low = mass_set - accuracy / 1000.0;
    digitalWrite(greenLedPin, mass < accuracy_low);
    digitalWrite(blueLedPin, mass >= accuracy_low && mass <= accuracy_high);
    digitalWrite(redLedPin, mass > accuracy_high);
  }

  // Prepare the data to send to PC
  String send_data = "{";
  send_data +=String(mass, 1) + ", ";
  send_data +=String(digitalRead(saveBPin)) + ", ";
  send_data +=String(digitalRead(zeroBPin)) + ", ";
  send_data +=String(digitalRead(blueLedPin)) + ", ";
  send_data +=String(digitalRead(greenLedPin)) + ", ";
  send_data +=String(digitalRead(redLedPin)) + "}";

  // Send the data to PC
  Serial.println(send_data);

  // Update LCD display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("M" + String(mass, 1));
  lcd.setCursor(6, 0);
  lcd.print("R" + String(digitalRead(redLedPin)));
  lcd.setCursor(8, 0);
  lcd.print("G" + String(digitalRead(greenLedPin)));
  lcd.setCursor(10, 0);
  lcd.print("B" + String(digitalRead(blueLedPin)));
  lcd.setCursor(12, 0);
  lcd.print("S" + String(digitalRead(saveBPin)));
  lcd.setCursor(14, 0);
  lcd.print("Z" + String(digitalRead(zeroBPin)));
  lcd.setCursor(0, 1);
  lcd.print("W" + String(mass_set, 1));
  lcd.setCursor(7, 1);
  lcd.print("A" + String(accuracy));
  lcd.setCursor(13, 1);
  lcd.print("AM" + String(accuracy_minus));
  delay(500);
}

void parsing() {
  if (serial.available()) {
    Parser data(serial.buf, ',');
    int ints[10];
    data.parseInts(ints);

    switch (ints[0]) {
      case 0:
        zeroB = ints[1];
        if (zeroB == 1) {
          setZeroBOnceFlag = true;  // Set the flag to true when receiving case 0 with zeroB = 1
        }
        break;
      case 1:
        accuracy_minus = bool(ints[1]);
        break;
      case 2:
        mass_set = ints[1];
        break;
      case 3:
        accuracy = ints[1];
        break;
      // Add more cases if needed
    }
  }
}