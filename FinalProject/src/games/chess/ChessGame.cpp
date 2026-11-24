#include "ChessGame.h"
#include "../../sdk/BoardConfig.h"
#include "assets/icon.h"
#include <array>
#include <esp_now.h>

// Chess piece sprites & classes
#include "chessPiece.h"
#include "assets/White_King.h"
#include "assets/White_Queen.h"
#include "assets/White_Rook.h"
#include "assets/White_Bishop.h"
#include "assets/White_Knight.h"
#include "assets/White_Pawn.h"
#include "assets/Black_King.h"
#include "assets/Black_Queen.h"
#include "assets/Black_Rook.h"
#include "assets/Black_Bishop.h"
#include "assets/Black_Knight.h"
#include "assets/Black_Pawn.h"

namespace {

constexpr uint16_t BLACK                   = 0x0000;
constexpr uint16_t WHITE                   = 0xFFFF;
constexpr uint16_t RED                     = 0xF800;
constexpr uint16_t YELLOW                  = 0xFFE0;
constexpr uint16_t CURSOR_COLOR            = 0xF800;
constexpr uint16_t HP_GREEN                = 0x2624;
constexpr uint16_t CHESS_BOARD_LIGHT_COLOR = 0xDEFB;
constexpr uint16_t CHESS_BOARD_DARK_COLOR  = 0x7D1F;

sdk::Console* gConsole = nullptr;
Adafruit_ILI9341* gTft = nullptr;

// Wireless / Serial link state
int connectionMode = 0; // 0: Serial, 1: ESP-NOW, 2: Single Player
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

struct ChessMessage {
  char header; // 'M' for move
  int fromIdx;
  int toIdx;
};

ChessMessage myData = {};
ChessMessage incomingMove = {};
volatile bool wirelessMoveReceived = false;

enum ChessPhase {
  CONNECTION_SELECT,
  MENU,
  START_GAME,
  WHITE_TURN,
  BLACK_TURN,
  GAME_OVER
};

ChessPhase chessPhase = MENU;
int turnNumber = 0; // Even = White, Odd = Black
int chessMenuSelection = 0;
int previousChessMenuSelection = -1;

const char* chessMenu[] = {"Play as White", "Play as Black"};
constexpr int numChessMenu = 2;

const char* connMenu[] = {"Wired", "Wireless", "Single Player"};
constexpr int numConnMenu = 3;
int connMenuSelection = 0;

bool playingAsWhite = false;
bool gameOverScreenDrawn = false;

const std::array<std::array<int, 8>, 8> INITIAL_BOARD = {{
  {-4, -3, -2, -5, -6, -2, -3, -4},
  {-1, -1, -1, -1, -1, -1, -1, -1},
  { 0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0},
  { 0,  0,  0,  0,  0,  0,  0,  0},
  { 1,  1,  1,  1,  1,  1,  1,  1},
  { 4,  3,  2,  5,  6,  2,  3,  4},
}};

std::array<std::array<int, 8>, 8> chessBoard = INITIAL_BOARD;

int chessBoardCursorLocation = 0;
int chessBoardPreviousCursorLocation = -1;
int selectedSourceSquare = -1;

int boardHeight = SCREEN_HEIGHT;
constexpr int squareSize = 30;
int chessSquareStartPosX = (SCREEN_WIDTH - (squareSize * 8)) / 2;
int chessSquareStartPosY = (SCREEN_HEIGHT - (squareSize * 8)) / 2;

constexpr int chessUIWidthMargin = 2;
int chessUIHeight = SCREEN_HEIGHT;
constexpr int chessUIWidth = 40 - chessUIWidthMargin;
constexpr int chessUIStartingX = 0;
constexpr int chessUIStartingY = 0;

constexpr int chessCursorStartLocationWhite = 0;
constexpr int chessCursorStartLocationBlack = 63;

// Chess piece sprite wrappers
chessPiece MY_WHITE_KING   = createWhiteKing();
chessPiece MY_WHITE_QUEEN  = createwhiteQueen();
chessPiece MY_WHITE_ROOK   = createWhiteRook();
chessPiece MY_WHITE_BISHOP = createWhiteBishop();
chessPiece MY_WHITE_KNIGHT = createWhiteKnight();
chessPiece MY_WHITE_PAWN   = createWhitePawn();

chessPiece MY_BLACK_KING   = createBlackKing();
chessPiece MY_BLACK_QUEEN  = createBlackQueen();
chessPiece MY_BLACK_ROOK   = createBlackRook();
chessPiece MY_BLACK_BISHOP = createBlackBishop();
chessPiece MY_BLACK_KNIGHT = createBlackKnight();
chessPiece MY_BLACK_PAWN   = createBlackPawn();

bool isInCheck(int color);
void resetChess();

void displayChessStatus(const char* message, uint16_t color) {
  auto& tft = *gTft;
  const int statusY = 220;
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(color, BLACK);
  tft.fillRect(10, statusY, tft.width() - 20, 20, BLACK);
  tft.setCursor(10, statusY);
  tft.print(message);
}

int getPieceAt(int index) {
  return chessBoard[index / 8][index % 8];
}

bool isPathClear(int startIdx, int endIdx) {
  int startRow = startIdx / 8;
  int startCol = startIdx % 8;
  int endRow = endIdx / 8;
  int endCol = endIdx % 8;

  int dRow = (endRow - startRow) == 0 ? 0 : ((endRow - startRow) > 0 ? 1 : -1);
  int dCol = (endCol - startCol) == 0 ? 0 : ((endCol - startCol) > 0 ? 1 : -1);

  int currentRow = startRow + dRow;
  int currentCol = startCol + dCol;

  while (currentRow != endRow || currentCol != endCol) {
    if (chessBoard[currentRow][currentCol] != 0) {
      return false;
    }
    currentRow += dRow;
    currentCol += dCol;
  }
  return true;
}

bool isValidMove(int fromIdx, int toIdx) {
  int piece = getPieceAt(fromIdx);
  int target = getPieceAt(toIdx);

  if (target != 0) {
    bool sameColor = (piece > 0 && target > 0) || (piece < 0 && target < 0);
    if (sameColor) return false;
  }

  int startRow = fromIdx / 8;
  int startCol = fromIdx % 8;
  int endRow = toIdx / 8;
  int endCol = toIdx % 8;
  int dRow = endRow - startRow;
  int dCol = endCol - startCol;
  int absRow = abs(dRow);
  int absCol = abs(dCol);
  int type = abs(piece);

  switch (type) {
    case 1: { // Pawns
      int direction = (piece > 0) ? -1 : 1;
      int startRowLimit = (piece > 0) ? 6 : 1;
      if (dCol == 0 && dRow == direction && target == 0) return true;
      if (dCol == 0 && dRow == direction * 2 && startRow == startRowLimit && target == 0) {
        if (chessBoard[startRow + direction][startCol] == 0) return true;
      }
      if (absCol == 1 && dRow == direction && target != 0) return true;
      return false;
    }
    case 2: // Bishops
      return (absRow == absCol) ? isPathClear(fromIdx, toIdx) : false;
    case 3: // Knights
      return (absRow == 2 && absCol == 1) || (absRow == 1 && absCol == 2);
    case 4: // Rooks
      return (dRow == 0 || dCol == 0) ? isPathClear(fromIdx, toIdx) : false;
    case 5: // Queens
      return (dRow == 0 || dCol == 0 || absRow == absCol) ? isPathClear(fromIdx, toIdx) : false;
    case 6: // Kings
      return (absRow <= 1 && absCol <= 1);
  }
  return false;
}

void drawChessUI() {
  auto& tft = *gTft;
  tft.fillRect(chessUIStartingX, chessUIStartingY, chessUIWidth, chessUIHeight, WHITE);

  int leftJustified = chessUIStartingX + 2;
  tft.setFont();
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  tft.setCursor(leftJustified, chessUIStartingY + 2);
  tft.print("Turn:");
  tft.setCursor(leftJustified, chessUIStartingY + 12);
  tft.print((turnNumber % 2 == 0) ? "White" : "Black");

  if (isInCheck(1) || isInCheck(-1)) {
    tft.setTextColor(RED);
    tft.setCursor(leftJustified, chessUIStartingY + 24);
    tft.print("Check!");
  }
}

void drawChessMenu(int menuType) {
  auto& tft = *gTft;
  tft.fillScreen(BLACK);
  tft.setFont();
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(50, 20);
  tft.print("CHESS");

  tft.setTextSize(2);
  if (menuType == 0) {
    for (int i = 0; i < numConnMenu; i++) {
      tft.setCursor(50, 80 + i * 40);
      tft.setTextColor(WHITE);
      tft.print(connMenu[i]);
    }
  } else {
    for (int i = 0; i < numChessMenu; i++) {
      tft.setCursor(50, 80 + i * 40);
      tft.setTextColor(WHITE);
      tft.print(chessMenu[i]);
    }
  }
}

void drawChessMenuCursor(int prevSelection, int newSelection, int menuType) {
  auto& tft = *gTft;
  int xPos = 40;
  int yStart = 80;
  int height = 20;
  int width = tft.width() - 80;
  int maxItems = (menuType == 0) ? numConnMenu : numChessMenu;

  if (prevSelection >= 0 && prevSelection < maxItems) {
    int prevYPos = yStart + prevSelection * 40;
    tft.fillRect(xPos, prevYPos, width, height, BLACK);
    tft.setCursor(50, prevYPos);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print((menuType == 0) ? connMenu[prevSelection] : chessMenu[prevSelection]);
  }

  int newYPos = yStart + newSelection * 40;
  tft.fillRect(xPos, newYPos, width, height, CURSOR_COLOR);
  tft.setCursor(50, newYPos);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print((menuType == 0) ? connMenu[newSelection] : chessMenu[newSelection]);
}

void drawChessGameOver() {
  auto& tft = *gTft;
  gameOverScreenDrawn = true;
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.setCursor(50, 20);
  tft.print("Game Over");
  tft.setTextSize(1);
  tft.setCursor(50, 70);
  tft.print("Press SELECT to Play Again");
}

int getChessSquareLocationX(int square) {
  return chessSquareStartPosX + (square % 8) * squareSize;
}

int getChessSquareLocationY(int square) {
  return chessSquareStartPosY + (square / 8) * squareSize;
}

uint16_t getChessSquareColor(int square) {
  int row = square / 8;
  int col = square % 8;
  return ((row + col) % 2 == 0) ? CHESS_BOARD_LIGHT_COLOR : CHESS_BOARD_DARK_COLOR;
}

void drawChessCursor(int currentPosition, int previousPosition) {
  auto& tft = *gTft;
  if (previousPosition >= 0) {
    uint16_t previousColor = getChessSquareColor(previousPosition);
    int prevX = getChessSquareLocationX(previousPosition);
    int prevY = getChessSquareLocationY(previousPosition);
    tft.drawRect(prevX, prevY, squareSize, squareSize, previousColor);
  }

  int sqX = getChessSquareLocationX(currentPosition);
  int sqY = getChessSquareLocationY(currentPosition);
  tft.drawRect(sqX, sqY, squareSize, squareSize, CURSOR_COLOR);

  if (selectedSourceSquare != -1) {
    int selX = getChessSquareLocationX(selectedSourceSquare);
    int selY = getChessSquareLocationY(selectedSourceSquare);
    tft.drawRect(selX, selY, squareSize, squareSize, HP_GREEN);
  }
}

void drawChessPiece(int posX, int posY, int pieceType) {
  constexpr int offSet = 2;
  constexpr int pieceSize = 26;
  chessPiece pieceToDraw;

  if (pieceType > 0) {
    switch (pieceType) {
      case 1: pieceToDraw = MY_WHITE_PAWN;   break;
      case 2: pieceToDraw = MY_WHITE_BISHOP; break;
      case 3: pieceToDraw = MY_WHITE_KNIGHT; break;
      case 4: pieceToDraw = MY_WHITE_ROOK;   break;
      case 5: pieceToDraw = MY_WHITE_QUEEN;  break;
      case 6: pieceToDraw = MY_WHITE_KING;   break;
      default: return;
    }
  } else if (pieceType < 0) {
    switch (-pieceType) {
      case 1: pieceToDraw = MY_BLACK_PAWN;   break;
      case 2: pieceToDraw = MY_BLACK_BISHOP; break;
      case 3: pieceToDraw = MY_BLACK_KNIGHT; break;
      case 4: pieceToDraw = MY_BLACK_ROOK;   break;
      case 5: pieceToDraw = MY_BLACK_QUEEN;  break;
      case 6: pieceToDraw = MY_BLACK_KING;   break;
      default: return;
    }
  } else {
    return;
  }

  sdk::Sprite spr = { pieceToDraw.getSprite(), pieceSize, pieceSize, 0x0000, true };
  gConsole->display.drawSprite(spr, posX + offSet, posY + offSet, 1);
}

void drawChessBoard() {
  auto& tft = *gTft;
  tft.fillScreen(BLACK);

  boardHeight = tft.height();
  chessUIHeight = boardHeight;
  chessSquareStartPosX = (tft.width() - (squareSize * 8)) / 2;
  chessSquareStartPosY = (tft.height() - (squareSize * 8)) / 2;

  drawChessUI();

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      int x = chessSquareStartPosX + (j * squareSize);
      int y = chessSquareStartPosY + (i * squareSize);
      uint16_t color = ((i + j) % 2 == 0) ? CHESS_BOARD_LIGHT_COLOR : CHESS_BOARD_DARK_COLOR;
      tft.fillRect(x, y, squareSize, squareSize, color);

      int squareValue = chessBoard[i][j];
      if (squareValue != 0) {
        drawChessPiece(x, y, squareValue);
      }
    }
  }
}

