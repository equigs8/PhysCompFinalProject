#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <map>
#include <array>
#include <string>
#include <math.h>
#include "pokemon.h"
#include "pikachu.h"
#include "charmander.h"


// ==============================================================================
// 1. PIN DEFINITIONS (ADJUST THESE FOR YOUR WIRING)
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
  STATE_POKEMON_BATTLER,
  STATE_CHESS
};
GameState currentState = STATE_MENU;

// Menu Variables
int menuSelection = 0; // Index of the currently selected menu item
const char* menuItems[] = {"Tic-Tac-Toe", "Pokemon", "Chess", "Settings"};
const int numMenuItems = sizeof(menuItems) / sizeof(menuItems[0]);

// Color Definitions
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define CURSOR_COLOR 0xF800 // Light green/cyan for cursor outline
#define MAIN_MENU_COLOR BLACK
#define CHESS_BOARD_LIGHT_COLOR 0xDEFB 
#define CHESS_BOARD_DARK_COLOR 0x7D1F


// Pokemon Battle Specific Colors
#define SKY_BLUE    0x7D1F // Light blue for sky
#define GRASS_GREEN 0x5E09 // Green for field
#define PLATFORM_COL 0xDEFB // Light yellowish-green for ellipses
#define HUD_BG      0xEF7D // Light Gray for text boxes
#define HP_GREEN    0x2624 // Bright Green
#define HP_RED      0xF800


// Icon Bitmap Data (8x8 pixels, 1-bit monochrome)
// This icon is a simple 3x3 grid for Tic-Tac-Toe
const uint8_t TIC_TAC_TOE_ICON_BITS[] PROGMEM = {
 0x24, 0x24, 0xff, 0x24, 0x24, 0xff, 0x24, 0x24
};

