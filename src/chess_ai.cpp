#include "chess_ai.h"
#include "game_logic.h"
#include "display.h"

// Piece-Square Tables (oriented for Black playing from top to bottom)
// Encourages knights & pawns to control center, king to castle, etc.
static const int pawnTable[8][8] = {
  { 0,  0,  0,  0,  0,  0,  0,  0},
  {50, 50, 50, 50, 50, 50, 50, 50},
  {10, 10, 20, 30, 30, 20, 10, 10},
  { 5,  5, 10, 25, 25, 10,  5,  5},
  { 0,  0,  0, 20, 20,  0,  0,  0},
  { 5, -5,-10,  0,  0,-10, -5,  5},
  { 5, 10, 10,-20,-20, 10, 10,  5},
  { 0,  0,  0,  0,  0,  0,  0,  0}
};

static const int knightTable[8][8] = {
  {-50,-40,-30,-30,-30,-30,-40,-50},
  {-40,-20,  0,  0,  0,  0,-20,-40},
  {-30,  0, 10, 15, 15, 10,  0,-30},
  {-30,  5, 15, 20, 20, 15,  5,-30},
  {-30,  0, 15, 20, 20, 15,  0,-30},
  {-30,  5, 10, 15, 15, 10,  5,-30},
  {-40,-20,  0,  5,  5,  0,-20,-40},
  {-50,-40,-30,-30,-30,-30,-40,-50}
};

static const int bishopTable[8][8] = {
  {-20,-10,-10,-10,-10,-10,-10,-20},
  {-10,  0,  0,  0,  0,  0,  0,-10},
  {-10,  0,  5, 10, 10,  5,  0,-10},
  {-10,  5,  5, 10, 10,  5,  5,-10},
  {-10,  0, 10, 10, 10, 10,  0,-10},
  {-10, 10, 10, 10, 10, 10, 10,-10},
  {-10,  5,  0,  0,  0,  0,  5,-10},
  {-20,-10,-10,-10,-10,-10,-10,-20}
};

static int getPieceValue(char piece, int x, int y) {
  if (piece == '.') return 0;
  int val = 0;
  char upper = toupper(piece);
  bool isWhite = isWhitePiece(piece);

  switch (upper) {
    case 'P': val = 100 + (isWhite ? pawnTable[7 - y][x] : pawnTable[y][x]); break;
    case 'N': val = 320 + (isWhite ? knightTable[7 - y][x] : knightTable[y][x]); break;
    case 'B': val = 330 + (isWhite ? bishopTable[7 - y][x] : bishopTable[y][x]); break;
    case 'R': val = 500; break;
    case 'Q': val = 900; break;
    case 'K': val = 20000; break;
  }

  return isWhite ? val : -val;
}

int evaluateBoard() {
  int totalScore = 0;
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      totalScore += getPieceValue(board[y][x], x, y);
    }
  }
  return totalScore; // Positive favors White, Negative favors Black
}

// Alpha-Beta Minimax search (Depth 3 is instant and strong for casual play)
int minimax(int depth, int alpha, int beta, bool isMaximizing) {
  if (depth == 0) {
    return evaluateBoard();
  }

  if (isMaximizing) { // White's turn
    int maxEval = -100000;
    for (int sy = 0; sy < 8; sy++) {
      for (int sx = 0; sx < 8; sx++) {
        if (!isWhitePiece(board[sy][sx])) continue;

        for (int dy = 0; dy < 8; dy++) {
          for (int dx = 0; dx < 8; dx++) {
            if (isValidMove(sx, sy, dx, dy)) {
              char src = board[sy][sx];
              char dst = board[dy][dx];

              board[dy][dx] = src;
              board[sy][sx] = '.';

              int eval = minimax(depth - 1, alpha, beta, false);

              board[sy][sx] = src;
              board[dy][dx] = dst;

              maxEval = max(maxEval, eval);
              alpha = max(alpha, eval);
              if (beta <= alpha) break; // Beta cutoff
            }
          }
        }
      }
    }
    return maxEval;
  } else { // Black's turn (Minimizing)
    int minEval = 100000;
    for (int sy = 0; sy < 8; sy++) {
      for (int sx = 0; sx < 8; sx++) {
        if (!isBlackPiece(board[sy][sx])) continue;

        for (int dy = 0; dy < 8; dy++) {
          for (int dx = 0; dx < 8; dx++) {
            if (isValidMove(sx, sy, dx, dy)) {
              char src = board[sy][sx];
              char dst = board[dy][dx];

              board[dy][dx] = src;
              board[sy][sx] = '.';

              int eval = minimax(depth - 1, alpha, beta, true);

              board[sy][sx] = src;
              board[dy][dx] = dst;

              minEval = min(minEval, eval);
              beta = min(beta, eval);
              if (beta <= alpha) break; // Alpha cutoff
            }
          }
        }
      }
    }
    return minEval;
  }
}

void makeAIMove() {
  Move bestMove = {-1, -1, -1, -1, 100000};
  int depth = 3; // 3 plies of lookahead

  // Find best move for Black
  for (int sy = 0; sy < 8; sy++) {
    for (int sx = 0; sx < 8; sx++) {
      if (!isBlackPiece(board[sy][sx])) continue;

      for (int dy = 0; dy < 8; dy++) {
        for (int dx = 0; dx < 8; dx++) {
          if (isValidMove(sx, sy, dx, dy)) {
            char src = board[sy][sx];
            char dst = board[dy][dx];

            board[dy][dx] = src;
            board[sy][sx] = '.';

            int score = minimax(depth - 1, -100000, 100000, true);

            board[sy][sx] = src;
            board[dy][dx] = dst;

            if (score < bestMove.score) {
              bestMove.score = score;
              bestMove.fromX = sx;
              bestMove.fromY = sy;
              bestMove.toX = dx;
              bestMove.toY = dy;
            }
          }
        }
      }
    }
  }

  // Execute Best Move if found
  if (bestMove.fromX != -1) {
    char piece = board[bestMove.fromY][bestMove.fromX];
    board[bestMove.toY][bestMove.toX] = piece;
    board[bestMove.fromY][bestMove.fromX] = '.';

    // Auto-promote Black pawn to Queen
    if (piece == 'p' && bestMove.toY == 7) {
      board[bestMove.toY][bestMove.toX] = 'q';
    }

    // Switch turn back to player
    isWhiteTurn = true;
    selX = bestMove.toX;
    selY = bestMove.toY;
    drawBoard();
  }
}