int findKingLocation(int color) {
  int targetKing = (color > 0) ? 6 : -6;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      if (chessBoard[i][j] == targetKing) return i * 8 + j;
    }
  }
  return -1;
}

bool isSquareAttacked(int targetIdx, int defenderColor) {
  for (int i = 0; i < 64; i++) {
    int piece = chessBoard[i / 8][i % 8];
    if (piece == 0) continue;
    bool isFriendly = (defenderColor > 0 && piece > 0) || (defenderColor < 0 && piece < 0);
    if (isFriendly) continue;
    if (isValidMove(i, targetIdx)) return true;
  }
  return false;
}

bool isInCheck(int color) {
  int kingIdx = findKingLocation(color);
  if (kingIdx == -1) return false;
  return isSquareAttacked(kingIdx, color);
}

bool isMoveSafe(int fromIdx, int toIdx) {
  int rSrc = fromIdx / 8, cSrc = fromIdx % 8;
  int rDst = toIdx / 8, cDst = toIdx % 8;

  int movingPiece = chessBoard[rSrc][cSrc];
  int capturedPiece = chessBoard[rDst][cDst];

  chessBoard[rDst][cDst] = movingPiece;
  chessBoard[rSrc][cSrc] = 0;

  int turnColor = (movingPiece > 0) ? 1 : -1;
  bool inCheck = isInCheck(turnColor);

  chessBoard[rSrc][cSrc] = movingPiece;
  chessBoard[rDst][cDst] = capturedPiece;

  return !inCheck;
}

