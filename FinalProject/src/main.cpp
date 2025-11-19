#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// ==============================================================================
// 1. PIN DEFINITIONS (ADJUST THESE FOR YOUR WIRING)
// These pins reflect the latest configuration in the user-provided code.
// ==============================================================================

// SPI Display Pins (Common configuration for ESP32)
#define TFT_CS   5  // Chip Select (CS)
#define TFT_DC   4  // Data/Command (DC)
#define TFT_RST -1 // Reset pin (or -1 if connected to ESP32's reset)
// The CLK (SCK) and MOSI (SDA) pins for SPI are fixed on the ESP32 (GPIO18 and GPIO23 by default).

// Input Button Pins (Assuming buttons are wired as Active-LOW: Press connects pin to GND)
#define PIN_UP     0
#define PIN_DOWN   48
#define PIN_LEFT   39
#define PIN_RIGHT  20
#define PIN_SELECT 21
#define PIN_HOME   40
#define PIN_BUTTONA 15
#define PIN_BUTTONB 2

// ==============================================================================
// 2. DISPLAY SETUP & GLOBAL VARIABLES
// ==============================================================================

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// --- GAME STATE MANAGEMENT ---
enum GameState {
  STATE_MENU,
  STATE_TICTACTOE,
  STATE_POKEMON_BATTLER
};
GameState currentState = STATE_MENU;

// Menu Variables
int menuSelection = 0; // Index of the currently selected menu item
const char* menuItems[] = {"Tic-Tac-Toe", "Coming Soon..."};
const int numMenuItems = sizeof(menuItems) / sizeof(menuItems[0]);

// Color Definitions
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CURSOR_COLOR 0xF800 // Light green/cyan for cursor outline
#define MAIN_MENU_COLOR BLACK



// Icon Bitmap Data (8x8 pixels, 1-bit monochrome)
// This icon is a simple 3x3 grid for Tic-Tac-Toe
const uint8_t TIC_TAC_TOE_ICON_BITS[] PROGMEM = {
 0x24, 0x24, 0xff, 0x24, 0x24, 0xff, 0x24, 0x24
};


// Tic-Tac-Toe Variables
#define BOARD_SIZE 3
#define EMPTY 0
#define PLAYER_X 1
#define PLAYER_O 2

int board[BOARD_SIZE][BOARD_SIZE];
int currentPlayer = PLAYER_X; // X starts
int gameStatus = 0; // 0: Running, 1: X Win, 2: O Win, 3: Draw
bool gameOverScreenDrawn = false;


// Cursor position (0-2) for Tic-Tac-Toe
int cursorX = 0;
int cursorY = 0;



// Forward declaration needed for handleMenuInput
void resetTicTacToe();
void ticTacToeSelected();
void pokemonBattlerSelected();
void resetPokemonBattler();

// ==============================================================================
// 3. DRAWING FUNCTIONS
// ==============================================================================

/**
 * @brief Displays game status messages at the bottom of the screen.
 */
void displayStatus(const char* message, uint16_t color) {
  const int statusY = 220;
  tft.setTextSize(2);
  tft.setTextColor(color, BLACK);
  // Clear previous message area
  tft.fillRect(10, statusY, tft.width() - 20, 30, BLACK);
  tft.setCursor(10, statusY);
  tft.print(message);
}

/**
 * @brief Draws a simple icon next to a menu item using bitmaps or simple shapes.
 * @param itemIndex The index of the menu item (0 for Tic-Tac-Toe).
 * @param xPos The horizontal position to start drawing the icon.
 * @param yPos The vertical position (top edge) to start drawing the icon.
 * @param color The color to draw the icon.
 */
