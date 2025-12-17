/**
 * @file main.cpp
 * @author Ethan Quigley (https://github.com/equigs8)
 * @author August (https://github.com/goosey999)
 * @brief Main file for the Handheld Game Console
*/



#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <map>
#include <array>
#include <string>
#include <math.h>
//#include <WIFI.h>
#include <esp_now.h>
//Sprites and game clases
#include "pokemon.h"
// Monsters
#include "leafle.h"
#include "finpup.h"
#include "katanid.h"
#include "kitflare.h"
#include "mantiscyth.h"
#include "hammerfat.h"
#include "anchorjaw.h"
#include "omenmask.h"
#include "spiritail.h"
//Chess
#include "chessPiece.h"
#include "White_King.h"
#include "White_Queen.h"
#include "White_Rook.h"
#include "White_Bishop.h"
#include "White_Knight.h"
#include "White_Pawn.h"
#include "Black_King.h"
#include "Black_Queen.h"
#include "Black_Rook.h"
#include "Black_Bishop.h"
#include "Black_Knight.h"
#include "Black_Pawn.h"
#include "Icons.h"

// ==============================================================================
// 1. PIN DEFINITIONS (ADJUST THESE FOR YOUR WIRING)
// ==============================================================================

// SPI Display Pins (Common configuration for ESP32)
#define TFT_CS   5  // Chip Select (CS)
#define TFT_DC   4  // Data/Command (DC)
#define TFT_RST -1 // Reset pin (or -1 if connected to ESP32's reset)
// The CLK (SCK) and MOSI (SDA) pins for SPI are fixed on the ESP32 (GPIO18 and GPIO23 by default).

// Input Button Pins (Assuming buttons are wired as Active-LOW: Press connects pin to GND)
#define PIN_UP     45
#define PIN_DOWN   2
#define PIN_LEFT   48
#define PIN_RIGHT  1
#define PIN_SELECT 16
#define PIN_HOME   15
#define PIN_BUTTONA 18
#define PIN_BUTTONB 17

// #define PIN_UP     100
// #define PIN_DOWN   101
// #define PIN_LEFT   102
// #define PIN_RIGHT  103
// #define PIN_SELECT 104
// #define PIN_HOME   105
// #define PIN_BUTTONA 18
// #define PIN_BUTTONB 107

// Power LED Pin
//#define PIN_POWER_INDICATOR 46

// Volume Control
//#define PIN_VOLUME 8
//#define PIN_PIEZO 7

// ==============================================================================
// 2. DISPLAY SETUP & GLOBAL VARIABLES
// ==============================================================================

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// --- GAME STATE MANAGEMENT ---
enum GameState {
  STATE_MENU,
  STATE_TICTACTOE,
  STATE_POKEMON_BATTLER,
  STATE_CHESS,
  STATE_SETTINGS
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



// Serial Connection
#define SERIAL_BAUD_RATE 9600
#define SERIAL_TX_PIN 16
#define SERIAL_RX_PIN 17

int connectionMode = 0; // 0: Serial, 1: ESP-NOW

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
  char header; // 'M' for move
  int fromIdx;
  int toIdx;
} struct_message;

struct_message myData; // Outgoing data
struct_message incomingMove; // Incoming data
bool wirelessMoveReceived = false;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if(len == sizeof(incomingMove)) {
    memcpy(&incomingMove, incomingData, sizeof(incomingMove));
    if (incomingMove.header == 'M')
    {
      wirelessMoveReceived = true;
    }
  }
}

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
void resetSettings();
void settingsSelected();

// ==============================================================================
// 3. DRAWING FUNCTIONS
// ==============================================================================

/**
 * @brief Draws a sprite to the TFT screen, ignoring pixels of a specific color to create transparency.
 * * @param x Horizontal starting position.
 * @param y Vertical starting position.
 * @param bitmap Pointer to the 16-bit color bitmap array.
 * @param w Width of the sprite.
 * @param h Height of the sprite.
 * @param transparentColor The color value to be treated as transparent (not drawn).
 */
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
  int iconSize = 16;
  
  // Clear the icon area before drawing the icon to prevent color artifacts
  tft.fillRect(xPos + 3, yPos + 3, iconSize, iconSize, BLACK); 
  
  if (itemIndex == 0) {
    // Icon for Tic-Tac-Toe: Draw a 16x16 Bitmap
    // x, y, bitmap array, width, height, color
    drawSpriteWithTransparency(xPos, yPos, TIC_TAC_TOE_ICON_BITS, iconSize, iconSize, 0x0000);
    
  }else if(itemIndex == 1) {
    // draw Pokemon Icon
    drawSpriteWithTransparency(xPos, yPos, POKEMON_ICON_BITS, iconSize, iconSize, 0x0000);
  }else if (itemIndex == 2) {
    // draw Chess Icon
    drawSpriteWithTransparency(xPos, yPos, CHESS_ICON_BITS, iconSize, iconSize, 0x0000);
  }else if (itemIndex == 3) {
    // draw Settings Icon
    drawSpriteWithTransparency(xPos, yPos, SETTINGS_ICON_BITS, iconSize, iconSize, 0x0000);
  }
   else {
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
    tft.fillRect(xPos, prevYPos -2, width, height, BLACK);
    
    // Redraw Text (WHITE)
    tft.setCursor(50, prevYPos);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print(menuItems[prevSelection]);
    
    // Redraw Icon (WHITE) (Icon position: x=5, centered vertically around text line)
    //drawMenuItemIcon(prevSelection, 15, prevYPos, WHITE);
  }

  // 2. Draw new cursor highlight and redraw text/icon in BLACK.
  int newYPos = yStart + newSelection * 40;
  
  // Draw new cursor (filled box)
  tft.fillRect(xPos, newYPos - 2, width, height, CURSOR_COLOR);
  
  // Redraw Text (BLACK on the colored cursor)
  tft.setCursor(50, newYPos);
  tft.setTextColor(BLACK); 
  tft.setTextSize(2);
  tft.print(menuItems[newSelection]);
  
  // Redraw Icon (BLACK on the colored cursor)
  //drawMenuItemIcon(newSelection, 15, newYPos, WHITE);
}

