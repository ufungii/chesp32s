#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <Arduino.h>

enum class GameMode {
  PASS_AND_PLAY,
  VS_ENGINE,
  PEER_ESPNOW,
  ONLINE_LICHESS
};

extern GameMode currentGameMode;

extern char board[8][8];
extern bool isWhiteTurn; // Tracks active turn

// Castling rights
extern bool whiteKingMoved;
extern bool whiteRookLMoved; // a1 rook (Queenside)
extern bool whiteRookRMoved; // h1 rook (Kingside)
extern bool blackKingMoved;
extern bool blackRookLMoved; // a8 rook (Queenside)
extern bool blackRookRMoved; // h8 rook (Kingside)

// En Passant tracking (-1 if none available)
extern int epTargetX;
extern int epTargetY;

bool isWhitePiece(char piece);
bool isBlackPiece(char piece);
bool isSameTeam(char p1, char p2);
bool isValidMove(int srcX, int srcY, int destX, int destY);
bool isKingInCheck(bool isWhite);
void findKing(bool isWhite, int &kx, int &ky);
void initBoard();


#endif