#include "display.h"
#include "game_logic.h"
#include "inputs.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

GameState currentState = STATE_MENU;
int menuSelection = 0;

int selX = 4;
int selY = 6;
int sourceX = -1;
int sourceY = -1;
bool autoFlipPerspective = true; // Flips board view automatically on turn

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

void initDisplay() {
  tft.begin(20000000);
  delay(50);
  tft.setRotation(1);
}

// -------------------------------------------------------------
// BATTERYYYY
// -------------------------------------------------------------
void drawBatteryIndicator(int pct) {
  tft.setTextSize(1);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.setCursor(tft.width() - 45, 4);
  tft.printf("%3d%%", pct); // prints the raw count (e.g. 2350)
}

// -------------------------------------------------------------
// MENU RENDERING (MODERN SLEEK UI)
// -------------------------------------------------------------
void drawHomeScreen() {
  tft.fillScreen(COLOR_BG);

  // Banner Header
  tft.fillRoundRect(12, 10, 296, 38, 6, 0x2124);
  tft.drawRoundRect(12, 10, 296, 38, 6, 0x4A69);
  tft.setTextColor(COLOR_PIECE_WHITE);
  tft.setTextSize(2);
  tft.setCursor(42, 22);
  tft.print("chess :)");

  const char* options[] = {
    "1. Pass & Play",
    "2. VS Computer",
    "3. Settings (Flip On)"
  };

  for (int i = 0; i < 3; i++) {
    int yPos = 60 + (i * 44);
    bool isSelected = (i == menuSelection);

    if (isSelected) {
      tft.fillRoundRect(20, yPos, 280, 36, 6, COLOR_SELECT);
      tft.setTextColor(0x0000);
    } else {
      tft.fillRoundRect(20, yPos, 280, 36, 6, 0x2965);
      tft.setTextColor(COLOR_PIECE_WHITE);
    }

    tft.setTextSize(2);
    tft.setCursor(35, yPos + 10);
    tft.print(options[i]);
  }
}

void updateMenuSelection(int newIndex) {
  if (newIndex < 0) newIndex = 2;
  if (newIndex > 2) newIndex = 0;
  if (newIndex == menuSelection) return;

  menuSelection = newIndex;
  drawHomeScreen();
}

// -------------------------------------------------------------
// BOARD COORDINATE MAPPING (AUTO-PERSPECTIVE)
// -------------------------------------------------------------
static void boardToScreenCoords(int bx, int by, int &sx, int &sy) {
  // If Black's turn and auto-flip is active, invert coordinates
  if (!isWhiteTurn && autoFlipPerspective) {
    sx = 7 - bx;
    sy = 7 - by;
  } else {
    sx = bx;
    sy = by;
  }
}

// -------------------------------------------------------------
// CHESS BOARD RENDERING
// -------------------------------------------------------------
void drawBoard() {
  tft.fillScreen(COLOR_BG);

  // Outer Border Box
  tft.drawRoundRect(OFFSET_X - 3, OFFSET_Y - 3, (8 * SQ_SIZE) + 6, (8 * SQ_SIZE) + 6, 4, 0x7BEF);

  // Draw 64 Squares
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      drawSquare(x, y, (x == selX && y == selY));
    }
  }

  // Side Panel Info
  int sideX = 240;
  tft.fillRoundRect(sideX, OFFSET_Y, 70, 80, 6, 0x2124);
  tft.drawRoundRect(sideX, OFFSET_Y, 70, 80, 6, 0x4A69);

  tft.setTextColor(0xAD55);
  tft.setTextSize(1);
  tft.setCursor(sideX + 18, OFFSET_Y + 12);
  tft.print("TURN");

  tft.setTextSize(2);
  tft.setCursor(sideX + 6, OFFSET_Y + 32);
  if (isWhiteTurn) {
    tft.setTextColor(COLOR_PIECE_WHITE);
    tft.print("WHITE");
  } else {
    tft.setTextColor(COLOR_CURSOR);
    tft.print("BLACK");
  }

  // View indicator
  tft.setTextSize(1);
  tft.setTextColor(0x7BEF);
  tft.setCursor(sideX + 10, OFFSET_Y + 60);
  tft.print(isWhiteTurn ? "(A-H 1-8)" : "(H-A 8-1)");
}