// ==============================================================================
// 2. AUDIO HANDLER
// ==============================================================================


#define AUDIO_RESOLUTION 8
int currentVolume = 128;

void initAudio() {
  //ledcAttach(PIN_PIEZO, 2000, AUDIO_RESOLUTION);
  //ledcWrite(PIN_PIEZO, 0);
}

void updateVolume(){
  // int potValue = analogRead(PIN_VOLUME);
  // int mappedValue = map(potValue, 0, 4095, 0, 128);

  // currentVolume = (mappedValue * mappedValue) / 128;
  // if(currentVolume < 2){
  //   currentVolume = 0;
  // }
}

void playTone(int freq, int duration) {
  // updateVolume();

  // if(currentVolume > 0 && freq > 0){
  //   ledcWriteTone(PIN_PIEZO, freq);
    
  //   ledcWrite(PIN_PIEZO, currentVolume);

  //   delay(duration);
  //   ledcWrite(PIN_PIEZO, 0);
  // }
}


// ==============================================================================
// 3.1 GENERAL INPUT HANDLER
// ==============================================================================

void goHome(){
  //playTone(1000, 50);
  currentState = STATE_MENU;
  drawMenu();
  drawMenuCursor(-1, menuSelection); // Draw cursor at current selection
  delay(300); // Debounce
}

/**
 * @brief Handles input for all game states. Will be used for volume, home, and rest buttons.
 */

