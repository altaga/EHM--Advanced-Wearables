#include <Wire.h>
#include "Adafruit_MLX90614.h"
#include "MAX30100_PulseOximeter.h"

// TriWire pin definitions

#define datapin 2
#define command 3
#define enable  4

// Reporting values every 1 second

#define REPORTING_PERIOD_MS     1000

// Define Class Sensors.

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
PulseOximeter pox;

// Define timer reporting variable

uint32_t tsLastReport = 0;

void setup() {
  // Setup the TriWire Gpio to Output
  pinMode(datapin, OUTPUT);
  pinMode(command, OUTPUT);
  pinMode(enable, OUTPUT);
  // Setup All Values LOW
  digitalWrite(datapin, LOW);
  digitalWrite(command, LOW);
  digitalWrite(enable, LOW);
  // Begin SPO2 Sensor
  if (!pox.begin()) {
    for (;;);
  } else {
  }
  // Begin Temperature Sensor
  mlx.begin();
}

void loop() {
  pox.update(); // Call this Function as fast as posible.
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    sendTriWire(85, 1); // Sening first Dummy Data
    float temps = mlx.readObjectTempC();
    unsigned int temp = round(temps);
    // Sending Temperature Data
    sendTriWire((unsigned char)temp, 2);
    temps = pox.getHeartRate();
    temp = round(temps);
    // Sending BPM Data
    sendTriWire((unsigned char)temp, 3);
    temps = pox.getSpO2();
    temp = round(temps);
    // Sending SPO2
    sendTriWire((unsigned char)temp, 4);
    tsLastReport = millis();
  }
}

// Send one bit 
void sendBitTriWire(bool mybit) {
  digitalWrite(datapin, mybit);
  digitalWrite(command, HIGH);
  delayMicroseconds(1);
  digitalWrite(command, LOW);
  delayMicroseconds(1);
}

// Send sensor number and value encoding in 32 bits
void sendTriWire(unsigned char data, unsigned char sensor) {
  digitalWrite(enable, HIGH);
  delay(1);
  // Encoding sensor number
  sendBitTriWire((data & 1));
  sendBitTriWire((data & 2) >> 1);
  sendBitTriWire((data & 4) >> 2);
  sendBitTriWire((data & 8) >> 3);
  sendBitTriWire((data & 16) >> 4);
  sendBitTriWire((data & 32) >> 5);
  sendBitTriWire((data & 64) >> 6);
  sendBitTriWire((data & 128) >> 7);
  // Encoding sensor value
  sendBitTriWire((sensor & 1));
  sendBitTriWire((sensor & 2) >> 1);
  sendBitTriWire((sensor & 4) >> 2);
  sendBitTriWire((sensor & 8) >> 3);
  sendBitTriWire((sensor & 16) >> 4);
  sendBitTriWire((sensor & 32) >> 5);
  sendBitTriWire((sensor & 64) >> 6);
  sendBitTriWire((sensor & 128) >> 7);
  digitalWrite(enable, LOW);
  delayMicroseconds(1);
}
