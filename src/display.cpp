#include "display.h"
#include "game_logic.h"
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

GameState currentState = STATE_MENU;
int menuSelection = 0; // 0: Pass&Play, 1: AI, 2: ESP-NOW, 3: Lichess

int selX = 0;
int selY = 6;
int sourceX = -1;
int sourceY = -1;

char board[8][8] = {
  {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
  {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
  {'.', '.', '.', '.', '.', '.', '.', '.'},
  {'.', '.', '.', '.', '.', '.', '.', '.'},
  {'.', '.', '.', '.', '.', '.', '.', '.'},
  {'.', '.', '.', '.', '.', '.', '.', '.'},
  {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
  {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
};

const char* menuOptions[] = {
  "1. Pass & Play (Local 2P)",
  "2. Player vs. Local AI",
  "3. ESP-NOW Peer Match",
  "4. Lichess Online"
};

void initDisplay() {
  tft.begin(24000000);
  tft.setRotation(1);
  drawHomeScreen();
}

void drawHomeScreen() {
  tft.fillScreen(ILI9341_NAVY);
  
  // Title Header
  tft.fillRect(0, 0, 320, 45, ILI9341_DARKCYAN);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(55, 12);
  tft.print("ESP32 CHESS SYSTEM");

  // Options
  for (int i = 0; i < 4; i++) {
    updateMenuSelection(menuSelection);
  }
}

void updateMenuSelection(int newSel) {
  menuSelection = constrain(newSel, 0, 3);

  for (int i = 0; i < 4; i++) {
    int yPos = 65 + (i * 40);
    if (i == menuSelection) {
      tft.fillRect(20, yPos, 280, 32, ILI9341_YELLOW);
      tft.setTextColor(ILI9341_BLACK);
    } else {
      tft.fillRect(20, yPos, 280, 32, ILI9341_BLACK);
      tft.drawRect(20, yPos, 280, 32, ILI9341_WHITE);
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.setTextSize(2);
    tft.setCursor(30, yPos + 8);
    tft.print(menuOptions[i]);
  }
}

void drawSquare(int col, int row, bool isSelected) {
  int x = OFFSET_X + (col * SQ_SIZE);
  int y = OFFSET_Y + (row * SQ_SIZE);

  uint16_t bgColor = ((col + row) % 2 == 0) ? ILI9341_LIGHTGREY : ILI9341_DARKGREY;
  tft.fillRect(x, y, SQ_SIZE, SQ_SIZE, bgColor);

  if (col == sourceX && row == sourceY) {
    tft.drawRect(x, y, SQ_SIZE, SQ_SIZE, ILI9341_GREEN);
    tft.drawRect(x + 1, y + 1, SQ_SIZE - 2, SQ_SIZE - 2, ILI9341_GREEN);
  } else if (isSelected) {
    tft.drawRect(x, y, SQ_SIZE, SQ_SIZE, ILI9341_YELLOW);
    tft.drawRect(x + 1, y + 1, SQ_SIZE - 2, SQ_SIZE - 2, ILI9341_YELLOW);
  } else {
    tft.drawRect(x, y, SQ_SIZE, SQ_SIZE, ILI9341_BLACK);
  }

  char piece = board[row][col];
  if (piece != '.') {
    tft.setTextSize(2);
    tft.setTextColor(isupper(piece) ? ILI9341_WHITE : ILI9341_RED);
    tft.setCursor(x + 7, y + 5);
    tft.print((char)toupper(piece));
  }
}

void drawLabels() {
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  for (int c = 0; c < 8; c++) {
    tft.setCursor(OFFSET_X + (c * SQ_SIZE) + (SQ_SIZE / 2) - 3, OFFSET_Y - 12);
    tft.print((char)('A' + c));
  }
  for (int r = 0; r < 8; r++) {
    tft.setCursor(OFFSET_X - 15, OFFSET_Y + (r * SQ_SIZE) + (SQ_SIZE / 2) - 4);
    tft.print(8 - r);
  }
}

void drawBoard() {
  tft.fillScreen(ILI9341_NAVY);
  drawLabels();
  for (int r = 0; r < 8; r++) {
    for (int c = 0; c < 8; c++) {
      drawSquare(c, r, (c == selX && r == selY));
    }
  }
}

void handleAction() {
  if (sourceX == -1 && sourceY == -1) {
    char piece = board[selY][selX];
    if (piece == '.') return;

    // Only allow selecting current player's pieces
    if ((isWhiteTurn && isupper(piece)) || (!isWhiteTurn && islower(piece))) {
      sourceX = selX;
      sourceY = selY;
      drawSquare(sourceX, sourceY, true);
    }
  } else {
    // Deselect if tapping same piece
    if (selX == sourceX && selY == sourceY) {
      int oldX = sourceX, oldY = sourceY;
      sourceX = -1; sourceY = -1;
      drawSquare(oldX, oldY, true);
      return;
    }

    if (isValidMove(sourceX, sourceY, selX, selY)) {
      // Temporary move simulation for check validation
      char tempSource = board[sourceY][sourceX];
      char tempTarget = board[selY][selX];

      board[selY][selX] = tempSource;
      board[sourceY][sourceX] = '.';

      // Reject move if it leaves or puts own King in check
      if (isKingInCheck(isWhiteTurn)) {
        board[sourceY][sourceX] = tempSource; // Revert
        board[selY][selX] = tempTarget;
        Serial.println("Illegal Move: King is in check!");
        return;
      }

      // Move is legal! Finalize and switch turns
      int oldX = sourceX, oldY = sourceY;
      sourceX = -1; sourceY = -1;

      isWhiteTurn = !isWhiteTurn; // Swap turns

      drawSquare(oldX, oldY, false);
      drawSquare(selX, selY, true);
    }
  }
}