const uint16_t POKEMON_ICON_BITS[] PROGMEM = {
  
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0010 (16) pixels
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0020 (32) pixels
0x0000, 0x0000, 0x0000, 0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0030 (48) pixels
0x0000, 0x0000, 0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x0000, 0x0000, 0x0000,   // 0x0040 (64) pixels
0x0000, 0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x0000, 0x0000,   // 0x0050 (80) pixels
0x0000, 0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x0000, 0x0000,   // 0x0060 (96) pixels
0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x0000, 0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x0000,   // 0x0070 (112) pixels
0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0xA986, 0xA986, 0xA986, 0xA986, 0xA986, 0x0000,   // 0x0080 (128) pixels
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0090 (144) pixels
0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000,   // 0x00A0 (160) pixels
0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000,   // 0x00B0 (176) pixels
0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000,   // 0x00C0 (192) pixels
0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000,   // 0x00D0 (208) pixels
0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x00E0 (224) pixels
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x00F0 (240) pixels
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,   // 0x0100 (256) pixels

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
void chessSelected();
void resetPokemonBattler();
void resetChess();

// ==============================================================================
// 3. DRAWING FUNCTIONS
// ==============================================================================


void drawSpriteWithTransparency(int x, int y, const uint16_t *bitmap, int w, int h, uint16_t transparentColor) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            
            uint16_t color = pgm_read_word(&bitmap[j * w + i]);
            
            // Only draw if it matches your transparency key
            if (color != transparentColor) {
                tft.drawPixel(x + i, y + j, color);
            }
        }
    }
}

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
    
  }else if(itemIndex == 1) {
    // draw Pokemon Icon
    drawSpriteWithTransparency(xPos, yPos, POKEMON_ICON_BITS, 16, 16, 0x0000);
  }
   else if (itemIndex == 2) {
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
    }else if (menuSelection == 2) {
      // Option 2: CHESS
      chessSelected();
    }
    else {
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
void chessSelected(){
  currentState = STATE_CHESS;
  resetChess();
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
// 5.1 POKEMON BATTLER GAME (NON-BLOCKING)
// ==============================================================================



// Pokemon
Pokemon playerPokemon = createPikachu(1);


Pokemon enemyPokemon = createCharmander(1);


//Player Bag
std::string playerBag[4] = {"Poke Ball", "Great Ball", "Ultra Ball", "Master Ball"};


// Pokemon Menu Layout Variables
int pokemonMenuRow1StartingX = 10;
int pokemonMenuRow1StartingY = 200;
int pokemonMenuRow2StartingX = 10;
int pokemonMenuRow2StartingY = 220;

// Define game state variables
bool isPlayersTurn = true;
enum BattlePhase {
  PHASE_PLAYER_CHOICE,
  PHASE_EXECUTING_TURN,
  PHASE_ENEMY_CHOICE,
  PHASE_ENEMY_EXECUTING_TURN,
  PHASE_GAME_OVER
};
BattlePhase battlePhase = PHASE_PLAYER_CHOICE;

void executeTurn(Move playerMove);

int pokemonMenuSelection = 0; 
int pokemonSubMenuSelection = 0; 
int previousPokemonSubMenuSelection = -1;

// Visual Feedback State Variables (For the non-blocking flash effect)
bool feedbackActive = false;
unsigned long feedbackStartTime = 0;
const int FEEDBACK_DURATION = 100; // ms

std::string pokemonMove1 = playerPokemon.getMove(0).getName();
std::string pokemonMove2 = playerPokemon.getMove(1).getName();
std::string pokemonMove3 = playerPokemon.getMove(2).getName();
std::string pokemonMove4 = playerPokemon.getMove(3).getName();

const std::map<int, std::array<std::string, 4>> pokemonMenuItems = {
  {0, {"Move", "Switch", "Bag", "Run"}},
  {1, {pokemonMove1, pokemonMove2, pokemonMove3, pokemonMove4}}, 
  {2, {"Pikachu", "Charmander", "Squirtle", "Bulbasaur"}}, 
  {3, {"Poke Ball", "Great Ball", "Ultra Ball", "Master Ball"}}, 
  {4, {"Yes", "No", "",""}} 
};


// Game Over state variable
bool gameIsOver = false;



// Helper function to get X, Y, Width, Height based on cursor index (0-3)
void getCursorRect(int index, int &x, int &y, int &w, int &h) {
  w = 90; 
  h = 18; 

  // Column Logic
  if (index == 0 || index == 2) x = pokemonMenuRow1StartingX;
  else x = pokemonMenuRow1StartingX + 100;

  // Row Logic
  if (index == 0 || index == 1) y = pokemonMenuRow1StartingY;
  else y = pokemonMenuRow2StartingY;
}

void drawPokemonMenuCursor(int prevSelection, int newSelection) {
  int x, y, w, h;
  // 1. Erase previous cursor 
  if (prevSelection >= 0 && prevSelection <= 3) {
    getCursorRect(prevSelection, x, y, w, h);
    tft.drawRect(x - 2, y - 2, w, h, WHITE); 
  }
  // 2. Draw new cursor
  if (newSelection >= 0 && newSelection <= 3) {
    getCursorRect(newSelection, x, y, w, h);
    tft.drawRect(x - 2, y - 2, w, h, BLACK);
  }
}

void drawPokemonBattlerUI() {
  tft.fillRect(0, 190, tft.width(), tft.height() - 190, WHITE);
  
  std::string opt0 = pokemonMenuItems.at(pokemonMenuSelection).at(0);
  std::string opt1 = pokemonMenuItems.at(pokemonMenuSelection).at(1);
  std::string opt2 = pokemonMenuItems.at(pokemonMenuSelection).at(2);
  std::string opt3 = pokemonMenuItems.at(pokemonMenuSelection).at(3);

  tft.setTextSize(2);
  tft.setTextColor(BLACK);
  
  tft.setCursor(pokemonMenuRow1StartingX, pokemonMenuRow1StartingY);
  tft.print(opt0.c_str());
  tft.setCursor(pokemonMenuRow1StartingX + 100, pokemonMenuRow1StartingY);
  tft.print(opt1.c_str());
  tft.setCursor(pokemonMenuRow2StartingX, pokemonMenuRow2StartingY);
  tft.print(opt2.c_str());
  tft.setCursor(pokemonMenuRow2StartingX + 100, pokemonMenuRow2StartingY);
  tft.print(opt3.c_str());

  drawPokemonMenuCursor(-1, pokemonSubMenuSelection);
}

void handlePokemonBattlerInput() {
  static unsigned long lastNavTime = 0;
  static unsigned long lastButtonTime = 0;
  const unsigned long navDelay = 200;
  const unsigned long buttonDelay = 300;
  unsigned long currentTime = millis();

  // 1. HANDLE GAME OVER
  if (battlePhase == PHASE_GAME_OVER) {
    if (digitalRead(PIN_BUTTONB) == LOW) {
       currentState = STATE_MENU;
       drawMenu();
       drawMenuCursor(-1, 0);
       delay(500);
    }else if (digitalRead(PIN_SELECT) == LOW) {
       resetPokemonBattler();
    }
    return; // Stop processing other inputs
  }

  // 2. IF EXECUTING TURN, IGNORE INPUTS (Blocking)
  if (battlePhase == PHASE_EXECUTING_TURN) {
    return; 
  }

  // 3. MENU NAVIGATION (Only if in Player Choice Phase)
  if (currentTime - lastNavTime >= navDelay) {
    bool moved = false;
    previousPokemonSubMenuSelection = pokemonSubMenuSelection;

    if (digitalRead(PIN_UP) == LOW) {
      if (pokemonSubMenuSelection >= 2) { pokemonSubMenuSelection -= 2; moved = true; }
    } 
    else if (digitalRead(PIN_DOWN) == LOW) {
      if (pokemonSubMenuSelection <= 1) { pokemonSubMenuSelection += 2; moved = true; }
    }
    else if (digitalRead(PIN_LEFT) == LOW) {
      if (pokemonSubMenuSelection % 2 != 0) { pokemonSubMenuSelection -= 1; moved = true; }
    } 
    else if (digitalRead(PIN_RIGHT) == LOW) {
      if (pokemonSubMenuSelection % 2 == 0) { pokemonSubMenuSelection += 1; moved = true; }
    }

    if (moved) {
      lastNavTime = currentTime;
      drawPokemonMenuCursor(previousPokemonSubMenuSelection, pokemonSubMenuSelection);
    }
  }

  // 4. BUTTON INPUTS
  if (currentTime - lastButtonTime >= buttonDelay) {
    
    // BUTTON A (Select)
    if (digitalRead(PIN_BUTTONA) == LOW) {
      lastButtonTime = currentTime;

      if (pokemonMenuSelection == 0) {
        // We are in the Top Level (Fight, Bag, etc). Enter "Fight"
        pokemonMenuSelection = pokemonSubMenuSelection + 1;
        pokemonSubMenuSelection = 0; 
        drawPokemonBattlerUI();
      } 
      else if (pokemonMenuSelection == 1) { 
        // We are in the "Fight" Menu (Selecting a Move)
        Move selectedMove = playerPokemon.getMove(pokemonSubMenuSelection);
        executeTurn(selectedMove); // <--- TRIGGER THE TURN
      }
    }

    // BUTTON B (Back)
    if (digitalRead(PIN_BUTTONB) == LOW) {
      lastButtonTime = currentTime;
      
      if (pokemonMenuSelection != 0) {
        pokemonMenuSelection = 0;
        pokemonSubMenuSelection = 0;
        drawPokemonBattlerUI();
      }
    }
  }
}

// --- HELPER: Draw Filled Ellipse ---
void drawFilledEllipse(int cx, int cy, int rx, int ry, uint16_t color) {
  for (int y = -ry; y <= ry; y++) {
    // Calculate the width of the ellipse at this y-level
    int width = (int)(2.0 * rx * sqrt(1.0 - ((float)(y * y) / (float)(ry * ry))));
    
    tft.drawFastHLine(cx - width / 2, cy + y, width, color);
  }
}

// --- HELPER: Draw HP Bar and Text Box ---
void drawHUD(int x, int y, const char* name, int level, int currentHP, int maxHP, bool isPlayer) {
  int w = 140;
  int h = 45;
  
  // Draw Box Background (with black outline)
  tft.fillRect(x, y, w, h, HUD_BG); // Gray background
  tft.drawRect(x, y, w, h, BLACK);  // Border
  
  // Text Settings
  tft.setTextColor(BLACK);
  tft.setTextSize(1); // Standard small font
  
  // 1. Name
  tft.setCursor(x + 10, y + 5);
  //set text size acording to name length
  if(strlen(name) > 7){
    tft.setCursor(x + 10, y + 10);
    tft.setTextSize(1);
  }else{
    tft.setTextSize(2);
  }
  
  tft.print(name);
  
  // 2. Level symbol (simple "Lv")
  tft.setTextSize(1);
  tft.setCursor(x + 100, y + 10);
  tft.print("Lv");
  tft.print(level);

  // 3. HP Bar Background
  int barX = x + 30;
  int barY = y + 25;
  int barW = 100;
  int barH = 6;
  
  tft.fillRect(barX, barY, barW, barH, 0x52AA); // Dark grey empty bar
  
  // 4. HP Bar Fill (Calculate percentage)
  float hpPercent = (float)currentHP / maxHP;
  int fillW = (int)(barW * hpPercent);
  
  // Determine color (Green > 50%, Yellow > 20%, Red otherwise)
  uint16_t hpColor = HP_GREEN;
  if(hpPercent < 0.2) hpColor = RED;
  else if(hpPercent < 0.5) hpColor = YELLOW;
  
  tft.fillRect(barX, barY, fillW, barH, hpColor);

  // 5. Numeric HP (Only for Player)
  if (isPlayer) {
    tft.setCursor(x + 50, y + 35);
    tft.print(currentHP);
    tft.print("/");
    tft.print(maxHP);
  }
}

void drawHPUI(){
  // 4. Draw Enemy HUD (Top Left)
  drawHUD(10, 15, enemyPokemon.getName(), enemyPokemon.getLevel(), enemyPokemon.getHealth(), enemyPokemon.getMaxHealth(), false);

  // 5. Draw Player HUD (Right side, above menu)
  drawHUD(170, 135, playerPokemon.getName(), playerPokemon.getLevel(), playerPokemon.getHealth(), playerPokemon.getMaxHealth(), true); // Player
}

// --- MAIN SCENE DRAW FUNCTION ---
void drawBattleScene() {
  // 1. Draw Background (Split Screen)
  // Sky (Top ~40%)
  tft.fillRect(0, 0, 320, 80, SKY_BLUE);
  // Grass (Bottom ~60% until the menu starts at 190)
  tft.fillRect(0, 80, 320, 110, GRASS_GREEN); // Ends at y=190

  // 2. Draw Enemy Platform (Top Right)
  int enemyBaseX = 230;
  int enemyBaseY = 90;

  drawFilledEllipse(enemyBaseX, enemyBaseY, 70, 25, PLATFORM_COL);

  

  // 3. Draw Player Platform (Bottom Left)
  int playerBaseX = 90;
  int playerBaseY = 180;
  drawFilledEllipse(playerBaseX, playerBaseY, 80, 30, PLATFORM_COL);

  drawHPUI();
  

  // 6. Draw Pokemon SpritesS
  // Enemy
  drawSpriteWithTransparency(enemyBaseX - 30, enemyBaseY - 30, enemyPokemon.getSprite(), enemyPokemon.getSpriteWidth(), enemyPokemon.getSpriteHeight(), 0x0000); 

  // Player  
  drawSpriteWithTransparency(playerBaseX - 30, playerBaseY - 60, playerPokemon.getSprite(), playerPokemon.getSpriteWidth(), playerPokemon.getSpriteHeight(), 0x0000);
}

// BATTLE MECHANICS
void drawBattleMessage(const char* message) {
  tft.fillRect(0,190,tft.width(),tft.height() - 190,WHITE);
  tft.drawRect(10,200,300,35,BLACK);
  tft.setCursor(20,210);
  tft.setTextSize(2);
  tft.print(message);
}

bool performAttack(Pokemon &attacker, Pokemon &defender, Move move) {
  String msg = String(attacker.getName()) + " used " + move.getName() + "!";
  drawBattleMessage(msg.c_str());
  
  delay(1500); // Wait for message to be read

  int damage = move.getDamage();

  defender.takeDamage(damage);

  if(defender.getHealth() < 0){
    defender.heal(abs(defender.getHealth()));
  }

  drawHPUI();
  delay(1000);

  

  if (defender.getHealth() <= 0) {
    char * defenderName = defender.getNonConstName();
    msg = String(defenderName) + " fainted!";
    drawBattleMessage(msg.c_str());
    delay(2000);
    return true; // Battle Ends
  }
  return false; // Battle Continues
  
}

void endBattle(bool playerWon){
  battlePhase = PHASE_GAME_OVER;
  tft.fillScreen(BLACK);
  tft.setCursor(30, 100);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  
  if (playerWon) {
    char* pokemonName = enemyPokemon.getNonConstName();
    tft.print("YOU WON!");
    tft.setCursor(30, 140);
    tft.setTextSize(2);
    tft.print(pokemonName);
    tft.print(" fainted.");
  } else {
    char* pokemonName = playerPokemon.getNonConstName();
    tft.print("YOU LOST...");
    tft.setCursor(30, 140);
    tft.setTextSize(2);
    tft.print("Pikachu fainted.");
  }
  
  tft.setTextSize(1);
  tft.setCursor(30, 180);
  tft.print("Press Home to Exit");
  tft.setCursor(30, 200);
  tft.print("Press Select to Play Again");
}
void executeTurn(Move playerMove) {
  battlePhase = PHASE_EXECUTING_TURN; // Lock inputs

  // --- PLAYER TURN ---
  bool enemyFainted = performAttack(playerPokemon, enemyPokemon, playerMove);

  if (enemyFainted) {
    endBattle(true); // Player wins
    return;
  }

  // --- ENEMY TURN ---
  // Simple AI: Pick a random move (0-3)
  int randIndex = random(0, 4);
  Move enemyMove = enemyPokemon.getMove(randIndex);

  bool playerFainted = performAttack(enemyPokemon, playerPokemon, enemyMove);

  if (playerFainted) {
    endBattle(false); // Player loses
    return;
  }

  // --- RETURN TO MENU ---
  battlePhase = PHASE_PLAYER_CHOICE;
  drawPokemonBattlerUI(); // Redraw the menu options
}

void resetPokemonBattler() {
  // Reset Logic
  pokemonMenuSelection = 0; 
  pokemonSubMenuSelection = 0; 
  battlePhase = PHASE_PLAYER_CHOICE; // Reset phase
  
  // Heals for a new game (optional, or you can keep persistence)
  playerPokemon.heal(1000); 
  enemyPokemon.heal(1000);

  tft.fillScreen(BLACK);
  drawBattleScene();
  drawPokemonBattlerUI();
}


// ==============================================================================
// 5.2 CHESS
// ==============================================================================

// White device will act as the game manager and main source of truth

enum ChessPhase{
  MENU,
  START_GAME,
  WHITE_TURN,
  BLACK_TURN,
  GAME_OVER
};
ChessPhase chessPhase = MENU;
int turnNumber = 0; // turn 0 is the first. Even turns are White and Black are odd.

int chessMenuSelection = 0; //0 = Play as white, 1 = play as black;
int previousChessMenuSelection = -1;

const char * chessMenu[] = {"Play as White", "Play as Black"};
int numChessMenu = 2;

bool playingAsWhite = false;

// Chess board. Each element represents a square on the board. The value of the element corisponds to the type and color of the piece.
// 0 = Empty Square
// Pawns = 1,  Bishops = 2,  Knights= 3, Rooks = 4, Queens = 5, Kings = 6
// Black pieces will be negative while white pieces will be positive
std::array<std::array<int, 8>, 8> chessBoard = {{
  {-4, -3, -2, -5, -6, -2, -3, -4},
  {-1, -1, -1, -1, -1, -1, -1, -1},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1, 1},
  {4, 3, 2, 5, 6, 2, 3, 4},
}};
// Chess board cursor. Represents the current location of the cursor on the chess board. Numbered from left to right and top to bottom.
int chessBoardCursorLocation = 0;
int chessBoardPreviousCursorLocation = -1;

// Grid configuration
int boardHeight = tft.height(); // 240
int squareSize = 28;
// Center the board on the screen
int chessSquareStartPosX = (tft.width() - (squareSize * 8)) / 2; 
int chessSquareStartPosY = (tft.height() - (squareSize * 8)) / 2;

void drawChessMenu(int previousSelection, int selection){
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(50,20);
  tft.print("CHESS");

  tft.setTextSize(2);
  for (int i = 0; i < numChessMenu; i++) {
    int yPos = 80 + i * 40;

    // Draw Text
    tft.setCursor(50, 80 + i * 40);
    tft.setTextColor(WHITE);
    tft.print(chessMenu[i]);
  }
}

/**
 * @brief Draws or moves the cursor/selection highlight on the Chess menu.
 */
void drawChessMenuCursor(int prevSelection, int newSelection) {
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
    tft.print(chessMenu[prevSelection]);
    
  }

  // 2. Draw new cursor highlight and redraw text/icon in BLACK.
  int newYPos = yStart + newSelection * 40;
  
  // Draw new cursor (filled box)
  tft.fillRect(xPos, newYPos, width, height, CURSOR_COLOR);
  
  // Redraw Text (BLACK on the colored cursor)
  tft.setCursor(50, newYPos);
  tft.setTextColor(BLACK); 
  tft.setTextSize(2);
  tft.print(chessMenu[newSelection]);
}