void handleGeneralInput() {
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();
  
  // Handle Home Button
  if(currentTime - lastMoveTime >= moveDelay){
    bool moved = false;
    if(digitalRead(PIN_HOME) == LOW) {
      Serial.println("Home Button Pressed");
      moved = true;
      goHome();
    }
    // Handle Reset. Pushing both Home and Select will reset the divice
    
    if(digitalRead(PIN_HOME) == LOW && digitalRead(PIN_SELECT) == LOW) {
      //ESP.restart();
    }

    if(digitalRead(PIN_BUTTONB) == LOW) {
      Serial.println("Button B Pressed");
      moved = true;
    }
    if(digitalRead(PIN_BUTTONA) == LOW) {
      Serial.println("Button A Pressed");
      moved = true;
    }
    if (digitalRead(PIN_UP) == LOW)
    {
      Serial.println("Button Up Pressed");
      moved = true;
    }
    if(digitalRead(PIN_DOWN) == LOW) {
      Serial.println("Button Down Pressed");
      moved = true;
    }
    if(digitalRead(PIN_LEFT) == LOW) {
      Serial.println("Button Left Pressed");
      moved = true;
    }
    if(digitalRead(PIN_RIGHT) == LOW) {
      Serial.println("Button Right Pressed");
      moved = true;
    }
    if (digitalRead(PIN_SELECT) == LOW)
    {
      Serial.println("Select Pressed");
      moved = true;
    }
    if(digitalRead(PIN_HOME) == LOW) {
      Serial.println("Home Button Pressed");
      moved = true;
    }
    

     if (moved) {
      lastMoveTime = currentTime;
    }
  }

  
  
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
    else if (menuSelection == 3) {
      // Option 3: Settings
      settingsSelected();
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
void settingsSelected(){
  currentState = STATE_SETTINGS;
  resetSettings();
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



Pokemon playerPokemon;
Pokemon enemyPokemon;



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

// Helper function to generate a random Pokemon
Pokemon createRandomPokemon(int level) {
  int randomNum = random(0, 11); // 0 to 10 (11 total options)
  
  switch(randomNum) {
    case 0: return createLeafle(level);
    case 1: return createMantiscythe(level); // Ensure spelling matches your header
    case 2: return createKatanid(level);
    case 3: return createFinpup(level);
    case 4: return createHammerfat(level);
    case 5: return createAnchorjaw(level);
    case 6: return createKitflare(level);
    case 7: return createSpiritail(level);
    case 8: return createOmenmask(level);
    default: return createLeafle(level);
  }
}

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
  
  std::string opt0, opt1, opt2, opt3;

  // If we are selecting a move (Menu Index 1), get moves from the current Pokemon
  if (pokemonMenuSelection == 1) {
      opt0 = playerPokemon.getMove(0).getName();
      opt1 = playerPokemon.getMove(1).getName();
      opt2 = playerPokemon.getMove(2).getName();
      opt3 = playerPokemon.getMove(3).getName();
  } else {
      // Otherwise use the static menu text
      opt0 = pokemonMenuItems.at(pokemonMenuSelection).at(0);
      opt1 = pokemonMenuItems.at(pokemonMenuSelection).at(1);
      opt2 = pokemonMenuItems.at(pokemonMenuSelection).at(2);
      opt3 = pokemonMenuItems.at(pokemonMenuSelection).at(3);
  }

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

  if (damage < 0) {
    attacker.heal(abs(damage));
  }else{
    defender.takeDamage(damage);
  }

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
  playerPokemon = createRandomPokemon(1);
  enemyPokemon = createRandomPokemon(1);

  tft.fillScreen(BLACK);
  drawBattleScene();
  drawPokemonBattlerUI();
}


// ==============================================================================
// 5.2 CHESS
// ==============================================================================

// White device will act as the game manager and main source of truth

enum ChessPhase{
  CONNECTION_SELECT,
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

// Track the source square of the move. -1 = nothing selected
int selectedSourceSquare = -1;

// Grid configuration
int boardHeight = tft.height(); // 240
int squareSize = 30;

// Center the board on the screen
int chessSquareStartPosX = (tft.width() - (squareSize * 8)) / 2; 
int chessSquareStartPosY = (tft.height() - (squareSize * 8)) / 2;

int chessUIWidthMargin = 2;
int chessUIHeight = boardHeight;
int chessUIWidth = 40 - chessUIWidthMargin;
int chessUIStartingX = 0;
int chessUIStartingY = 0;

int chessCursorStartLocationWhite = 0;
int chessCursorStartLocationBlack = 63;

bool displayedgameOver = false;

bool isCheck = false;

bool isInCheck(int color);

const char * connMenu[] = {"Wired", "Wireless", "Single Player"};
int numConnMenu = 3;
int connMenuSelection = 0;

int getPieceAt(int index){
  int row = index / 8;
  int col = index % 8;
  return chessBoard[row][col];
}

bool isPathClear(int startIdx, int endIdx){
  int startRow = startIdx / 8;
  int startCol = startIdx % 8;
  int endRow = endIdx / 8;
  int endCol = endIdx % 8;
 
  int dRow = (endRow - startRow) == 0 ? 0 : (endRow - startRow) > 0 ? 1 : -1;
  int dCol = (endCol - startCol) == 0 ? 0 : (endCol - startCol) > 0 ? 1 : -1;
  
  int currentRow = startRow + dRow;
  int currentCol = startCol + dCol;
  
  while (currentRow != endRow || currentCol != endCol){
    if (chessBoard[currentRow][currentCol] != 0){
      return false;
    }
    currentRow += dRow;
    currentCol += dCol;
  }
  return true;
}

bool isValidMove(int fromIdx, int toInx){
  int piece = getPieceAt(fromIdx);
  int target = getPieceAt(toInx);

  // check if target is own piece
  if (target != 0)
  {
    bool sameColor = (piece > 0 && target > 0) || (piece < 0 && target < 0);
    if(sameColor)
    {
      return false;
    }
  }
  
  int startRow = fromIdx / 8;
  int startCol = fromIdx % 8;
  int endRow = toInx / 8;
  int endCol = toInx % 8;
  int dRow = endRow - startRow;
  int dCol = endCol - startCol;
  int absRow = abs(dRow);
  int absCol = abs(dCol);

  int type = abs(piece);

  switch(type){
    case 1: // Pawns
    {
      // White Pieces are Positive (>0). Black are Negative (<0).
      // Row 0 is Top. Row 7 is Bottom.
      // White moves UP (Row decreases: 6 -> 5). Direction = -1.
      // Black moves DOWN (Row increases: 1 -> 2). Direction = 1.
      int direction = (piece > 0) ? -1 : 1; 
      int startRowLimit = (piece > 0) ? 6 : 1;

      // 1. Move Forward 1 space (Non-Capture)
      if (dCol == 0 && dRow == direction && target == 0) {
        return true;
      }

      // 2. Move forward 2 (First move only, Non-Capture)
      if (dCol == 0 && dRow == direction * 2 && startRow == startRowLimit && target == 0) {
        // Check obstruction
        if (chessBoard[startRow + direction][startCol] == 0) {
          return true;
        }
      }
      if (absCol == 1 && dRow == direction) {
          
          // Ideally, we split this, but for simplicity:
          if (target != 0) return true; 
          
          // Hack: if target is 0, this returns false, which is correct for moving
          // BUT incorrect for "is square attacked".
          // For now, let's leave it as returning true ONLY if target != 0.
          // *Correction*: We need a slight modification to isSquareAttacked to handle pawn captures specifically 
          // or we modify this logic.
          // Simplest fix: Just check target != 0.
          return false;
      }
      return false;
    }
    case 2: // Bishops
      if (absRow == absCol)
      {
        return isPathClear(fromIdx, toInx);
      }
      return false;
      
    case 3: // Knights
      if((absRow == 2 && absCol == 1) || (absRow == 1 && absCol == 2)){
        return true;
      }
      return false;
    case 4: // Rooks
      if (dRow == 0 || dCol == 0)
      {
        return isPathClear(fromIdx, toInx);
      }
      return false;
      
    case 5: // Queens
      if (dRow == 0 || dCol == 0 || absRow == absCol)
      {
        return isPathClear(fromIdx, toInx);
      }
      return false;
    case 6: // Kings
      if (absRow <= 1 && absCol <= 1)
      {
        return true;
      }
      return false;
  }
  return false;
}


void drawChessUI(){
  tft.fillRect(chessUIStartingX, chessUIStartingY, chessUIWidth, chessUIHeight, WHITE);
  
  int leftJustified = chessUIStartingX + 2;

  tft.setTextSize(1);
  tft.print(turnNumber);
  tft.setCursor(leftJustified, chessUIStartingY + 2);
  tft.print("Turn: ");
  //tft.print(turnNumber);
  tft.setTextColor(BLACK);
  if(turnNumber % 2 == 0){
    tft.setCursor(leftJustified, chessUIStartingY + 10);
    tft.print("White");
  }else {
    tft.setCursor(leftJustified, chessUIStartingY + 10);
    tft.print("Black");
  }

  //print check if needed
  if(isInCheck(1) || isInCheck(-1)){
    tft.setTextSize(1);
    tft.setCursor(leftJustified, chessUIStartingY + 18);
    tft.print("Check!");
  }
}



void drawChessMenu(int previousSelection, int selection, int menuType){
  //Menu type is 0 for connection menu and 1 for color menu
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(50,20);
  tft.print("CHESS");

  tft.setTextSize(2);
  if(menuType == 0){
    for (int i = 0; i < numConnMenu; i++) {
      int yPos = 80 + i * 40;

      // Draw Text
      tft.setCursor(50, 80 + i * 40);
      tft.setTextColor(WHITE);
      tft.print(connMenu[i]);
    }
  }else{
    for (int i = 0; i < numChessMenu; i++) {
      int yPos = 80 + i * 40;

      // Draw Text
      tft.setCursor(50, 80 + i * 40);
      tft.setTextColor(WHITE);
      tft.print(chessMenu[i]);
    }
  }
}

/**
 * @brief Draws or moves the cursor/selection highlight on the Chess menu.
 */
void drawChessMenuCursor(int prevSelection, int newSelection, int menuType) {
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
    if(menuType == 0){
      tft.print(connMenu[prevSelection]);
    }else{
      tft.print(chessMenu[prevSelection]);
    }
    
    
  }

  // 2. Draw new cursor highlight and redraw text/icon in BLACK.
  int newYPos = yStart + newSelection * 40;
  
  // Draw new cursor (filled box)
  tft.fillRect(xPos, newYPos, width, height, CURSOR_COLOR);
  
  // Redraw Text (BLACK on the colored cursor)
  tft.setCursor(50, newYPos);
  tft.setTextColor(BLACK); 
  tft.setTextSize(2);
  if(menuType == 0){
    tft.print(connMenu[newSelection]);
  }else{
    tft.print(chessMenu[newSelection]);
  }
}


void drawChessGameOver(){
  gameOverScreenDrawn = true;
  tft.fillScreen(BLACK);

  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(50,20);
  tft.print("Game Over");
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
  int row = square / 8;
  int col = square % 8;

  return ((row + col) % 2 == 0) ? CHESS_BOARD_LIGHT_COLOR : CHESS_BOARD_DARK_COLOR;
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

    if(previousPosition != selectedSourceSquare){
      tft.drawRect(previousSquareLocationX, previousSquareLocationY, squareSize, squareSize, previousColor);
    }else{
      // Redraw the selection indicator
      tft.drawRect(previousSquareLocationX, previousSquareLocationY, squareSize, squareSize, CURSOR_COLOR);
    }

    tft.drawRect(previousSquareLocationX, previousSquareLocationY, squareSize, squareSize, previousColor);
  }
  //Draw a box for new selection
  tft.drawRect(squareLocationX, squareLocationY, squareSize, squareSize, CURSOR_COLOR);

  if (selectedSourceSquare != -1){
    int selX = getChessSquareLocationX(selectedSourceSquare);
    int selY = getChessSquareLocationY(selectedSourceSquare);

    tft.drawRect(selX, selY, squareSize, squareSize, HP_GREEN);
  }
}

// Chess Pieces
// White:
chessPiece MY_WHITE_KING = createWhiteKing();
chessPiece MY_WHITE_QUEEN = createwhiteQueen();
chessPiece MY_WHITE_ROOK = createWhiteRook();
chessPiece MY_WHITE_BISHOP = createWhiteBishop();
chessPiece MY_WHITE_KNIGHT = createWhiteKnight();
chessPiece MY_WHITE_PAWN = createWhitePawn();

// Black:
chessPiece MY_BLACK_KING = createBlackKing();
chessPiece MY_BLACK_QUEEN = createBlackQueen();
chessPiece MY_BLACK_ROOK = createBlackRook();
chessPiece MY_BLACK_BISHOP = createBlackBishop();
chessPiece MY_BLACK_KNIGHT = createBlackKnight();
chessPiece MY_BLACK_PAWN = createBlackPawn();

void drawChessPiece(int posX, int posY, int squareSize, int color, int pieceType) {

  int offSet = 2;
  int pieceSize = 26;
  chessPiece pieceToDraw;

  if (pieceType > 0) {
    switch (pieceType)
    {
    case 1:
      pieceToDraw = MY_WHITE_PAWN;
      break;
    case 2:
      pieceToDraw = MY_WHITE_BISHOP;
      break;
    case 3:
      pieceToDraw = MY_WHITE_KNIGHT;
      break;
    case 4:
      pieceToDraw = MY_WHITE_ROOK;
      break;
    case 5:
      pieceToDraw = MY_WHITE_QUEEN;
      break;
    case 6:
      pieceToDraw = MY_WHITE_KING;
      break;
    }
  }else if(pieceType < 0){
    pieceType = pieceType * -1;
    switch (pieceType)
    {
    case 1:
      pieceToDraw = MY_BLACK_PAWN;
      break;
    case 2:
      pieceToDraw = MY_BLACK_BISHOP;
      break;
    case 3:
      pieceToDraw = MY_BLACK_KNIGHT;
      break;
    case 4:
      pieceToDraw = MY_BLACK_ROOK;
      break;
    case 5:
      pieceToDraw = MY_BLACK_QUEEN;
      break;
    case 6:
      pieceToDraw = MY_BLACK_KING;
      break;
    }
  }

  drawSpriteWithTransparency(posX + offSet, posY + offSet, pieceToDraw.getSprite(), pieceSize, pieceSize, 0x0000);
    
}

void drawChessBoard() {
  tft.fillScreen(BLACK);

  chessSquareStartPosX = (tft.width() - (squareSize * 8)) / 2; 
  chessSquareStartPosY = (tft.height() - (squareSize * 8)) / 2;
  
  drawChessUI();

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


/*
* @brief Finds the location of the king of the given color.
* @param color The color of the king to find (1 for white, -1 for black).
* @return The location of the king in the chessBoard array (0-63), or -1 if not found.
*/
int findKingLocation(int color) {
  int targetKing = (color > 0) ? 6 : -6;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (chessBoard[i][j] == targetKing) {
        return i * 8 + j;
      }
    }
  }
  return -1;
}

