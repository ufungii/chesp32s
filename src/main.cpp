#include <Arduino.h>
#include "display.h"
#include "inputs.h"

void setup() {
  Serial.begin(115200);
  initDisplay();
  initInputs();
  Serial.println("Chess System Booted to Main Menu!");
}

void loop() {
  processInputs();
  delay(20);
}