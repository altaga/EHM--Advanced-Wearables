#include <Wire.h>
#include "Adafruit_MLX90614.h"
#include "MAX30100_PulseOximeter.h"

#define datapin 2
#define command 3
#define enable  4
#define REPORTING_PERIOD_MS     1000

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
PulseOximeter pox;

uint32_t tsLastReport = 0;

void setup() {
  pinMode(datapin, OUTPUT);
  pinMode(command, OUTPUT);
  pinMode(enable, OUTPUT);
  digitalWrite(datapin, LOW);
  digitalWrite(command, LOW);
  digitalWrite(enable, LOW);
  if (!pox.begin()) {
    for (;;);
  } else {
  }
  mlx.begin();
}

void loop() {
  pox.update();
  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    sendTriWire(85, 1);
    float temps = mlx.readObjectTempC();
    unsigned int temp = round(temps);
    sendTriWire((unsigned char)temp, 2);
    temps = pox.getHeartRate();
    temp = round(temps);
    sendTriWire((unsigned char)temp, 3);
    temps = pox.getSpO2();
    temp = round(temps);
    sendTriWire((unsigned char)temp, 4);
    tsLastReport = millis();
  }
}

void sendBitTriWire(bool mybit) {
  digitalWrite(datapin, mybit);
  digitalWrite(command, HIGH);
  delayMicroseconds(1);
  digitalWrite(command, LOW);
  delayMicroseconds(1);
}

void sendTriWire(unsigned char data, unsigned char sensor) {
  digitalWrite(enable, HIGH);
  delay(1);
  sendBitTriWire((data & 1));
  sendBitTriWire((data & 2) >> 1);
  sendBitTriWire((data & 4) >> 2);
  sendBitTriWire((data & 8) >> 3);
  sendBitTriWire((data & 16) >> 4);
  sendBitTriWire((data & 32) >> 5);
  sendBitTriWire((data & 64) >> 6);
  sendBitTriWire((data & 128) >> 7);
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
