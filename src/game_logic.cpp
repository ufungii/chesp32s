#include "game_logic.h"
#include "display.h"

bool isWhiteTurn = true; // White moves first

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

// Locate the King on the board
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

// Check if any enemy piece can attack the King's square
bool isKingInCheck(bool isWhite) {
  int kx = -1, ky = -1;
  findKing(isWhite, kx, ky);
  if (kx == -1) return false;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      char p = board[y][x];
      if (p == '.') continue;
      
      // If it's an enemy piece, check if it has a legal attack line to the King
      bool enemyPiece = isWhite ? isBlackPiece(p) : isWhitePiece(p);
      if (enemyPiece && isValidMove(x, y, kx, ky)) {
        return true;
      }
    }
  }
  return false;
}

// Validates geometric/board legality of a move (ignoring active turn state)
bool isValidMove(int srcX, int srcY, int destX, int destY) {
  char piece = board[srcY][srcX];
  char target = board[destY][destX];

  if (piece == '.') return false;

  // Cannot capture your own piece
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