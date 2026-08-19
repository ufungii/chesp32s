#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Game States
enum GameState {
  STATE_MENU,
  STATE_GAME_LOCAL,
  STATE_GAME_ESP_NOW,
  STATE_GAME_LICHESS,
  STATE_GAME_AI
};

extern GameState currentState;
extern int menuSelection;

// Display Pins
#define TFT_CS    5
#define TFT_DC    2
#define TFT_RST  -1

// Board Layout
#define SQ_SIZE 26                  
#define OFFSET_X 70                 
#define OFFSET_Y 16                 

extern Adafruit_ILI9341 tft;
extern char board[8][8];
extern int selX, selY;
extern int sourceX, sourceY;

void initDisplay();
void drawHomeScreen();
void updateMenuSelection(int newSel);
void drawBoard();
void drawSquare(int col, int row, bool isSelected);
void handleAction();

#endif