int checkGameState(int color) {
  for (int src = 0; src < 64; src++) {
    int piece = chessBoard[src / 8][src % 8];
    if (piece == 0) continue;
    if ((color == 1 && piece < 0) || (color == -1 && piece > 0)) continue;

    for (int dst = 0; dst < 64; dst++) {
      if (src == dst) continue;
      if (isValidMove(src, dst) && isMoveSafe(src, dst)) {
        return 0; // Has legal move
      }
    }
  }
  return isInCheck(color) ? 1 : 2; // 1 = Checkmate, 2 = Stalemate
}

void sendRemoteMove(int fromIdx, int toIdx, bool useAlt = false) {
  if (useAlt) {
    Serial.print("M");
    Serial.print(fromIdx);
    Serial.print(toIdx);
    return;
  }
  Serial.write("M");
  Serial.write((uint8_t)fromIdx);
  Serial.write((uint8_t)toIdx);
}

void sendWirelessMove(int fromIdx, int toIdx) {
  myData.header = 'M';
  myData.fromIdx = fromIdx;
  myData.toIdx = toIdx;
  esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
}

void sendChessMove(int fromIdx, int toIdx) {
  if (connectionMode == 0) {
    sendRemoteMove(fromIdx, toIdx);
  } else if (connectionMode == 1) {
    sendWirelessMove(fromIdx, toIdx);
  }
}

