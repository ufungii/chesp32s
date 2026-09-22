#include <Arduino.h>
#include "display.h"
#include "inputs.h"
#include "game_logic.h"
#include "chess_ai.h"

unsigned long lastBatUpdate = 0;

// Inter-core queues
QueueHandle_t qAIRequest;
QueueHandle_t qAIResponse;
bool isAIThinking = false;

// Task running exclusively on Core 0 for background Minimax calculation
void vAITask(void *pvParameters) {
  int dummy;
  for (;;) {
    if (xQueueReceive(qAIRequest, &dummy, portMAX_DELAY) == pdTRUE) {
      // Depth 2 runs fast (<200ms) without triggering the ESP32 watchdog
      Move best = calculateBestAIMove(2);
      xQueueSend(qAIResponse, &best, portMAX_DELAY);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Booting ESP32 Chess Console...");

  initBoard();
  initDisplay();
  initInputs();

  // Set default mode to VS_ENGINE for testing
  currentGameMode = GameMode::VS_ENGINE;

  // Create thread-safe message queues
  qAIRequest = xQueueCreate(1, sizeof(int));
  qAIResponse = xQueueCreate(1, sizeof(Move));

  // Launch Engine worker on Core 0 (Core 1 handles display & inputs)
  xTaskCreatePinnedToCore(
    vAITask,
    "AITask",
    8192,
    NULL,
    1,
    NULL,
    0
  );

  drawHomeScreen();
  Serial.println("System Ready! Enjoy the game.");
}

void loop() {
  // 1. If in VS_ENGINE mode and it's Black's turn, trigger Core 0
  if (currentGameMode == GameMode::VS_ENGINE && !isWhiteTurn && !isAIThinking) {
    isAIThinking = true;
    int trigger = 1;
    xQueueSend(qAIRequest, &trigger, 0);
  }

  // 2. Poll for the engine's move without blocking UI
  if (isAIThinking) {
    Move aiMove;
    if (xQueueReceive(qAIResponse, &aiMove, 0) == pdTRUE) {
      if (aiMove.fromX != -1) {
        char piece = board[aiMove.fromY][aiMove.fromX];
        board[aiMove.toY][aiMove.toX] = piece;
        board[aiMove.fromY][aiMove.fromX] = '.';

        // Auto-promote Black pawn to Queen
        if (piece == 'p' && aiMove.toY == 7) {
          board[aiMove.toY][aiMove.toX] = 'q';
        }

        isWhiteTurn = true;
        drawBoard();
      }
      isAIThinking = false;
    }
  } else {
    // 3. Handle player joystick and button inputs
    processInputs();
  }

  // Update battery percentage every 5 seconds
  if (millis() - lastBatUpdate > 5000) {
    lastBatUpdate = millis();
    int pct = getBatteryPercentage();
    drawBatteryIndicator(pct);
  }
}