/*
* @brief Checks if the square at the given index is attacked by a piece of the given color.
* @param targetIdx The index of the square to check (0-63).
* @param defenderColor The color of the defender (1 for white, -1 for black).
* @return True if the square is attacked, false otherwise.
*/
bool isSquareAttacked(int targetIdx, int defenderColor){
  int row = targetIdx / 8;
  int col = targetIdx % 8;

  for (int i = 0; i < 64; i++){
    int r = i / 8;
    int c = i % 8;
    int piece = chessBoard[r][c];

    if (piece == 0){
      continue;
    }
    bool isFriendly = (defenderColor > 0 && piece > 0) || (defenderColor < 0 && piece < 0);
    if (isFriendly){
      continue;
    }

    if(isValidMove(i, targetIdx)){
      return true;
    }
  }
  return false;
}

/*
* @brief Checks if the king of the given color is in check.
* @param color The color of the king to check (1 for white, -1 for black).
* @return True if the king is in check, false otherwise.
*/
bool isInCheck(int color){
  int kingIdx = findKingLocation(color);
  if(kingIdx == -1){
    return false;
  }
  return isSquareAttacked(kingIdx, color);
}

bool isMoveSafe(int fromIdx, int toIdx){
  int rSrc = fromIdx / 8;
  int cSrc = fromIdx % 8;

  int rDst = toIdx / 8;
  int cDst = toIdx % 8;

  int movingPiece = chessBoard[rSrc][cSrc];
  int capturedPiece = chessBoard[rDst][cDst];

  int turnColor = (movingPiece > 0) ? 1 : -1;
  int inCheck = isInCheck(turnColor);

  chessBoard[rSrc][cSrc] = movingPiece;
  chessBoard[rDst][cDst] = capturedPiece;

  return !inCheck;
}

