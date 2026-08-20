#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include <Arduino.h>

bool isWhitePiece(char piece);
bool isBlackPiece(char piece);
bool isSameTeam(char p1, char p2);
bool isValidMove(int srcX, int srcY, int destX, int destY);

#endif