void drawMenuItemIcon(int itemIndex, int xPos, int yPos, uint16_t color) {
  int iconSize = 8;
  
  // Clear the icon area before drawing the icon to prevent color artifacts
  tft.fillRect(xPos + 3, yPos + 3, iconSize, iconSize, BLACK); 
  
  if (itemIndex == 0) {
    // Icon for Tic-Tac-Toe: Draw a 16x16 Bitmap
    // x, y, bitmap array, width, height, color
    tft.drawBitmap(xPos + 7, yPos + 7, TIC_TAC_TOE_ICON_BITS, iconSize, iconSize, RED);
    
  } else if (itemIndex == 1) {
    // Icon for "Coming Soon...": Placeholder box with '?'
    tft.drawRect(xPos + 3, yPos + 3, iconSize + 2, iconSize + 2, color);
    tft.setCursor(xPos + 5, yPos + 5);
    tft.setTextSize(1);
    tft.setTextColor(color);
    tft.print("?");
    tft.setTextSize(2); // Reset text size
  }
}


/**
 * @brief Draws the initial main menu screen.
 */
void drawMenu() {
  tft.fillScreen(MAIN_MENU_COLOR);
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.setCursor(50, 20);
  tft.print("HANDHELD MENU");

  tft.setTextSize(2);
  for (int i = 0; i < numMenuItems; i++) {
    int yPos = 80 + i * 40;

    // Draw Icon
    drawMenuItemIcon(i, 15, yPos, WHITE);

    // Draw Text
    tft.setCursor(50, 80 + i * 40);
    tft.setTextColor(WHITE);
    tft.print(menuItems[i]);
  }
}

/**
 * @brief Draws or moves the cursor/selection highlight on the menu.
 */
void drawMenuCursor(int prevSelection, int newSelection) {
  int xPos = 40;
  int yStart = 80;
  int height = 20;
  int width = tft.width() - 80;

  // 1. Erase previous cursor and redraw text/icon to un-highlight (WHITE).
  if (prevSelection >= 0 && prevSelection < numMenuItems) {
    int prevYPos = yStart + prevSelection * 40;
    
    // Erase the background highlight
    tft.fillRect(xPos, prevYPos, width, height, BLACK);
    
    // Redraw Text (WHITE)
    tft.setCursor(50, prevYPos);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print(menuItems[prevSelection]);
    
    // Redraw Icon (WHITE) (Icon position: x=5, centered vertically around text line)
    drawMenuItemIcon(prevSelection, 15, prevYPos, WHITE);
  }

  // 2. Draw new cursor highlight and redraw text/icon in BLACK.
  int newYPos = yStart + newSelection * 40;
  
  // Draw new cursor (filled box)
  tft.fillRect(xPos, newYPos, width, height, CURSOR_COLOR);
  
  // Redraw Text (BLACK on the colored cursor)
  tft.setCursor(50, newYPos);
  tft.setTextColor(BLACK); 
  tft.setTextSize(2);
  tft.print(menuItems[newSelection]);
  
  // Redraw Icon (BLACK on the colored cursor)
  drawMenuItemIcon(newSelection, 15, newYPos, WHITE);
}

// ==============================================================================
// 3.1 GENERAL INPUT HANDLER
// ==============================================================================
/**
 * @brief Handles input for all game states. Will be used for volume, home, and rest buttons.
 */

void handleGeneralInput() {
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();
  
  // Handle Home Button

  if(digitalRead(PIN_HOME) == LOW) {
    currentState = STATE_MENU;
    drawMenu();
    drawMenuCursor(-1, menuSelection); // Draw cursor at current selection
    delay(300); // Debounce
  }
  // Handle Reset. Pushing both Home and Select will reset the divice
  //TODO: Add reset action.
  // if(digitalRead(PIN_HOME) == LOW && digitalRead(PIN_SELECT) == LOW) {
  // }

}



// ==============================================================================
// 4. MENU HANDLER
// ==============================================================================

/**
 * @brief Handles input when the device is in the main menu state.
 */
