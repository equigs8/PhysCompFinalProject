#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// ==============================================================================
// 1. PIN DEFINITIONS (ADJUST THESE FOR YOUR WIRING)
// ==============================================================================

// SPI Display Pins (Common configuration for ESP32)
#define TFT_CS   5  // Chip Select (CS)
#define TFT_DC   4  // Data/Command (DC)
#define TFT_RST -1 // Reset pin (or -1 if connected to ESP32's reset)
// The CLK (SCK) and MOSI (SDA) pins for SPI are fixed on the ESP32 (GPIO18 and GPIO23 by default).

// Input Button Pins (Assuming buttons are wired as Active-LOW: Press connects pin to GND)
#define PIN_UP     37
#define PIN_DOWN   48
#define PIN_LEFT   39
#define PIN_RIGHT  20
#define PIN_SELECT 21

// ==============================================================================
// 2. DISPLAY SETUP & GLOBAL VARIABLES
// ==============================================================================

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

#define BOARD_SIZE 3
#define EMPTY 0
#define PLAYER_X 1
#define PLAYER_O 2

int board[BOARD_SIZE][BOARD_SIZE];
int currentPlayer = PLAYER_X; // X starts
int gameStatus = 0; // 0: Running, 1: X Win, 2: O Win, 3: Draw

// Cursor position (0-2)
int cursorX = 0;
int cursorY = 0;

// Color Definitions
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CURSOR_COLOR 0x69E0 // Light green/cyan for cursor outline

// ==============================================================================
// 3. DRAWING FUNCTIONS
// ==============================================================================

/**
 * @brief Draws the initial board and title.
 */
void drawBoard() {
  tft.fillScreen(BLACK);

  // Board dimensions setup
  int startX = 20;
  int startY = 50;
  int cellSize = 60;
  int boardDim = cellSize * BOARD_SIZE;

  // Draw grid lines
  for (int i = 1; i < BOARD_SIZE; i++) {
    // Vertical lines
    tft.fillRect(startX + i * cellSize - 1, startY, 3, boardDim, WHITE);
    // Horizontal lines
    tft.fillRect(startX, startY + i * cellSize - 1, boardDim, 3, WHITE);
  }

  // Draw Title
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(30, 10);
  tft.print("Tic-Tac-Toe Handheld");
}

/**
 * @brief Draws X or O marker in a cell.
 */
void drawMarker(int x, int y, int player) {
  int startX = 20;
  int startY = 50;
  int cellSize = 60;
  int centerX = startX + x * cellSize + cellSize / 2;
  int centerY = startY + y * cellSize + cellSize / 2;
  int size = cellSize / 3;

  if (player == PLAYER_X) {
    // Draw an 'X' (two crossing lines) using drawLine, since drawFastLine only works for horizontal/vertical lines.
    tft.drawLine(centerX - size, centerY - size, centerX + size, centerY + size, RED);
    tft.drawLine(centerX - size, centerY + size, centerX + size, centerY - size, RED);
    // Draw again slightly offset for a thicker line
    tft.drawLine(centerX - size + 1, centerY - size, centerX + size + 1, centerY + size, RED);
    tft.drawLine(centerX - size + 1, centerY + size, centerX + size + 1, centerY - size, RED);
  } else if (player == PLAYER_O) {
    // Draw an 'O' (two concentric circles for thickness)
    tft.drawCircle(centerX, centerY, size, BLUE);
    tft.drawCircle(centerX, centerY, size + 1, BLUE);
  }
}

/**
 * @brief Draws or erases the cursor outline around the current cell.
 */
void drawCursor(int prevX, int prevY, int newX, int newY) {
  int startX = 20;
  int startY = 50;
  int cellSize = 60;
  int margin = 5;

  // 1. Erase previous cursor position
  // The border is set to BLACK to clear it.
  tft.drawRect(startX + prevX * cellSize + margin,
               startY + prevY * cellSize + margin,
               cellSize - 2 * margin, cellSize - 2 * margin, BLACK);


  // 2. Draw new cursor position only if the game is running AND the spot is empty
  if (gameStatus == 0 && board[newX][newY] == EMPTY) {
    tft.drawRect(startX + newX * cellSize + margin,
                 startY + newY * cellSize + margin,
                 cellSize - 2 * margin, cellSize - 2 * margin, CURSOR_COLOR);
  }
}

/**
 * @brief Displays game status messages at the bottom of the screen.
 */
