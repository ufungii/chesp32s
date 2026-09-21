#include "inputs.h"
#include "display.h"
#include "game_logic.h"

// Flags for analog deadzone edge-triggering
static bool joyCenteredX = true;
static bool joyCenteredY = true;

bool lastUp = HIGH, lastDown = HIGH, lastLeft = HIGH, lastRight = HIGH;
bool lastSel = HIGH, lastOpt1 = HIGH, lastBack = HIGH, lastOpt2 = HIGH;

unsigned long bootTime = 0;

void initInputs() {
    pinMode(JOY_SW_PIN, INPUT_PULLUP);
    pinMode(BTN_OPT1, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    pinMode(BTN_OPT2, INPUT_PULLUP);

    analogSetPinAttenuation(JOY_X_PIN, ADC_11db);
    analogSetPinAttenuation(JOY_Y_PIN, ADC_11db);

    bootTime = millis();

    // ADC configuration for battery sensing
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

int getBatteryPercentage() {
  int rawSum = 0;
  for (int i = 0; i < 10; i++) {
    rawSum += analogRead(BAT_SENSE_PIN);
    delay(2);
  }
  float rawAvg = rawSum / 10.0;

  // ESP32 ADC full-scale reference is typically ~3.3V, but internal Vref varies (often 3.1V - 3.3V)
  // Multiply by 2.0 to account for the 100k:100k divider
  float batVoltage = (rawAvg / 4095.0) * 3.3 * 2.0;

  // Usable LiPo discharge curve: 3.4V (0%) to 4.2V (100%)
  int pct = (int)(((batVoltage - 3.4) / (4.2 - 3.4)) * 100.0);
  return constrain(pct, 0, 100);
}

void processInputs() {
    // Read raw ADC values
    int rawX = analogRead(JOY_X_PIN);
    int rawY = analogRead(JOY_Y_PIN);

    // Default virtual D-pad buttons to HIGH (unpressed)
    bool curLeft  = HIGH;
    bool curRight = HIGH;
    bool curUp    = HIGH;
    bool curDown  = HIGH;

    // Horizontal axis edge detection
    if (rawX < JOY_THRESH_LOW) {
        if (joyCenteredX) {
            curLeft = LOW;
            joyCenteredX = false;
        }
    } else if (rawX > JOY_THRESH_HIGH) {
        if (joyCenteredX) {
            curRight = LOW;
            joyCenteredX = false;
        }
    } else {
        joyCenteredX = true; // Returned to deadzone
    }

    // Vertical axis edge detection
    if (rawY < JOY_THRESH_LOW) {
        if (joyCenteredY) {
            curUp = LOW;
            joyCenteredY = false;
        }
    } else if (rawY > JOY_THRESH_HIGH) {
        if (joyCenteredY) {
            curDown = LOW;
            joyCenteredY = false;
        }
    } else {
        joyCenteredY = true; // Returned to deadzone
    }

    // Read physical buttons (Joystick switch is mapped to BTN_SEL)
    bool curSel  = digitalRead(JOY_SW_PIN);
    bool curOpt1 = digitalRead(BTN_OPT1);
    bool curBack = digitalRead(BTN_BACK);
    bool curOpt2 = digitalRead(BTN_OPT2);

    if (millis() - bootTime < 300) {
        lastUp = curUp; lastDown = curDown; lastLeft = curLeft; lastRight = curRight;
        lastSel = curSel; lastOpt1 = curOpt1; lastBack = curBack; lastOpt2 = curOpt2;
        return;
    }

    // --- GLOBAL BACK KEY (`BACK` / GPIO 33) ---
    if (lastBack == HIGH && curBack == LOW) {
        if (currentState != STATE_MENU) {
            currentState = STATE_MENU;
            drawHomeScreen();
            delay(100);
            return;
        }
    }

    // --- MENU STATE ---
    if (currentState == STATE_MENU) {
        if (lastUp == HIGH && curUp == LOW)     updateMenuSelection(menuSelection - 1);
        if (lastDown == HIGH && curDown == LOW) updateMenuSelection(menuSelection + 1);

        if (lastSel == HIGH && curSel == LOW) {
            currentState = (GameState)(menuSelection + 1);
            drawBoard();
        }
    }
    // --- CHESS BOARD STATE ---
    else {
        if (lastUp == HIGH && curUp == LOW) {
            int newY = constrain(selY - 1, 0, 7);
            drawSquare(selX, selY, false); selY = newY; drawSquare(selX, selY, true);
        }
        if (lastDown == HIGH && curDown == LOW) {
            int newY = constrain(selY + 1, 0, 7);
            drawSquare(selX, selY, false); selY = newY; drawSquare(selX, selY, true);
        }
        if (lastLeft == HIGH && curLeft == LOW) {
            int newX = constrain(selX - 1, 0, 7);
            drawSquare(selX, selY, false); selX = newX; drawSquare(selX, selY, true);
        }
        if (lastRight == HIGH && curRight == LOW) {
            int newX = constrain(selX + 1, 0, 7);
            drawSquare(selX, selY, false); selX = newX; drawSquare(selX, selY, true);
        }

        // Action 1: Select / Move (Click joystick stick down)
        if (lastSel == HIGH && curSel == LOW) {
            handleAction();
        }

        // Action 2: Cancel / Deselect
        if (lastOpt1 == HIGH && curOpt1 == LOW) {
            if (sourceX != -1 || sourceY != -1) {
                int oldX = sourceX, oldY = sourceY;
                sourceX = -1;
                sourceY = -1;
                drawSquare(oldX, oldY, false);
                drawSquare(selX, selY, true);
                Serial.println("Selection Canceled");
            }
        }

        // Track button hold duration on BTN_OPT2
        static unsigned long opt2PressTime = 0;
        static bool opt2Held = false;

        if (lastOpt2 == HIGH && curOpt2 == LOW) {
            opt2PressTime = millis();
            opt2Held = false;
        }

        // If held for > 1000ms: RESTART GAME
        if (curOpt2 == LOW && !opt2Held && (millis() - opt2PressTime > 1000)) {
            opt2Held = true;
            initBoard();
            sourceX = -1; sourceY = -1;
            drawBoard();
            Serial.println("Long press: Game Restarted!");
        }

        // Quick tap: FLIP BOARD
        if (lastOpt2 == LOW && curOpt2 == HIGH) {
            if (!opt2Held) {
                Serial.println("Short press: Flip Board triggered");
            }
        }
    }

    lastUp    = curUp;
    lastDown  = curDown;
    lastLeft  = curLeft;
    lastRight = curRight;
    lastSel   = curSel;
    lastOpt1  = curOpt1;
    lastBack  = curBack;
    lastOpt2  = curOpt2;
}