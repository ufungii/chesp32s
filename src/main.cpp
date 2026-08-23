#include <Arduino.h>
#include "display.h"
#include "inputs.h"
#include "game_logic.h"

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Booting ESP32 Chess Console...");

  initBoard();
  initDisplay();
  initInputs();

  drawHomeScreen();
  Serial.println("System Ready! Enjoy the game.");
}

void loop() {
  processInputs();
}