void drawChessGameOver(){
  tft.fillScreen(BLACK);

}

int getChessSquareLocationX(int square){
  int squareX = square % 8;
  return chessSquareStartPosX + squareX * squareSize;
}
int getChessSquareLocationY(int square){
  int squareY = square / 8;
  return chessSquareStartPosY + squareY * squareSize;
}

int getChessSquareColor(int square){
  int squareValue = chessBoard[square / 8][square % 8];
  return (squareValue > 0) ? CHESS_BOARD_LIGHT_COLOR : CHESS_BOARD_DARK_COLOR;
}

void drawChessCursor(int currentPostion, int previousPosition){
  //draw a a cursor color box around the current square.
  
  //get color of previous square
  int previousColor = getChessSquareColor(previousPosition);
  
  int previousSquareLocationX = getChessSquareLocationX(previousPosition);
  int previousSquareLocationY = getChessSquareLocationY(previousPosition);
  int squareLocationX = getChessSquareLocationX(currentPostion);
  int squareLocationY = getChessSquareLocationY(currentPostion);

  if (previousPosition >= 0) {
    //Draw a box to color in the previous position
    tft.drawRect(previousSquareLocationX, previousSquareLocationY, squareSize, squareSize, previousColor);
  }
  //Draw a box for new selection
  tft.drawRect(squareLocationX, squareLocationY, squareSize, squareSize, CURSOR_COLOR);
}

