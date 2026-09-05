#ifndef CHESS_AI_H
#define CHESS_AI_H

#include <Arduino.h>

struct Move {
  int fromX, fromY;
  int toX, toY;
  int score;
};

// Main AI invocation: computes best move for Black and executes it
void makeAIMove();

// Evaluation and Minimax search
int evaluateBoard();
int minimax(int depth, int alpha, int beta, bool isMaximizing);

#endif