int checkGameState(int color){
  bool hasLegalMoves = false;

  for(int src = 0; src < 64; src++){
    int r = src / 8;
    int c = src % 8;
    int piece = chessBoard[r][c];

    if (piece == 0){
      continue;
    }
    if ((color == 1 && piece < 0) || (color == -1 && piece > 0)){
      continue;
    }

    for(int dst = 0; dst < 64; dst++){
      if (src == dst){
        continue;
      }

      if(isValidMove(src, dst)){
        if(isMoveSafe(src, dst)){
          hasLegalMoves = true;
          break;
        }
      }
    }
    if(hasLegalMoves){
      break;
    }
  }
  if (hasLegalMoves)
  {
    return 0;
  }

  if(isInCheck(color)){
    return 1; // Checkmate
  }else{
    return 2; // Stalemate
  }
}

void sendRemoteMove(int fromIdx, int toIds, bool useAlt = false){
  if (useAlt){
    Serial.print("M");
    Serial.print(fromIdx);
    Serial.print(toIds);
    return;
  }
  Serial.write("M"); // M for move
  Serial.write((uint8_t)fromIdx);
  Serial.write((uint8_t)toIds); 
}

void sendWirelessMove(int fromIdx, int toIdx){
  myData.header = 'M'; // M for move
  myData.fromIdx = fromIdx;
  myData.toIdx = toIdx;
  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
}