void drawSquare(int bx, int by, bool isCursor) {
  if (bx < 0 || bx > 7 || by < 0 || by > 7) return;

  int sx, sy;
  boardToScreenCoords(bx, by, sx, sy);

  int px = OFFSET_X + (sx * SQ_SIZE);
  int py = OFFSET_Y + (sy * SQ_SIZE);

  // Base Board Tile Color
  uint16_t baseColor = ((bx + by) % 2 == 0) ? COLOR_SQ_LIGHT : COLOR_SQ_DARK;
  tft.fillRect(px, py, SQ_SIZE, SQ_SIZE, baseColor);

  // Piece Selected Highlight (Cyan Double Border)
  if (bx == sourceX && by == sourceY) {
    tft.fillRect(px, py, SQ_SIZE, SQ_SIZE, 0x0310); // subtle tint under selected piece
    tft.drawRect(px, py, SQ_SIZE, SQ_SIZE, COLOR_SELECT);
    tft.drawRect(px + 1, py + 1, SQ_SIZE - 2, SQ_SIZE - 2, COLOR_SELECT);
  }

  // Active D-Pad Cursor (Amber / Orange Ring)
  if (isCursor) {
    tft.drawRect(px, py, SQ_SIZE, SQ_SIZE, COLOR_CURSOR);
    tft.drawRect(px + 1, py + 1, SQ_SIZE - 2, SQ_SIZE - 2, COLOR_CURSOR);
  }

  // Draw Piece Glyph
  char piece = board[by][bx];
  if (piece != '.') {
    bool isWhite = isWhitePiece(piece);
    tft.setTextSize(2);

    // Draw high-contrast outline for black pieces
    if (!isWhite) {
      tft.setTextColor(0x0000); // Black piece
      tft.setCursor(px + 8, py + 6);
      tft.print((char)toupper(piece));
    } else {
      tft.setTextColor(0xFFFF); // White piece
      tft.setCursor(px + 8, py + 6);
      tft.print((char)toupper(piece));
    }
  }
}

// -------------------------------------------------------------
// PAWN PROMOTION DIALOG
// -------------------------------------------------------------
char showPromotionDialog(bool isWhite) {
  int dialogX = 35;
  int dialogY = 80;
  int dialogW = 250;
  int dialogH = 80;
  int btnW = 50;
  int btnH = 50;

  tft.fillRoundRect(dialogX, dialogY, dialogW, dialogH, 8, 0x18C3);
  tft.drawRoundRect(dialogX, dialogY, dialogW, dialogH, 8, COLOR_PIECE_WHITE);

  char options[4] = {
    (char)(isWhite ? 'Q' : 'q'),
    (char)(isWhite ? 'R' : 'r'),
    (char)(isWhite ? 'B' : 'b'),
    (char)(isWhite ? 'N' : 'n')
  };

  const char* labels[4] = {"Q", "R", "B", "N"};

  for (int i = 0; i < 4; i++) {
    int bx = dialogX + 12 + (i * 58);
    int by = dialogY + 15;
    tft.fillRoundRect(bx, by, btnW, btnH, 4, 0x0013);
    tft.drawRoundRect(bx, by, btnW, btnH, 4, COLOR_SELECT);
    tft.setTextColor(COLOR_PIECE_WHITE);
    tft.setTextSize(3);
    tft.setCursor(bx + 16, by + 14);
    tft.print(labels[i]);
  }

  while (true) {
    if (digitalRead(BTN_SEL) == LOW) {
      delay(200);
      return options[0]; // Select Queen on SEL press
    }
    delay(20);
  }
}