void drawChessPiece(int posX, int posY, int squareSize, int color, int pieceType) {
  // Calculate center of the specific square
  int centerX = posX + (squareSize / 2);
  int centerY = posY + (squareSize / 2);
  int radius = (squareSize / 2) - 3; // Leave a small margin so pieces don't touch edges

  // Draw the piece
  tft.fillCircle(centerX, centerY, radius, color);
  
  // Draw the outline
  tft.drawCircle(centerX, centerY, radius, BLACK); 

  // TODO: Add specific logic here for bitmaps
  // For now, print a char to identify the piece
  tft.setCursor(centerX - 3, centerY - 4);
  tft.setTextColor(color == WHITE ? BLACK : WHITE);
  tft.setTextSize(1);
  tft.print(abs(pieceType));
}

void drawChessBoard() {
  tft.fillScreen(BLACK);

  chessSquareStartPosX = (tft.width() - (squareSize * 8)) / 2; 
  chessSquareStartPosY = (tft.height() - (squareSize * 8)) / 2;
  

  for (int i = 0; i < 8; i++) {   // i = rows (Y)
    for (int j = 0; j < 8; j++) { // j = cols (X)
      
      int x = chessSquareStartPosX + (j * squareSize);
      int y = chessSquareStartPosY + (i * squareSize);

      uint16_t color = ((i + j) % 2 == 0) ? CHESS_BOARD_LIGHT_COLOR : CHESS_BOARD_DARK_COLOR;
      
      tft.fillRect(x, y, squareSize, squareSize, color);

      // Draw the piece if it exists
      int squareValue = chessBoard[i][j];
      if (squareValue != 0) {
        uint16_t pieceColor = (squareValue > 0) ? WHITE : BLACK;
        drawChessPiece(x, y, squareSize, pieceColor, squareValue);
      }
    }
  }
}