void sendChessMove(int fromIdx, int toIdx){
  if(connectionMode == 0){
    sendRemoteMove(fromIdx, toIdx);
  }else if(connectionMode == 1){
    sendWirelessMove(fromIdx, toIdx);
  }
}

bool receiveChessMove(int &outFrom, int &outTo){
  if(connectionMode == 0){
    if (Serial.available() >= 3) {
      if(Serial.read() == 'M'){
        outFrom = Serial.read();
        outTo = Serial.read();
        return true;
      }
    }
  }else if(connectionMode == 1){
    if(wirelessMoveReceived){
      outFrom = incomingMove.fromIdx;
      outTo = incomingMove.toIdx;
      wirelessMoveReceived = false;
      return true;
    }
  }
  return false;
}


void handleChessInputs(){
  int rxFrom, rxTo;


//  if (receiveChessMove(rxFrom, rxTo) && connectionMode != 2) {
//     // A move arrived! Execute it.
//     int rSrc = rxFrom / 8; int cSrc = rxFrom % 8;
//     int rDst = rxTo / 8;   int cDst = rxTo % 8;

//     chessBoard[rDst][cDst] = chessBoard[rSrc][cSrc];
//     chessBoard[rSrc][cSrc] = 0;
    
//     // Handle Promotion
//     if (abs(chessBoard[rDst][cDst]) == 1) {
//         if (rDst == 0 || rDst == 7) chessBoard[rDst][cDst] *= 5; 
//     }
//       turnNumber++;

//       drawChessBoard();

//       int myColor = (playingAsWhite) ? 1 : -1;
//       int gameState = checkGameState(myColor);

//       if(gameState == 1){
//         chessPhase = GAME_OVER;
//       }else if(gameState == 2){
//         chessPhase = GAME_OVER;
//       }else if(isInCheck(myColor)){
//         //displayStatus("Check!", YELLOW);
//       }
//   }

  if(Serial.available() > 0){
    displayStatus("Serial Available!", YELLOW);
  }
  


  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();

  bool isWhiteTurn = (turnNumber % 2 == 0);

  if(chessPhase == WHITE_TURN || chessPhase == BLACK_TURN){
    if(isWhiteTurn){
      chessPhase = WHITE_TURN;
    }else{
      chessPhase = BLACK_TURN;
    }
  }

  if ((chessPhase == WHITE_TURN && playingAsWhite) || (chessPhase == BLACK_TURN && !playingAsWhite)) {
    // Handle Movement (Directional Buttons)
    if (currentTime - lastMoveTime >= moveDelay) {
      bool moved = false;
      if(digitalRead(PIN_UP) == LOW){
        chessBoardCursorLocation = max(0, chessBoardCursorLocation - 8);
        moved = true;
      }
      else if (digitalRead(PIN_DOWN) == LOW)
      {
        chessBoardCursorLocation = min(63, chessBoardCursorLocation + 8);
        moved = true;
      }else if (digitalRead(PIN_RIGHT) == LOW){
        chessBoardCursorLocation = min(63, chessBoardCursorLocation + 1);
        moved = true;
      }else if (digitalRead(PIN_LEFT) == LOW){
        chessBoardCursorLocation = max(0, chessBoardCursorLocation - 1);
        moved = true;
      }

      if (moved) {
        lastMoveTime = currentTime;
        drawChessCursor(chessBoardCursorLocation,chessBoardPreviousCursorLocation);
        chessBoardPreviousCursorLocation = chessBoardCursorLocation;
      }
    }


    // Handle Action (A Button)
    if (digitalRead(PIN_BUTTONA) == LOW) {
      delay(300); // Debounce

      int clickedPiece = getPieceAt(chessBoardCursorLocation);

      //Nothing selected. Try to select piece
      if(selectedSourceSquare == -1){
        // Check if selected square is own piece
        bool isOwnPiece = false;
        if(isWhiteTurn && clickedPiece > 0){
          isOwnPiece = true;
        }else if(!isWhiteTurn && clickedPiece < 0){
          isOwnPiece = true;
        }

        if(isOwnPiece){
          selectedSourceSquare = chessBoardCursorLocation;
          drawChessCursor(chessBoardCursorLocation, chessBoardCursorLocation);
        }
      }

      // Piece already selected. Try to move or deselect.
      else {
        // If clicked the same piece deselect it
        if (chessBoardCursorLocation == selectedSourceSquare) {
          selectedSourceSquare = -1;
          drawChessBoard();
          drawChessCursor(chessBoardCursorLocation, -1);
        }
        // If clicking a different square, try to move
        else {
          // 1. Check GEOMETRY (L-shape, Diagonal, etc)
          if (isValidMove(selectedSourceSquare, chessBoardCursorLocation)) {
             
             // 2. Check SAFETY (Does this put/leave me in check?)
             if (isMoveSafe(selectedSourceSquare, chessBoardCursorLocation)) {
                
                // EXECUTE MOVE
                int rSrc = selectedSourceSquare / 8;
                int cSrc = selectedSourceSquare % 8;
                int rDst = chessBoardCursorLocation / 8;
                int cDst = chessBoardCursorLocation % 8;

                chessBoard[rDst][cDst] = chessBoard[rSrc][cSrc];
                chessBoard[rSrc][cSrc] = 0;

                // --- PAWN PROMOTION (Auto-Queen for simplicity) ---
                // If a pawn reaches the last rank, turn it into a Queen (5 or -5)
                int movedPiece = chessBoard[rDst][cDst];
                if (abs(movedPiece) == 1) {
                    if ((movedPiece > 0 && rDst == 0) || (movedPiece < 0 && rDst == 7)) {
                        chessBoard[rDst][cDst] = (movedPiece > 0) ? 5 : -5;
                    }
                }
                if(connectionMode != 2){
                  sendChessMove(selectedSourceSquare, chessBoardCursorLocation);
                }
                //sendRemoteMove(selectedSourceSquare, chessBoardCursorLocation, true);
                //Serial.println("Move sent.");
                // End Turn
                
                turnNumber++;
                if(turnNumber % 2 == 0){
                  isWhiteTurn = true;
                  selectedSourceSquare = chessCursorStartLocationWhite;
                }else{
                  isWhiteTurn = false;
                  selectedSourceSquare = chessCursorStartLocationBlack;
                }
                
                drawChessBoard(); // Redraw immediately to show the move

                // --- CHECK GAME OVER STATUS ---
                // We just moved. Check the status of the OPPONENT.
                int opponentColor = (isWhiteTurn) ? -1 : 1;
                int status = checkGameState(opponentColor);

                if (status == 1) {
                    // Checkmate
                    displayStatus("CHECKMATE!", RED);
                    chessPhase = GAME_OVER;
                } else if (status == 2) {
                    // Stalemate
                    displayStatus("STALEMATE!", YELLOW);
                    chessPhase = GAME_OVER;
                } else {
                    // Game Continues
                    chessPhase = (isWhiteTurn) ? BLACK_TURN : WHITE_TURN;
                    
                    // Optional: Visual feedback if opponent is in Check
                    if (isInCheck(opponentColor)) {
                        //displayStatus("CHECK!", RED);
                    } else {
                        // Clear status bar or show turn
                        // if (opponentColor == 1) displayStatus("White's Turn", WHITE);
                        // else displayStatus("Black's Turn", WHITE);
                    }
                }
                
                drawChessCursor(chessBoardCursorLocation, -1);
             } 
             else {
                 // Move was geometrically valid, but illegal (King in check)
                 displayStatus("Invalid: King in Check", RED);
             }
          }
          else {
            // Invalid Geometry (e.g. Knight moving straight)
            // Allow changing selection to another own piece
            bool isOwnPiece = false;
            if (!isWhiteTurn && clickedPiece > 0) isOwnPiece = true; // Wait, logic reversed? 
            // Correct Logic:
            if (isWhiteTurn && clickedPiece > 0) isOwnPiece = true;
            if (!isWhiteTurn && clickedPiece < 0) isOwnPiece = true;

            if (isOwnPiece) {
              selectedSourceSquare = chessBoardCursorLocation;
              drawChessBoard();
              drawChessCursor(chessBoardCursorLocation, chessBoardCursorLocation);
            }
          }
        }
      }
    }

  }
  else if ((chessPhase == WHITE_TURN || chessPhase == BLACK_TURN) && receiveChessMove(rxFrom, rxTo)){
    int rSrc = rxFrom / 8; int cSrc = rxFrom % 8;
    int rDst = rxTo / 8;   int cDst = rxTo % 8;

    chessBoard[rDst][cDst] = chessBoard[rSrc][cSrc];
    chessBoard[rSrc][cSrc] = 0;
    
    // Handle Promotion
    if (abs(chessBoard[rDst][cDst]) == 1) {
        if (rDst == 0 || rDst == 7) chessBoard[rDst][cDst] *= 5; 
    }
      turnNumber++;

      drawChessBoard();

      int myColor = (playingAsWhite) ? 1 : -1;
      int gameState = checkGameState(myColor);

      if(gameState == 1){
        chessPhase = GAME_OVER;
      }else if(gameState == 2){
        chessPhase = GAME_OVER;
      }else if(isInCheck(myColor)){
        //displayStatus("Check!", YELLOW);
      }
  }   
  if(chessPhase == CONNECTION_SELECT){
    int previousChessMenuSelection = connMenuSelection;

    // Handle Movement (Directional Buttons)
    if (currentTime - lastMoveTime >= moveDelay) {
      bool moved = false;

      if (digitalRead(PIN_UP) == LOW) {
        connMenuSelection= max(0, connMenuSelection - 1);
        moved = true;
      } else if (digitalRead(PIN_DOWN) == LOW) {
        connMenuSelection = min(numConnMenu - 1, connMenuSelection + 1);
        moved = true;
      }

      if (moved) {
        lastMoveTime = currentTime;
        drawChessMenuCursor(previousChessMenuSelection,connMenuSelection,0);
      }
    }
    if (digitalRead(PIN_BUTTONA) == LOW) {
        connectionMode = connMenuSelection; // Set 0, 1, or 2
        chessPhase = MENU;          // Go to next screen
        tft.fillScreen(BLACK);              // Clear for next menu
        drawChessMenu(previousChessMenuSelection, chessMenuSelection, 1);
        drawChessMenuCursor(previousChessMenuSelection, 0, 1);
        delay(300);
    }
  }

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
        drawChessMenuCursor(previousChessMenuSelection,chessMenuSelection,1);
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
    
  } else if(chessPhase == START_GAME){
    // start the game and reset the move array
    chessPhase = WHITE_TURN;
    chessBoardCursorLocation = 0;
    drawChessBoard();
    drawChessCursor(chessBoardCursorLocation, -1);
    chessBoardPreviousCursorLocation = chessBoardCursorLocation;
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
    if(!gameOverScreenDrawn){
      drawChessGameOver();
    }
    if (digitalRead(PIN_SELECT) == LOW) {
      resetChess();
    }
  }
}