// -------------------------------------------------------------
// ACTION / MOVE HANDLER
// -------------------------------------------------------------
void handleAction() {
  if (sourceX == -1 && sourceY == -1) {
    char piece = board[selY][selX];
    if (piece == '.') return;

    if ((isWhiteTurn && isWhitePiece(piece)) || (!isWhiteTurn && isBlackPiece(piece))) {
      sourceX = selX;
      sourceY = selY;
      drawSquare(sourceX, sourceY, true);
    }
  } else {
    // Deselect if selecting the same piece
    if (selX == sourceX && selY == sourceY) {
      int ox = sourceX, oy = sourceY;
      sourceX = -1;
      sourceY = -1;
      drawSquare(ox, oy, true);
      return;
    }

    if (isValidMove(sourceX, sourceY, selX, selY)) {
      char movingPiece = board[sourceY][sourceX];
      char targetPiece = board[selY][selX];
      int dx = abs(selX - sourceX);
      int dy = abs(selY - sourceY);

      // 1. Castling
      bool isCastleMove = (toupper(movingPiece) == 'K' && dx == 2);
      int rookSrcX = -1, rookDestX = -1, castleRank = sourceY;

      if (isCastleMove) {
        if (selX == 6) { rookSrcX = 7; rookDestX = 5; }
        else if (selX == 2) { rookSrcX = 0; rookDestX = 3; }
        board[castleRank][rookDestX] = board[castleRank][rookSrcX];
        board[castleRank][rookSrcX] = '.';
      }

      // 2. En Passant
      bool isEPMove = (toupper(movingPiece) == 'P' && dx == 1 && targetPiece == '.' && selX == epTargetX && selY == epTargetY);
      if (isEPMove) {
        board[sourceY][selX] = '.';
      }

      // 3. Move Piece
      board[selY][selX] = movingPiece;
      board[sourceY][sourceX] = '.';

      // 4. King Check Safety Check
      if (isKingInCheck(isWhiteTurn)) {
        board[sourceY][sourceX] = movingPiece;
        board[selY][selX] = targetPiece;
        if (isCastleMove) {
          board[castleRank][rookSrcX] = board[castleRank][rookDestX];
          board[castleRank][rookDestX] = '.';
        }
        if (isEPMove) {
          board[sourceY][selX] = isWhiteTurn ? 'p' : 'P';
        }
        return;
      }

      // 5. Pawn Promotion Check
      if (movingPiece == 'P' && selY == 0) {
        board[selY][selX] = showPromotionDialog(true);
      } else if (movingPiece == 'p' && selY == 7) {
        board[selY][selX] = showPromotionDialog(false);
      }

      // 6. Castling Rights Tracking
      if (movingPiece == 'K') whiteKingMoved = true;
      if (movingPiece == 'k') blackKingMoved = true;
      if (sourceY == 7 && sourceX == 0) whiteRookLMoved = true;
      if (sourceY == 7 && sourceX == 7) whiteRookRMoved = true;
      if (sourceY == 0 && sourceX == 0) blackRookLMoved = true;
      if (sourceY == 0 && sourceX == 7) blackRookRMoved = true;

      // 7. En Passant target
      if (toupper(movingPiece) == 'P' && dy == 2) {
        epTargetX = sourceX;
        epTargetY = (sourceY + selY) / 2;
      } else {
        epTargetX = -1;
        epTargetY = -1;
      }

      sourceX = -1;
      sourceY = -1;

      // Switch turn
      isWhiteTurn = !isWhiteTurn;

      // Reset cursor to the other player's side on flip
      if (autoFlipPerspective) {
        selX = 4;
        selY = isWhiteTurn ? 6 : 1;
      }

      // Redraw fresh board with perspective flip
      drawBoard();
    }
  }
}