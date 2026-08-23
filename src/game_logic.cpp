#include "game_logic.h"
#include "display.h"

bool isWhiteTurn = true;

bool whiteKingMoved = false;
bool whiteRookLMoved = false;
bool whiteRookRMoved = false;
bool blackKingMoved = false;
bool blackRookLMoved = false;
bool blackRookRMoved = false;

int epTargetX = -1;
int epTargetY = -1;

void initBoard() {
  const char startingBoard[8][8] = {
    {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'},
    {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'.', '.', '.', '.', '.', '.', '.', '.'},
    {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
    {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'}
  };

  memcpy(board, startingBoard, sizeof(board));
  isWhiteTurn = true;

  whiteKingMoved = false;
  whiteRookLMoved = false;
  whiteRookRMoved = false;
  blackKingMoved = false;
  blackRookLMoved = false;
  blackRookRMoved = false;
  epTargetX = -1;
  epTargetY = -1;
}

bool isWhitePiece(char piece) {
  return piece >= 'A' && piece <= 'Z';
}

bool isBlackPiece(char piece) {
  return piece >= 'a' && piece <= 'z';
}

bool isSameTeam(char p1, char p2) {
  if (p1 == '.' || p2 == '.') return false;
  return (isWhitePiece(p1) && isWhitePiece(p2)) || (isBlackPiece(p1) && isBlackPiece(p2));
}

void findKing(bool isWhite, int &kx, int &ky) {
  char target = isWhite ? 'K' : 'k';
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (board[y][x] == target) {
        kx = x;
        ky = y;
        return;
      }
    }
  }
}

// Checks if a specific coordinate is targeted by any enemy piece
bool isSquareAttacked(int x, int y, bool attackedByWhite) {
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      char p = board[row][col];
      if (p == '.') continue;
      
      bool pieceIsWhite = isWhitePiece(p);
      if (pieceIsWhite == attackedByWhite) {
        // Evaluate attack line using pure geometric move checks
        int dx = abs(x - col);
        int dy = abs(y - row);
        char pUpper = toupper(p);

        if (pUpper == 'P') {
          int dir = pieceIsWhite ? -1 : 1;
          if (dx == 1 && (y - row) == dir) return true;
        } else if (pUpper == 'N') {
          if ((dx == 1 && dy == 2) || (dx == 2 && dy == 1)) return true;
        } else if (pUpper == 'K') {
          if (dx <= 1 && dy <= 1) return true;
        } else if (pUpper == 'R' || pUpper == 'B' || pUpper == 'Q') {
          if (isValidMove(col, row, x, y)) return true;
        }
      }
    }
  }
  return false;
}

bool isKingInCheck(bool isWhite) {
  int kx = -1, ky = -1;
  findKing(isWhite, kx, ky);
  if (kx == -1) return false;
  return isSquareAttacked(kx, ky, !isWhite);
}

bool isValidMove(int srcX, int srcY, int destX, int destY) {
  char piece = board[srcY][srcX];
  char target = board[destY][destX];

  if (piece == '.') return false;
  if (isSameTeam(piece, target)) return false;

  int dx = abs(destX - srcX);
  int dy = abs(destY - srcY);
  char pUpper = toupper(piece);

  switch (pUpper) {
    case 'P': { // PAWN
      int direction = isWhitePiece(piece) ? -1 : 1;
      int startRow = isWhitePiece(piece) ? 6 : 1;

      // 1-step forward
      if (dx == 0 && destY - srcY == direction && target == '.') return true;
      // 2-step forward from starting rank
      if (dx == 0 && srcY == startRow && destY - srcY == 2 * direction && target == '.' && board[srcY + direction][srcX] == '.') return true;
      // Diagonal standard capture
      if (dx == 1 && destY - srcY == direction && target != '.' && !isSameTeam(piece, target)) return true;
      // En Passant capture
      if (dx == 1 && destY - srcY == direction && target == '.' && destX == epTargetX && destY == epTargetY) return true;

      return false;
    }

    case 'N': // KNIGHT
      return (dx == 1 && dy == 2) || (dx == 2 && dy == 1);

    case 'K': { // KING
      // Standard 1-step movement
      if (dx <= 1 && dy <= 1) return true;

      // Castling
      bool isWhite = isWhitePiece(piece);
      int rank = isWhite ? 7 : 0;

      if (srcY == rank && destY == rank && srcX == 4) {
        // Cannot castle out of check
        if (isKingInCheck(isWhite)) return false;

        // Kingside (O-O) -> King to (6, rank)
        if (destX == 6) {
          bool rookMoved = isWhite ? whiteRookRMoved : blackRookRMoved;
          bool kingMoved = isWhite ? whiteKingMoved : blackKingMoved;
          char rookPiece = board[rank][7];

          if (!kingMoved && !rookMoved && toupper(rookPiece) == 'R') {
            if (board[rank][5] == '.' && board[rank][6] == '.') {
              if (!isSquareAttacked(5, rank, !isWhite) && !isSquareAttacked(6, rank, !isWhite)) {
                return true;
              }
            }
          }
        }

        // Queenside (O-O-O) -> King to (2, rank)
        if (destX == 2) {
          bool rookMoved = isWhite ? whiteRookLMoved : blackRookLMoved;
          bool kingMoved = isWhite ? whiteKingMoved : blackKingMoved;
          char rookPiece = board[rank][0];

          if (!kingMoved && !rookMoved && toupper(rookPiece) == 'R') {
            if (board[rank][1] == '.' && board[rank][2] == '.' && board[rank][3] == '.') {
              if (!isSquareAttacked(3, rank, !isWhite) && !isSquareAttacked(2, rank, !isWhite)) {
                return true;
              }
            }
          }
        }
      }
      return false;
    }

    case 'R': { // ROOK
      if (dx != 0 && dy != 0) return false;
      if (dx == 0) {
        int step = (destY > srcY) ? 1 : -1;
        for (int y = srcY + step; y != destY; y += step) {
          if (board[y][srcX] != '.') return false;
        }
      } else {
        int step = (destX > srcX) ? 1 : -1;
        for (int x = srcX + step; x != destX; x += step) {
          if (board[srcY][x] != '.') return false;
        }
      }
      return true;
    }

    case 'B': { // BISHOP
      if (dx != dy) return false;
      int stepX = (destX > srcX) ? 1 : -1;
      int stepY = (destY > srcY) ? 1 : -1;
      int x = srcX + stepX;
      int y = srcY + stepY;
      while (x != destX && y != destY) {
        if (board[y][x] != '.') return false;
        x += stepX;
        y += stepY;
      }
      return true;
    }

    case 'Q': { // QUEEN
      if (dx == dy || dx == 0 || dy == 0) {
        int stepX = (dx == 0) ? 0 : ((destX > srcX) ? 1 : -1);
        int stepY = (dy == 0) ? 0 : ((destY > srcY) ? 1 : -1);
        int x = srcX + stepX;
        int y = srcY + stepY;
        while (x != destX || y != destY) {
          if (board[y][x] != '.') return false;
          x += stepX;
          y += stepY;
        }
        return true;
      }
      return false;
    }

    default:
      return false;
  }
}