void handleChessInputs(){
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();
  if(chessPhase == MENU){
    
    int previousChessMenuSelection = chessMenuSelection;

    // Handle Movement (Directional Buttons)
    if (currentTime - lastMoveTime >= moveDelay) {
      bool moved = false;

      if (digitalRead(PIN_UP) == LOW) {
        chessMenuSelection= max(0, chessMenuSelection - 1);
        moved = true;
      } else if (digitalRead(PIN_DOWN) == LOW) {
        chessMenuSelection = min(numChessMenu - 1, chessMenuSelection + 1);
        moved = true;
      }

      if (moved) {
        lastMoveTime = currentTime;
        drawChessMenuCursor(previousChessMenuSelection,chessMenuSelection);
      }
    }

    // Handle Action (A Button)
    if (digitalRead(PIN_BUTTONA) == LOW) {
      if (chessMenuSelection == 0) {
        // Option 0: Play as White
        playingAsWhite = true;
        chessPhase = START_GAME;
        
      } else if (chessMenuSelection == 1) {
        // Option 1: Play as Black
        playingAsWhite = false;
        chessPhase = START_GAME;
      }
      delay(300); // Debounce select press
    }
    
  }else if(chessPhase == START_GAME){
    // start the game and reset the move array
    chessPhase = WHITE_TURN;
    drawChessBoard();
    drawChessCursor(chessBoardCursorLocation, chessBoardPreviousCursorLocation);
  }else if (chessPhase == WHITE_TURN){
    // White can send a move. Black waits for move.
    if(currentTime - lastMoveTime >= moveDelay){
      bool moved = false;

      if (digitalRead(PIN_UP) == LOW) {
        chessBoardCursorLocation = max(0, chessBoardCursorLocation - 8);
        moved = true;
      } else if (digitalRead(PIN_DOWN) == LOW) {
        chessBoardCursorLocation = min(63, chessBoardCursorLocation + 8);
        moved = true;
      } else if (digitalRead(PIN_RIGHT) == LOW) {
        chessBoardCursorLocation = min(63, chessBoardCursorLocation + 1);
        moved = true;
      } else if (digitalRead(PIN_LEFT) == LOW) {
        chessBoardCursorLocation = max(0, chessBoardCursorLocation - 1);
        moved = true;
      }

      if (moved) {
        lastMoveTime = currentTime;
        drawChessCursor(chessBoardCursorLocation, chessBoardPreviousCursorLocation);
        chessBoardPreviousCursorLocation = chessBoardCursorLocation;
      }
    }
  }else if (chessPhase == BLACK_TURN){
    // Black can send a move. White lisens for the move.
  }else if (chessPhase == GAME_OVER){
    drawChessGameOver();
  }
}

void resetChess(){
  turnNumber = 0;
  tft.fillScreen(BLACK);
  chessPhase = MENU;
  drawChessMenu(previousChessMenuSelection, chessMenuSelection);
  drawChessMenuCursor(previousChessMenuSelection, chessMenuSelection);
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
    case STATE_CHESS:
      handleChessInputs();
      break;
  }
  //delay(10);
}