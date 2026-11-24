#include "TicTacToeGame.h"
#include "../../sdk/BoardConfig.h"
#include "assets/icon.h"

namespace {

constexpr uint16_t BLACK   = 0x0000;
constexpr uint16_t WHITE   = 0xFFFF;
constexpr uint16_t RED     = 0xF800;
constexpr uint16_t YELLOW  = 0xFFE0;

constexpr int BOARD_SIZE = 3;
constexpr int EMPTY      = 0;
constexpr int PLAYER_X   = 1;
constexpr int PLAYER_O   = 2;

constexpr int TTT_START_X    = 73;
constexpr int TTT_START_Y    = 38;
constexpr int TTT_CELL_SIZE  = 58;
constexpr uint16_t TTT_PANEL_BG   = 0x18E3;
constexpr uint16_t TTT_BORDER_COL = 0x4208;
constexpr uint16_t TTT_O_COLOR    = 0x2D7F;

Adafruit_ILI9341* gTft = nullptr;

int board[BOARD_SIZE][BOARD_SIZE] = {};
int currentPlayer = PLAYER_X;
int gameStatus = 0; // 0: Running, 1: X Win, 2: O Win, 3: Draw
bool gameOverScreenDrawn = false;
int tttXWins = 0;
int tttOWins = 0;
int tttDraws = 0;
int cursorX = 0;
int cursorY = 0;

void drawTicTacToeTurnBadge(const char* label, uint16_t accentColor) {
  auto& tft = *gTft;
  int badgeX = 204;
  int badgeY = 4;
  int badgeW = 110;
  int badgeH = 20;

  tft.fillRect(badgeX, badgeY, badgeW, badgeH, BLACK);
  tft.drawRect(badgeX, badgeY, badgeW, badgeH, accentColor);
  tft.fillRect(badgeX, badgeY, 6, badgeH, accentColor);

  tft.setFont();
  tft.setTextSize(1);
  tft.setTextColor(accentColor);
  tft.setCursor(badgeX + 14, badgeY + 6);
  tft.print(label);
}

void drawTicTacToeFooter(const char* text, uint16_t textColor) {
  auto& tft = *gTft;
  tft.fillRect(0, 220, 320, 20, TTT_PANEL_BG);
  tft.drawFastHLine(0, 220, 320, TTT_BORDER_COL);
  tft.setFont();
  tft.setTextSize(1);
  tft.setTextColor(textColor);
  int textLen = strlen(text);
  int xPos = max(4, (320 - textLen * 6) / 2);
  tft.setCursor(xPos, 227);
  tft.print(text);
}

void drawTicTacToeScorePanels() {
  auto& tft = *gTft;
  int leftX = 6;
  int rightX = 256;
  int cardW = 58;

  // Left Card: Player X Wins
  tft.fillRect(leftX, 42, cardW, 76, TTT_PANEL_BG);
  tft.drawRect(leftX, 42, cardW, 76, TTT_BORDER_COL);
  tft.fillRect(leftX + 1, 43, cardW - 2, 16, RED);
  tft.setFont();
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(leftX + 7, 47);
  tft.print("PLAYER X");
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.setCursor(leftX + (tttXWins >= 10 ? 12 : 21), 72);
  tft.print(tttXWins);

  // Left Mini Card: Draws
  tft.fillRect(leftX, 130, cardW, 64, TTT_PANEL_BG);
  tft.drawRect(leftX, 130, cardW, 64, TTT_BORDER_COL);
  tft.fillRect(leftX + 1, 131, cardW - 2, 14, TTT_BORDER_COL);
  tft.setTextSize(1);
  tft.setTextColor(YELLOW);
  tft.setCursor(leftX + 14, 134);
  tft.print("DRAWS");
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(leftX + (tttDraws >= 10 ? 18 : 24), 156);
  tft.print(tttDraws);

  // Right Card: Player O Wins
  tft.fillRect(rightX, 42, cardW, 76, TTT_PANEL_BG);
  tft.drawRect(rightX, 42, cardW, 76, TTT_BORDER_COL);
  tft.fillRect(rightX + 1, 43, cardW - 2, 16, TTT_O_COLOR);
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.setCursor(rightX + 7, 47);
  tft.print("PLAYER O");
  tft.setTextSize(3);
  tft.setTextColor(WHITE);
  tft.setCursor(rightX + (tttOWins >= 10 ? 12 : 21), 72);
  tft.print(tttOWins);

  // Right Mini Card: Match Number
  int matchNum = tttXWins + tttOWins + tttDraws + (gameStatus == 0 ? 1 : 0);
  tft.fillRect(rightX, 130, cardW, 64, TTT_PANEL_BG);
  tft.drawRect(rightX, 130, cardW, 64, TTT_BORDER_COL);
  tft.fillRect(rightX + 1, 131, cardW - 2, 14, TTT_BORDER_COL);
  tft.setTextSize(1);
  tft.setTextColor(0xBDF7);
  tft.setCursor(rightX + 14, 134);
  tft.print("MATCH");
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(rightX + (matchNum >= 10 ? 18 : 24), 156);
  tft.print(matchNum);
}

void drawBoard() {
  auto& tft = *gTft;
  tft.fillScreen(BLACK);

  // Top Bar
  tft.fillRect(0, 0, 320, 28, TTT_PANEL_BG);
  tft.drawFastHLine(0, 28, 320, TTT_BORDER_COL);
  tft.setFont();
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(10, 7);
  tft.print("TIC-TAC-TOE");

  // Centered Board Frame & Grid
  int startX = TTT_START_X;
  int startY = TTT_START_Y;
  int cellSize = TTT_CELL_SIZE;
  int boardDim = cellSize * BOARD_SIZE;

  tft.drawRect(startX - 3, startY - 3, boardDim + 6, boardDim + 6, TTT_BORDER_COL);
  tft.drawRect(startX - 2, startY - 2, boardDim + 4, boardDim + 4, TTT_BORDER_COL);

  for (int i = 1; i < BOARD_SIZE; i++) {
    tft.fillRect(startX + i * cellSize - 1, startY + 2, 3, boardDim - 4, WHITE);
    tft.fillRect(startX + 2, startY + i * cellSize - 1, boardDim - 4, 3, WHITE);
  }

  drawTicTacToeScorePanels();
  drawTicTacToeTurnBadge("X'S TURN", RED);
  drawTicTacToeFooter("D-PAD: Move   A: Place   HOME: Menu", 0xBDF7);
}

void drawMarker(int col, int row, int player) {
  auto& tft = *gTft;
  int centerX = TTT_START_X + col * TTT_CELL_SIZE + TTT_CELL_SIZE / 2;
  int centerY = TTT_START_Y + row * TTT_CELL_SIZE + TTT_CELL_SIZE / 2;
  int size = TTT_CELL_SIZE / 3 - 2;

  if (player == PLAYER_X) {
    for (int offset = -1; offset <= 1; offset++) {
      tft.drawLine(centerX - size + offset, centerY - size, centerX + size + offset, centerY + size, RED);
      tft.drawLine(centerX - size + offset, centerY + size, centerX + size + offset, centerY - size, RED);
    }
  } else if (player == PLAYER_O) {
    tft.fillCircle(centerX, centerY, size + 1, TTT_O_COLOR);
    tft.fillCircle(centerX, centerY, size - 2, BLACK);
  }
}

void drawCursor(int prevX, int prevY, int newX, int newY) {
  auto& tft = *gTft;
  int margin = 4;

  // Erase previous cursor
  tft.drawRect(TTT_START_X + prevX * TTT_CELL_SIZE + margin,
               TTT_START_Y + prevY * TTT_CELL_SIZE + margin,
               TTT_CELL_SIZE - 2 * margin, TTT_CELL_SIZE - 2 * margin, BLACK);
  tft.drawRect(TTT_START_X + prevX * TTT_CELL_SIZE + margin + 1,
               TTT_START_Y + prevY * TTT_CELL_SIZE + margin + 1,
               TTT_CELL_SIZE - 2 * (margin + 1), TTT_CELL_SIZE - 2 * (margin + 1), BLACK);

  // Draw new cursor if running
  if (gameStatus == 0) {
    tft.drawRect(TTT_START_X + newX * TTT_CELL_SIZE + margin,
                 TTT_START_Y + newY * TTT_CELL_SIZE + margin,
                 TTT_CELL_SIZE - 2 * margin, TTT_CELL_SIZE - 2 * margin, YELLOW);
    tft.drawRect(TTT_START_X + newX * TTT_CELL_SIZE + margin + 1,
                 TTT_START_Y + newY * TTT_CELL_SIZE + margin + 1,
                 TTT_CELL_SIZE - 2 * (margin + 1), TTT_CELL_SIZE - 2 * (margin + 1), YELLOW);
  }
}

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
  drawCursor(0, 0, 0, 0);
}