bool receiveChessMove(int &outFrom, int &outTo) {
  if (connectionMode == 0) {
    if (Serial.available() >= 3 && Serial.read() == 'M') {
      outFrom = Serial.read();
      outTo = Serial.read();
      return true;
    }
  } else if (connectionMode == 1) {
    if (wirelessMoveReceived) {
      outFrom = incomingMove.fromIdx;
      outTo = incomingMove.toIdx;
      wirelessMoveReceived = false;
      return true;
    }
  }
  return false;
}

void resetChess() {
  gameOverScreenDrawn = false;
  turnNumber = 0;
  selectedSourceSquare = -1;
  chessBoardCursorLocation = 0;
  chessBoardPreviousCursorLocation = -1;
  connMenuSelection = 0;
  chessMenuSelection = 0;
  previousChessMenuSelection = -1;
  chessBoard = INITIAL_BOARD;

  gTft->fillScreen(BLACK);
  chessPhase = CONNECTION_SELECT;
  drawChessMenu(0);
  drawChessMenuCursor(-1, connMenuSelection, 0);
}

} // namespace

sdk::Sprite ChessGame::icon() const {
  return CHESS_ICON;
}

void ChessGame::onEnter(sdk::Console& console) {
  gConsole = &console;
  gTft = &console.display.rawTft();
  resetChess();
}

