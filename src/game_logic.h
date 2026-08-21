#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <Arduino.h>

extern char board[8][8];
extern bool isWhiteTurn; // Tracks active turn

bool isWhitePiece(char piece);
bool isBlackPiece(char piece);
bool isSameTeam(char p1, char p2);
bool isValidMove(int srcX, int srcY, int destX, int destY);
bool isKingInCheck(bool isWhite);
void findKing(bool isWhite, int &kx, int &ky);
void initBoard();

#endif