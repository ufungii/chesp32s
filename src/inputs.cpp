#include "inputs.h"
#include "display.h"

XPT2046_Touchscreen ts(TOUCH_CS);

bool lastUp = HIGH, lastDown = HIGH, lastLeft = HIGH, lastRight = HIGH, lastSel = HIGH;
bool lastBack = HIGH, lastOpt1 = HIGH, lastOpt2 = HIGH;

unsigned long bootTime = 0;

void initInputs() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);

  pinMode(BTN_OPT1, INPUT_PULLUP);
  pinMode(BTN_OPT2, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);

  ts.begin();
  ts.setRotation(1);

  bootTime = millis(); // Track startup time
}

void processInputs() {
  // Ignore button triggers for first 300ms to clear boot noise
  if (millis() - bootTime < 300) {
    lastUp = digitalRead(BTN_UP);
    lastDown = digitalRead(BTN_DOWN);
    lastLeft = digitalRead(BTN_LEFT);
    lastRight = digitalRead(BTN_RIGHT);
    lastSel = digitalRead(BTN_SEL);
    lastBack = digitalRead(BTN_BACK);
    lastOpt1 = digitalRead(BTN_OPT1);
    lastOpt2 = digitalRead(BTN_OPT2);
    return;
  }

  bool curUp    = digitalRead(BTN_UP);
  bool curDown  = digitalRead(BTN_DOWN);
  bool curLeft  = digitalRead(BTN_LEFT);
  bool curRight = digitalRead(BTN_RIGHT);
  bool curSel   = digitalRead(BTN_SEL);
  bool curBack  = digitalRead(BTN_BACK);
  bool curOpt1  = digitalRead(BTN_OPT1);
  bool curOpt2  = digitalRead(BTN_OPT2);

  // --- GLOBAL KEY: BACK BUTTON (Exits to Menu from anywhere) ---
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
    if (lastUp == HIGH && curUp == LOW) updateMenuSelection(menuSelection - 1);
    if (lastDown == HIGH && curDown == LOW) updateMenuSelection(menuSelection + 1);
    
    if (lastSel == HIGH && curSel == LOW) {
      if (menuSelection == 0) currentState = STATE_GAME_LOCAL;
      else if (menuSelection == 1) currentState = STATE_GAME_AI;
      else if (menuSelection == 2) currentState = STATE_GAME_ESP_NOW;
      else if (menuSelection == 3) currentState = STATE_GAME_LICHESS;

      drawBoard(); // Transition to board screen
    }
  }
  // --- CHESS GAME STATE ---
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
      int newX = constrain(selX - 1, 0, 7);
      drawSquare(selX, selY, false); selX = newX; drawSquare(selX, selY, true);
    }
    if (lastSel == HIGH && curSel == LOW) {
      handleAction();
    }
    
    // OPT1 Key: Reset board / Cancel piece pickup
    if (lastOpt1 == HIGH && curOpt1 == LOW) {
      if (sourceX != -1 || sourceY != -1) {
        int oldX = sourceX, oldY = sourceY;
        sourceX = -1; sourceY = -1;
        drawSquare(oldX, oldY, false);
        drawSquare(selX, selY, true);
        Serial.println("Selection Canceled");
      }
    }
  }

  lastUp    = curUp;
  lastDown  = curDown;
  lastLeft  = curLeft;
  lastRight = curRight;
  lastSel   = curSel;
  lastBack  = curBack;
  lastOpt1  = curOpt1;
  lastOpt2  = curOpt2;
}