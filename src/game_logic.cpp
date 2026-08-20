#include "game_logic.h"
#include "display.h"

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

bool isValidMove(int srcX, int srcY, int destX, int destY) {
  char piece = board[srcY][srcX];
  char target = board[destY][destX];

  // 1. Can't capture your own piece
  if (isSameTeam(piece, target)) return false;

  int dx = abs(destX - srcX);
  int dy = abs(destY - srcY);
  char pUpper = toupper(piece);

  switch (pUpper) {
    case 'P': { // PAWN
      int direction = isWhitePiece(piece) ? -1 : 1; // White moves UP (-Y), Black moves DOWN (+Y)
      int startRow = isWhitePiece(piece) ? 6 : 1;

      // Single step forward (must be empty square)
      if (dx == 0 && destY - srcY == direction && target == '.') return true;
      // Double step forward from starting rank
      if (dx == 0 && srcY == startRow && destY - srcY == 2 * direction && target == '.' && board[srcY + direction][srcX] == '.') return true;
      // Capture diagonally
      if (dx == 1 && destY - srcY == direction && target != '.' && !isSameTeam(piece, target)) return true;

      return false;
    }

    case 'N': // KNIGHT (L-Shape)
      return (dx == 1 && dy == 2) || (dx == 2 && dy == 1);

    case 'K': // KING (1 square in any direction)
      return dx <= 1 && dy <= 1;

    case 'R': // ROOK (Straight lines - check path clearing)
      if (dx != 0 && dy != 0) return false; // Must be straight line
      // Path obstruction check
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

    case 'B': // BISHOP (Diagonals)
      if (dx != dy) return false;
      {
        int stepX = (destX > srcX) ? 1 : -1;
        int stepY = (destY > srcY) ? 1 : -1;
        int x = srcX + stepX;
        int y = srcY + stepY;
        while (x != destX && y != destY) {
          if (board[y][x] != '.') return false;
          x += stepX;
          y += stepY;
        }
      }
      return true;

    case 'Q': // QUEEN (Rook + Bishop rules combined)
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

    default:
      return false;
  }
}