bool checkWin(int player) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    if ((board[i][0] == player && board[i][1] == player && board[i][2] == player) ||
        (board[0][i] == player && board[1][i] == player && board[2][i] == player)) {
      return true;
    }
  }
  if ((board[0][0] == player && board[1][1] == player && board[2][2] == player) ||
      (board[0][2] == player && board[1][1] == player && board[2][0] == player)) {
    return true;
  }
  return false;
}

bool checkDraw() {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      if (board[i][j] == EMPTY) return false;
    }
  }
  return true;
}

void drawTicTacToeWinningLine(int player) {
  auto& tft = *gTft;
  int pad = 10;
  uint16_t lineColor = YELLOW;

  for (int i = 0; i < BOARD_SIZE; i++) {
    if (board[i][0] == player && board[i][1] == player && board[i][2] == player) {
      int cx = TTT_START_X + i * TTT_CELL_SIZE + TTT_CELL_SIZE / 2;
      int y1 = TTT_START_Y + pad;
      int y2 = TTT_START_Y + BOARD_SIZE * TTT_CELL_SIZE - pad;
      tft.fillRect(cx - 2, y1, 5, y2 - y1, lineColor);
      return;
    }
  }
  for (int i = 0; i < BOARD_SIZE; i++) {
    if (board[0][i] == player && board[1][i] == player && board[2][i] == player) {
      int cy = TTT_START_Y + i * TTT_CELL_SIZE + TTT_CELL_SIZE / 2;
      int x1 = TTT_START_X + pad;
      int x2 = TTT_START_X + BOARD_SIZE * TTT_CELL_SIZE - pad;
      tft.fillRect(x1, cy - 2, x2 - x1, 5, lineColor);
      return;
    }
  }
  if (board[0][0] == player && board[1][1] == player && board[2][2] == player) {
    int x1 = TTT_START_X + pad;
    int y1 = TTT_START_Y + pad;
    int x2 = TTT_START_X + BOARD_SIZE * TTT_CELL_SIZE - pad;
    int y2 = TTT_START_Y + BOARD_SIZE * TTT_CELL_SIZE - pad;
    for (int offset = -2; offset <= 2; offset++) {
      tft.drawLine(x1 + offset, y1, x2 + offset, y2, lineColor);
      tft.drawLine(x1, y1 + offset, x2, y2 + offset, lineColor);
    }
    return;
  }
  if (board[0][2] == player && board[1][1] == player && board[2][0] == player) {
    int x1 = TTT_START_X + pad;
    int y1 = TTT_START_Y + BOARD_SIZE * TTT_CELL_SIZE - pad;
    int x2 = TTT_START_X + BOARD_SIZE * TTT_CELL_SIZE - pad;
    int y2 = TTT_START_Y + pad;
    for (int offset = -2; offset <= 2; offset++) {
      tft.drawLine(x1 + offset, y1, x2 + offset, y2, lineColor);
      tft.drawLine(x1, y1 + offset, x2, y2 + offset, lineColor);
    }
    return;
  }
}