void handleMenuInput() {
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();
  int prevSelection = menuSelection;

  // Handle Movement (Directional Buttons)
  if (currentTime - lastMoveTime >= moveDelay) {
    bool moved = false;

    if (digitalRead(PIN_UP) == LOW) {
      menuSelection = max(0, menuSelection - 1);
      moved = true;
    } else if (digitalRead(PIN_DOWN) == LOW) {
      menuSelection = min(numMenuItems - 1, menuSelection + 1);
      moved = true;
    }

    if (moved) {
      lastMoveTime = currentTime;
      drawMenuCursor(prevSelection, menuSelection);
    }
  }

  // Handle Action (A Button)
  if (digitalRead(PIN_BUTTONA) == LOW) {
    if (menuSelection == 0) {
      // Option 0: Tic-Tac-Toe
      ticTacToeSelected();
    } else if (menuSelection == 1) {
      // Option 1: POKEMON BATTLER
      pokemonBattlerSelected();
    }else {
      // Placeholder for other games
      displayStatus("Game unavailable", RED);
    }
    delay(300); // Debounce select press
  }
}

void ticTacToeSelected()
{
  currentState = STATE_TICTACTOE;
  resetTicTacToe();
}
void pokemonBattlerSelected(){
  currentState = STATE_POKEMON_BATTLER;
  resetPokemonBattler();
}
// ==============================================================================
// 5. TICTACTOE GAME LOGIC
// ==============================================================================

// --- Tic-Tac-Toe Drawing Functions ---

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
void drawMarker(int col, int row, int player) {
  int startX = 20;
  int startY = 50;
  int cellSize = 60;
  int centerX = startX + col * cellSize + cellSize / 2;
  int centerY = startY + row * cellSize + cellSize / 2;
  int size = cellSize / 3;

  if (player == PLAYER_X) {
    tft.drawLine(centerX - size, centerY - size, centerX + size, centerY + size, RED);
    tft.drawLine(centerX - size, centerY + size, centerX + size, centerY - size, RED);
    tft.drawLine(centerX - size + 1, centerY - size, centerX + size + 1, centerY + size, RED);
    tft.drawLine(centerX - size + 1, centerY + size, centerX + size + 1, centerY - size, RED);
  } else if (player == PLAYER_O) {
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
  tft.drawRect(startX + prevX * cellSize + margin,
               startY + prevY * cellSize + margin,
               cellSize - 2 * margin, cellSize - 2 * margin, BLACK);


  // // 2. Draw new cursor position only if the game is running AND the spot is empty
  // if (gameStatus == 0 && board[newX][newY] == EMPTY) {
  //   tft.drawRect(startX + newX * cellSize + margin,
  //                startY + newY * cellSize + margin,
  //                cellSize - 2 * margin, cellSize - 2 * margin, CURSOR_COLOR);
  // }
  // 2. Draw new cursor position only if the game is running
  if (gameStatus == 0) {
    tft.drawRect(startX + newX * cellSize + margin,
                 startY + newY * cellSize + margin,
                 cellSize - 2 * margin, cellSize - 2 * margin, CURSOR_COLOR);
  }
}

// --- Tic-Tac-Toe Game Logic Functions ---

/**
 * @brief Resets the board, state, and redraws the display for a new game.
 */
void resetTicTacToe() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      board[i][j] = EMPTY;
    }
  }
  currentPlayer = PLAYER_X;
  gameStatus = 0;
  cursorX = 0;
  cursorY = 0;
  gameOverScreenDrawn = false;

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
 * @brief Handles the game over state for Tic-Tac-Toe.
 * @param winner The player who won (1 for X, 2 for O, 3 for draw).
 */
void ticTacToeGameOver(int winner) {
  if(!gameOverScreenDrawn){
    tft.fillScreen(BLACK); // Clear the screen

    tft.setFont();
    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.setCursor(30, 10);
    tft.print("Game Over!");

    if (winner == 1) {
      tft.setTextColor(RED);
      tft.setCursor(30, 50);
      tft.print("X Wins!");
    } else if (winner == 2) {
      tft.setTextColor(BLUE);
      tft.setCursor(30, 50);
      tft.print("O Wins!");
    }else if (winner == 3) {
      tft.setTextColor(YELLOW);
      tft.setCursor(30, 50);
      tft.print("Draw!");
    }else{
      displayStatus("ERROR! (Select to Reset)", YELLOW);
    }

    tft.setTextSize(2);
    tft.setTextColor(WHITE);
    tft.setCursor(30, 100);
    tft.print("Press Select to Reset");
    tft.setCursor(30, 130);
    tft.print("Press Menu to Exit");

    gameOverScreenDrawn = true;

  }
  if(digitalRead(PIN_SELECT) == LOW) {
      resetTicTacToe();
      delay(300);
  }
  
}