void displayStatus(const char* message, uint16_t color) {
  tft.setTextSize(2);
  tft.setTextColor(color, BLACK);
  // Clear previous message area
  tft.fillRect(10, 250, tft.width() - 20, 30, BLACK);
  tft.setCursor(10, 250);
  tft.print(message);
}

// ==============================================================================
// 4. GAME LOGIC FUNCTIONS
// ==============================================================================

/**
 * @brief Resets the board, state, and redraws the display for a new game.
 */
void resetGame() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = EMPTY;
    }
  }
  currentPlayer = PLAYER_X;
  gameStatus = 0;
  cursorX = 0;
  cursorY = 0;

  drawBoard();
  drawCursor(0, 0, 0, 0); // Draw initial cursor at 0,0
  displayStatus("X's Turn", YELLOW);
}

/**
 * @brief Checks if the given player has won.
 * @return True if the player has won, false otherwise.
 */
bool checkWin(int player) {
  // Check rows and columns
  for (int i = 0; i < BOARD_SIZE; i++) {
    if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) || // Check row i
        (board[0][i] == player && board[1][i] == player && board[2][i] == player)) { // Check col i
      return true;
    }
  }

  // Check diagonals
  if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
      (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {
    return true;
  }

  return false;
}

/**
 * @brief Checks if the game has ended in a draw.
 * @return True if all spots are filled, false otherwise.
 */
bool checkDraw() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (board[i][j] == EMPTY) {
        return false;
      }
    }
  }
  return true;
}

/**
 * @brief Reads physical button inputs and updates the game state.
 */
void handleInput() {
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 150; // Throttle delay for cursor movement

  // If the game is over, only the SELECT button can restart it.
  if (gameStatus != 0) {
    if (digitalRead(PIN_SELECT) == LOW) {
      resetGame();
      delay(300); // Simple debounce for restart
    }
    return;
  }

  int prevX = cursorX;
  int prevY = cursorY;
  unsigned long currentTime = millis();

  // Handle Movement (Directional Buttons)
  if (currentTime - lastMoveTime >= moveDelay) {
    bool moved = false;

    if (digitalRead(PIN_UP) == LOW) {
      cursorY = max(0, cursorY - 1);
      moved = true;
    } else if (digitalRead(PIN_DOWN) == LOW) {
      cursorY = min(BOARD_SIZE - 1, cursorY + 1);
      moved = true;
    } else if (digitalRead(PIN_LEFT) == LOW) {
      cursorX = max(0, cursorX - 1);
      moved = true;
    } else if (digitalRead(PIN_RIGHT) == LOW) {
      cursorX = min(BOARD_SIZE - 1, cursorX + 1);
      moved = true;
    }

    if (moved) {
      lastMoveTime = currentTime;
      drawCursor(prevX, prevY, cursorX, cursorY);
      return; // Stop processing to prevent placing a marker immediately after moving
    }
  }

  // Handle Action (SELECT Button)
  if (digitalRead(PIN_SELECT) == LOW) {
    // Only place a marker if the current cell is empty
    if (board[cursorX][cursorY] == EMPTY) {
      // 1. Place marker
      board[cursorX][cursorY] = currentPlayer;
      drawMarker(cursorX, cursorY, currentPlayer);
      drawCursor(cursorX, cursorY, cursorX, cursorY); // This call erases the cursor

      // 2. Check game status
      if (checkWin(currentPlayer)) {
        gameStatus = currentPlayer;
        if (currentPlayer == PLAYER_X) {
          displayStatus("X WINS! (Select to Reset)", RED);
        } else {
          displayStatus("O WINS! (Select to Reset)", BLUE);
        }
      } else if (checkDraw()) {
        gameStatus = 3;
        displayStatus("DRAW! (Select to Reset)", YELLOW);
      } else {
        // 3. Switch turn
        currentPlayer = (currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;
        if (currentPlayer == PLAYER_X) {
          displayStatus("X's Turn", YELLOW);
        } else {
          displayStatus("O's Turn", YELLOW);
        }
      }

      // Debounce the select press
      delay(300);
    }
  }
}

// ==============================================================================
// 5. ARDUINO SETUP & LOOP
// ==============================================================================

void setup() {
  Serial.begin(115200);

  // Initialize display
  tft.begin();
  // Most ILI9341 screens are 240x320. Rotation(1) makes it 320x240 (landscape)
  tft.setRotation(1);

  // Initialize buttons with internal pull-up resistors
  // This means the button will be HIGH (unpressed) and LOW (pressed) when connected to GND.
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);

  // Start the game
  resetGame();
}

void loop() {
  // Check for button presses and update game state
  handleInput();
  delay(10);
}