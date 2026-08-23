#include "inputs.h"
#include "display.h"
#include "game_logic.h"

XPT2046_Touchscreen ts(TOUCH_CS);

bool lastUp = HIGH, lastDown = HIGH, lastLeft = HIGH, lastRight = HIGH;
bool lastSel = HIGH, lastOpt1 = HIGH, lastBack = HIGH, lastOpt2 = HIGH;

unsigned long bootTime = 0;

void initInputs() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);
  pinMode(BTN_SEL, INPUT_PULLUP);
  pinMode(BTN_OPT1, INPUT_PULLUP);
  pinMode(BTN_BACK, INPUT_PULLUP);
  pinMode(BTN_OPT2, INPUT_PULLUP); // Note: GPIO 35 requires an external pull-up resistor if used

  ts.begin();
  ts.setRotation(1);

  bootTime = millis();
}

void processInputs() {
  if (millis() - bootTime < 300) {
    lastUp    = digitalRead(BTN_UP);
    lastDown  = digitalRead(BTN_DOWN);
    lastLeft  = digitalRead(BTN_LEFT);
    lastRight = digitalRead(BTN_RIGHT);
    lastSel   = digitalRead(BTN_SEL);
    lastOpt1  = digitalRead(BTN_OPT1);
    lastBack  = digitalRead(BTN_BACK);
    lastOpt2  = digitalRead(BTN_OPT2);
    return;
  }

  bool curUp    = digitalRead(BTN_UP);
  bool curDown  = digitalRead(BTN_DOWN);
  bool curLeft  = digitalRead(BTN_LEFT);
  bool curRight = digitalRead(BTN_RIGHT);
  bool curSel   = digitalRead(BTN_SEL);
  bool curOpt1  = digitalRead(BTN_OPT1);
  bool curBack  = digitalRead(BTN_BACK);
  bool curOpt2  = digitalRead(BTN_OPT2);

  // --- TOUCH INPUT PROCESSOR ---
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    if (p.z > MIN_PRESSURE) {
      int px = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
      int py = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

      if (currentState == STATE_MENU) {
        if (px >= 20 && px <= 300) {
          for (int i = 0; i < 4; i++) {
            int yPos = 65 + (i * 40);
            if (py >= yPos && py <= yPos + 32) {
              updateMenuSelection(i);
              currentState = (GameState)(i + 1);
              drawBoard();
              delay(150);
              break;
            }
          }
        }
      } else {
        if (px >= OFFSET_X && px < (OFFSET_X + 8 * SQ_SIZE) &&
            py >= OFFSET_Y && py < (OFFSET_Y + 8 * SQ_SIZE)) {
          int col = (px - OFFSET_X) / SQ_SIZE;
          int row = (py - OFFSET_Y) / SQ_SIZE;
          
          if (col != selX || row != selY) {
            drawSquare(selX, selY, false);
            selX = col;
            selY = row;
            drawSquare(selX, selY, true);
          }
          handleAction();
          delay(150);
        }
      }
    }
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

  // --- MENU STATE BUTTONS ---
  if (currentState == STATE_MENU) {
    if (lastUp == HIGH && curUp == LOW) updateMenuSelection(menuSelection - 1);
    if (lastDown == HIGH && curDown == LOW) updateMenuSelection(menuSelection + 1);
    
    if (lastSel == HIGH && curSel == LOW) {
      currentState = (GameState)(menuSelection + 1);
      drawBoard();
    }
  }
  // --- CHESS BOARD BUTTONS ---
  else {
    // D-Pad Movement
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

    // Action 1: Select / Move
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

 // Track button hold duration
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

// On release, if it was just a quick tap: FLIP BOARD
if (lastOpt2 == LOW && curOpt2 == HIGH) {
  if (!opt2Held) {
    // Call your flip board logic here
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