/**
 * @brief Reads physical button inputs and updates the Tic-Tac-Toe game state.
 */
void handleTicTacToeInput() {
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 150; // Throttle delay for cursor movement

  // If the game is over, only the SELECT button can restart it.
  if (gameStatus != 0) {
    ticTacToeGameOver(gameStatus);
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
    } else if (digitalRead(PIN_RIGHT) == LOW) { 
      cursorX = min(BOARD_SIZE - 1, cursorX + 1);
      moved = true;
    }else if (digitalRead(PIN_LEFT) == LOW) { 
      cursorX = max(0, cursorX - 1);
      moved = true;
    }

    if (moved) {
      lastMoveTime = currentTime;
      drawCursor(prevX, prevY, cursorX, cursorY);
      return;
    }
  }

  // Handle Action (A Button)
  if (digitalRead(PIN_BUTTONA) == LOW) {
    // Only place a marker if the current cell is empty
    if (board[cursorX][cursorY] == EMPTY) {
      // 1. Place marker
      board[cursorX][cursorY] = currentPlayer;
      drawMarker(cursorX, cursorY, currentPlayer);
      //drawCursor(cursorX, cursorY, cursorX, cursorY); // This call erases the cursor

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
// 5.1 POKEMON BATTLER GAME
// ==============================================================================

// Define game state variables
bool isPlayersTurn = true;








void handlePokemonBattlerInput() {
  // Handle input for the Pokemon Battler game here
  // ...
}

void drawPokemonBattlerUI() {
  // Draw the UI for the Pokemon Battler game here
  // Select Menu
  //tft.drawRect(0, selectMenuY, tft.width(), tft.height(), WHITE);
  tft.fillRect(0, 200, tft.width(), tft.height(), WHITE);
  
  // The Select Menu shows the player options. Current options are: Move, Switch, Bag, Run
  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  tft.setCursor(10, 200);
  tft.print("Move");
  tft.setCursor(tft.getCursorX() + 5, tft.getCursorY());
  tft.print("Switch");
  tft.setCursor(10, 200);
  tft.print("Bag");
  tft.setCursor(10, 200);
  tft.print("Run");
}

void drawPokemonBattler() {
  // Draw the Pokemon Battler game screen here
  tft.fillScreen(BLACK);

  drawPokemonBattlerUI();
}

void resetPokemonBattler() {
  // Reset the game state here
  drawPokemonBattler();
}



// ==============================================================================
// 6. ARDUINO SETUP & LOOP
// ==============================================================================

void setup() {
  Serial.begin(115200);

  // Initialize display
  tft.begin();
  // Most ILI9341 screens are 240x320. Rotation(1) makes it 320x240 (landscape)
  tft.setRotation(1);

  // Initialize buttons with internal pull-up resistors
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_HOME, INPUT_PULLUP);
  pinMode(PIN_BUTTONA, INPUT_PULLUP);
  pinMode(PIN_BUTTONB, INPUT_PULLUP);

  // Start the device in the main menu
  drawMenu();
  drawMenuCursor(-1, 0); // Draw cursor on the first item
}

void loop() {
  // Main state machine to switch between Menu and Game modes

  handleGeneralInput();

  switch (currentState) {
    case STATE_MENU:
      handleMenuInput();
      break;
    case STATE_TICTACTOE:
      handleTicTacToeInput();
      break;
    case STATE_POKEMON_BATTLER:
      handlePokemonBattlerInput();
      break;
  }
  delay(10);
}