void ChessGame::onUpdate(sdk::Console& console, float dt) {
  (void)console;
  (void)dt;
  auto& tft = *gTft;

  int rxFrom = 0, rxTo = 0;

  if (Serial.available() > 0) {
    displayChessStatus("Serial Available!", YELLOW);
  }

  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 200;
  unsigned long currentTime = millis();

  bool isWhiteTurn = (turnNumber % 2 == 0);
  if (chessPhase == WHITE_TURN || chessPhase == BLACK_TURN) {
    chessPhase = isWhiteTurn ? WHITE_TURN : BLACK_TURN;
  }

  if (chessPhase == WHITE_TURN || chessPhase == BLACK_TURN) {
    if (currentTime - lastMoveTime >= moveDelay) {
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

    if (digitalRead(PIN_BUTTONA) == LOW) {
      delay(300);
      int clickedPiece = getPieceAt(chessBoardCursorLocation);

      if (selectedSourceSquare == -1) {
        bool isOwnPiece = (isWhiteTurn && clickedPiece > 0) || (!isWhiteTurn && clickedPiece < 0);
        if (isOwnPiece) {
          selectedSourceSquare = chessBoardCursorLocation;
          drawChessCursor(chessBoardCursorLocation, chessBoardCursorLocation);
        }
      } else {
        if (chessBoardCursorLocation == selectedSourceSquare) {
          selectedSourceSquare = -1;
          drawChessBoard();
          drawChessCursor(chessBoardCursorLocation, -1);
        } else {
          if (isValidMove(selectedSourceSquare, chessBoardCursorLocation)) {
            if (isMoveSafe(selectedSourceSquare, chessBoardCursorLocation)) {
              int rSrc = selectedSourceSquare / 8, cSrc = selectedSourceSquare % 8;
              int rDst = chessBoardCursorLocation / 8, cDst = chessBoardCursorLocation % 8;

              chessBoard[rDst][cDst] = chessBoard[rSrc][cSrc];
              chessBoard[rSrc][cSrc] = 0;

              int movedPiece = chessBoard[rDst][cDst];
              if (abs(movedPiece) == 1) {
                if ((movedPiece > 0 && rDst == 0) || (movedPiece < 0 && rDst == 7)) {
                  chessBoard[rDst][cDst] = (movedPiece > 0) ? 5 : -5;
                }
              }

              if (connectionMode != 2) {
                sendChessMove(selectedSourceSquare, chessBoardCursorLocation);
              }

              turnNumber++;
              selectedSourceSquare = -1;
              isWhiteTurn = (turnNumber % 2 == 0);
              chessBoardCursorLocation = isWhiteTurn ? chessCursorStartLocationWhite : chessCursorStartLocationBlack;

              drawChessBoard();

              int nextPlayerColor = isWhiteTurn ? 1 : -1;
              int status = checkGameState(nextPlayerColor);

              if (status == 1) {
                displayChessStatus("CHECKMATE!", RED);
                chessPhase = GAME_OVER;
              } else if (status == 2) {
                displayChessStatus("STALEMATE!", YELLOW);
                chessPhase = GAME_OVER;
              } else {
                chessPhase = isWhiteTurn ? WHITE_TURN : BLACK_TURN;
              }

              drawChessCursor(chessBoardCursorLocation, -1);
              chessBoardPreviousCursorLocation = chessBoardCursorLocation;
            } else {
              displayChessStatus("Invalid: King in Check", RED);
            }
          } else {
            bool isOwnPiece = (isWhiteTurn && clickedPiece > 0) || (!isWhiteTurn && clickedPiece < 0);
            if (isOwnPiece) {
              selectedSourceSquare = chessBoardCursorLocation;
              drawChessBoard();
              drawChessCursor(chessBoardCursorLocation, chessBoardCursorLocation);
            }
          }
        }
      }
    }
  } else if ((chessPhase == WHITE_TURN || chessPhase == BLACK_TURN) && receiveChessMove(rxFrom, rxTo)) {
    int rSrc = rxFrom / 8, cSrc = rxFrom % 8;
    int rDst = rxTo / 8, cDst = rxTo % 8;
    chessBoard[rDst][cDst] = chessBoard[rSrc][cSrc];
    chessBoard[rSrc][cSrc] = 0;
    if (abs(chessBoard[rDst][cDst]) == 1 && (rDst == 0 || rDst == 7)) {
      chessBoard[rDst][cDst] *= 5;
    }
    turnNumber++;
    drawChessBoard();

    int myColor = playingAsWhite ? 1 : -1;
    int status = checkGameState(myColor);
    if (status == 1 || status == 2) {
      chessPhase = GAME_OVER;
    }
  }

  if (chessPhase == CONNECTION_SELECT) {
    int prevSel = connMenuSelection;
    if (currentTime - lastMoveTime >= moveDelay) {
      bool moved = false;
      if (digitalRead(PIN_UP) == LOW) {
        connMenuSelection = max(0, connMenuSelection - 1);
        moved = true;
      } else if (digitalRead(PIN_DOWN) == LOW) {
        connMenuSelection = min(numConnMenu - 1, connMenuSelection + 1);
        moved = true;
      }
      if (moved) {
        lastMoveTime = currentTime;
        drawChessMenuCursor(prevSel, connMenuSelection, 0);
      }
    }
    if (digitalRead(PIN_BUTTONA) == LOW) {
      connectionMode = connMenuSelection;
      chessPhase = MENU;
      tft.fillScreen(BLACK);
      drawChessMenu(1);
      drawChessMenuCursor(-1, chessMenuSelection, 1);
      delay(300);
    }
  } else if (chessPhase == MENU) {
    int prevSel = chessMenuSelection;
    if (currentTime - lastMoveTime >= moveDelay) {
      bool moved = false;
      if (digitalRead(PIN_UP) == LOW) {
        chessMenuSelection = max(0, chessMenuSelection - 1);
        moved = true;
      } else if (digitalRead(PIN_DOWN) == LOW) {
        chessMenuSelection = min(numChessMenu - 1, chessMenuSelection + 1);
        moved = true;
      }
      if (moved) {
        lastMoveTime = currentTime;
        drawChessMenuCursor(prevSel, chessMenuSelection, 1);
      }
    }
    if (digitalRead(PIN_BUTTONA) == LOW) {
      playingAsWhite = (chessMenuSelection == 0);
      chessPhase = START_GAME;
      delay(300);
    }
  } else if (chessPhase == START_GAME) {
    chessPhase = WHITE_TURN;
    chessBoardCursorLocation = 0;
    drawChessBoard();
    drawChessCursor(chessBoardCursorLocation, -1);
    chessBoardPreviousCursorLocation = chessBoardCursorLocation;
  } else if (chessPhase == GAME_OVER) {
    if (!gameOverScreenDrawn) {
      drawChessGameOver();
    }
    if (digitalRead(PIN_SELECT) == LOW) {
      resetChess();
      delay(300);
    }
  }
}