void handleGameOver(int winner) {
  if (!gameOverScreenDrawn) {
    drawCursor(cursorX, cursorY, cursorX, cursorY);

    if (winner == 1) {
      tttXWins++;
      drawTicTacToeWinningLine(PLAYER_X);
      drawTicTacToeScorePanels();
      drawTicTacToeTurnBadge("X WINS!", RED);
      drawTicTacToeFooter("PLAYER X WINS!  SELECT: Replay  HOME: Menu", YELLOW);
    } else if (winner == 2) {
      tttOWins++;
      drawTicTacToeWinningLine(PLAYER_O);
      drawTicTacToeScorePanels();
      drawTicTacToeTurnBadge("O WINS!", TTT_O_COLOR);
      drawTicTacToeFooter("PLAYER O WINS!  SELECT: Replay  HOME: Menu", YELLOW);
    } else if (winner == 3) {
      tttDraws++;
      drawTicTacToeScorePanels();
      drawTicTacToeTurnBadge("DRAW GAME", YELLOW);
      drawTicTacToeFooter("IT'S A DRAW!  SELECT: Replay  HOME: Menu", YELLOW);
    } else {
      drawTicTacToeFooter("ERROR! Press SELECT to Reset", RED);
    }
    gameOverScreenDrawn = true;
  }
  if (digitalRead(PIN_SELECT) == LOW) {
    resetTicTacToe();
    delay(300);
  }
}

} // namespace

sdk::Sprite TicTacToeGame::icon() const {
  return TICTACTOE_ICON;
}

void TicTacToeGame::onEnter(sdk::Console& console) {
  gTft = &console.display.rawTft();
  resetTicTacToe();
}

void TicTacToeGame::onUpdate(sdk::Console& console, float dt) {
  (void)console;
  (void)dt;

  static unsigned long lastMoveTime = 0;
  const unsigned long moveDelay = 150;

  if (gameStatus != 0) {
    handleGameOver(gameStatus);
    return;
  }

  int prevX = cursorX;
  int prevY = cursorY;
  unsigned long currentTime = millis();

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
    } else if (digitalRead(PIN_LEFT) == LOW) {
      cursorX = max(0, cursorX - 1);
      moved = true;
    }

    if (moved) {
      lastMoveTime = currentTime;
      drawCursor(prevX, prevY, cursorX, cursorY);
      return;
    }
  }

  if (digitalRead(PIN_BUTTONA) == LOW) {
    if (board[cursorX][cursorY] == EMPTY) {
      board[cursorX][cursorY] = currentPlayer;
      drawMarker(cursorX, cursorY, currentPlayer);

      if (checkWin(currentPlayer)) {
        gameStatus = currentPlayer;
      } else if (checkDraw()) {
        gameStatus = 3;
      } else {
        currentPlayer = (currentPlayer == PLAYER_X) ? PLAYER_O : PLAYER_X;
        if (currentPlayer == PLAYER_X) {
          drawTicTacToeTurnBadge("X'S TURN", RED);
        } else {
          drawTicTacToeTurnBadge("O'S TURN", TTT_O_COLOR);
        }
      }
      delay(300);
    }
  }
}