void resetChess(){
  gameOverScreenDrawn = false;
  turnNumber = 0;
  tft.fillScreen(BLACK);
  chessPhase = CONNECTION_SELECT;
  drawChessMenu(previousChessMenuSelection, chessMenuSelection, 0);
  drawChessMenuCursor(previousChessMenuSelection, chessMenuSelection,0);
}



// ==============================================================================
// 5. SETTINGS MENU
// ==============================================================================
int previousSettingsMenuSelection = 0;
int settingsMenuSelection = 0;
int numSettingsMenu = 2;
const char* settingsMenu[2] = {"Volume", "Coming Soon..."};


void drawSettingsMenuCursor(int previousSelection, int currentSelection){
  tft.fillRect(0, previousSelection * 40, tft.width(), 40, BLACK);
  tft.setCursor(0, currentSelection * 40);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.println(settingsMenu[currentSelection]);
  tft.setTextColor(WHITE);
  previousSettingsMenuSelection = currentSelection;
}

void drawSettingsMenu(int previousSelection, int currentSelection){
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);

  for (int i = 0; i < numSettingsMenu; i++) {
    if (i == currentSelection) {
      tft.setTextColor(YELLOW);
    } else {
      tft.setTextColor(WHITE);
    }
    tft.println(settingsMenu[i]);
  }
  tft.setTextColor(WHITE);
  previousSettingsMenuSelection = currentSelection;
}





