#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   5
#define TFT_DC   15
#define TFT_RST  -1  // Tied to 3.3V

// Color Palette
#define COLOR_BG          0x0821 // Midnight Navy/Black
#define COLOR_SQ_LIGHT    0xDEFB // Crisp Silver/White
#define COLOR_SQ_DARK     0x4208 // Slate Grey
#define COLOR_CURSOR      0xFFE0 // Crisp Bright Yellow
#define COLOR_SELECT      0x07E0 // Bright Emerald Green
#define COLOR_PIECE_WHITE 0xFFFF // White
#define COLOR_PIECE_BLACK 0x1082 // Dark Onyx

#define OFFSET_X 12
#define OFFSET_Y 12
#define SQ_SIZE  27

enum GameState {
  STATE_MENU,
  STATE_CHESS_LOCAL,
  STATE_CHESS_AI,
  STATE_CHESS_WIRELESS
};

extern GameState currentState;
extern int menuSelection;
extern int selX, selY;
extern int sourceX, sourceY;
extern bool autoFlipPerspective; // Toggle board flip on turn

extern Adafruit_ILI9341 tft;

void initDisplay();
void drawHomeScreen();
void updateMenuSelection(int newIndex);
void drawBoard();
void drawSquare(int x, int y, bool isCursor);
void handleAction();
char showPromotionDialog(bool isWhite);
void drawBatteryIndicator(int pct);

#endif