#ifndef CHESS_AI_H
#define CHESS_AI_H

#include <Arduino.h>

struct Move {
  int fromX, fromY;
  int toX, toY;
  int score;
};

// Compute best move for Black
Move calculateBestAIMove(int depth = 2);

int evaluateBoard();
int minimax(int depth, int alpha, int beta, bool isMaximizing);

#endif