void handleSettingsInput(){
  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();
  int prevSelection = menuSelection;

  if(currentTime - lastMoveTime >= moveDelay){
    bool moved = false;
  
    if (digitalRead(PIN_UP) == LOW) {
      settingsMenuSelection = max(0, settingsMenuSelection - 1);
      moved = true;
    } else if (digitalRead(PIN_DOWN) == LOW) {
      settingsMenuSelection = min(numSettingsMenu - 1, settingsMenuSelection + 1);
      moved = true;
    }
    else if (digitalRead(PIN_BUTTONB) == LOW) {
      // Go home
      //goHome();
      return;
    }

    if (moved) {
      lastMoveTime = currentTime;
      drawSettingsMenuCursor(previousSettingsMenuSelection, settingsMenuSelection);
    }
  }
}



void resetSettings(){
  tft.fillScreen(BLACK);
  drawSettingsMenu(previousSettingsMenuSelection, settingsMenuSelection);
  drawSettingsMenuCursor(previousSettingsMenuSelection, settingsMenuSelection);
}


// ==============================================================================
// 6. ARDUINO SETUP & LOOP
// ==============================================================================

void setup() {
  
  //Serial2.begin(SERIAL_BAUD_RATE, SERIAL_8N1, SERIAL_RX_PIN, SERIAL_TX_PIN);
  //Serial.println("Starting up...");
  // Turn power indicator pin on
  // pinMode(PIN_POWER_INDICATOR, OUTPUT);
  // digitalWrite(PIN_POWER_INDICATOR, HIGH);

  
  // // initialize wireless
  // WiFi.mode(WIFI_STA);
  // if(esp_now_init() == ESP_OK){
  //   esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  //   esp_now_peer_info_t peerInfo;
  //   memset(&peerInfo, 0, sizeof(peerInfo));
  //   memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  //   peerInfo.channel = 0;
  //   peerInfo.encrypt = false;
  //   esp_now_add_peer(&peerInfo);
  // }else{
  //   Serial.println("Error initializing ESP-NOW");
  // }

  // Initialize display
  tft.begin();
  delay(100);
  Serial.begin(SERIAL_BAUD_RATE);
  // Most ILI9341 screens are 240x320. Rotation(1) makes it 320x240 (landscape)
  tft.setRotation(3);

  // Initialize buttons with internal pull-up resistors
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_HOME, INPUT_PULLUP);
  pinMode(PIN_BUTTONA, INPUT_PULLUP);
  pinMode(PIN_BUTTONB, INPUT_PULLUP);

  // Volume potentiometer
  // pinMode(PIN_VOLUME, INPUT);
  // pinMode(PIN_PIEZO, OUTPUT);
  //initAudio();
  // Start the device in the main menu
  drawMenu();
  drawMenuCursor(-1, 0); // Draw cursor on the first item
  //chessSelected();
  Serial.println("Starting up...");
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
    case STATE_SETTINGS:
      handleSettingsInput();
      break;
  }
  //delay(10);
}