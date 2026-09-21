#include <Arduino.h>
#include "display.h"
#include "inputs.h"
#include "game_logic.h"

unsigned long lastBatUpdate = 0;

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
  // Update battery percentage every 5 seconds
  if (millis() - lastBatUpdate > 5000) {
    lastBatUpdate = millis();
    int pct = getBatteryPercentage();
    drawBatteryIndicator(pct);
  }
}