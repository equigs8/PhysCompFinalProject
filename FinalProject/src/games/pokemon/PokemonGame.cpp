#include "PokemonGame.h"
#include "../../sdk/BoardConfig.h"
#include "assets/icon.h"
#include <array>
#include <map>
#include <string>
#include <math.h>

// Monsters
#include "pokemon.h"
#include "assets/leafle.h"
#include "assets/finpup.h"
#include "assets/katanid.h"
#include "assets/kitflare.h"
#include "assets/mantiscyth.h"
#include "assets/hammerfat.h"
#include "assets/anchorjaw.h"
#include "assets/omenmask.h"
#include "assets/spiritail.h"

namespace {

constexpr uint16_t BLACK        = 0x0000;
constexpr uint16_t WHITE        = 0xFFFF;
constexpr uint16_t RED          = 0xF800;
constexpr uint16_t YELLOW       = 0xFFE0;
constexpr uint16_t SKY_BLUE     = 0x7D1F;
constexpr uint16_t GRASS_GREEN  = 0x5E09;
constexpr uint16_t PLATFORM_COL = 0xDEFB;
constexpr uint16_t HUD_BG       = 0xEF7D;
constexpr uint16_t HP_GREEN     = 0x2624;

sdk::Console* gConsole = nullptr;
Adafruit_ILI9341* gTft = nullptr;

Pokemon playerPokemon;
Pokemon enemyPokemon;
int playerPokemonIndex = 0;
int enemyPokemonIndex = 0;

int pokemonMenuRow1StartingX = 10;
int pokemonMenuRow1StartingY = 200;
int pokemonMenuRow2StartingX = 10;
int pokemonMenuRow2StartingY = 220;

enum BattlePhase {
  PHASE_PLAYER_CHOICE,
  PHASE_EXECUTING_TURN,
  PHASE_ENEMY_CHOICE,
  PHASE_ENEMY_EXECUTING_TURN,
  PHASE_GAME_OVER
};
BattlePhase battlePhase = PHASE_PLAYER_CHOICE;

int pokemonMenuSelection = 0;
int pokemonSubMenuSelection = 0;
int previousPokemonSubMenuSelection = -1;

const std::map<int, std::array<std::string, 4>> pokemonMenuItems = {
  {0, {"Move", "Switch", "Bag", "Run"}},
  {1, {"", "", "", ""}},
  {2, {"Pikachu", "Charmander", "Squirtle", "Bulbasaur"}},
  {3, {"Poke Ball", "Great Ball", "Ultra Ball", "Master Ball"}},
  {4, {"Yes", "No", "", ""}}
};

void executeTurn(Move playerMove);
void resetPokemonBattler();

Pokemon createRandomPokemon(int level, int randomNum) {
  switch (randomNum) {
    case 0: return createLeafle(level);
    case 1: return createMantiscythe(level);
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

void getCursorRect(int index, int &x, int &y, int &w, int &h) {
  w = 90;
  h = 18;
  x = (index == 0 || index == 2) ? pokemonMenuRow1StartingX : (pokemonMenuRow1StartingX + 100);
  y = (index == 0 || index == 1) ? pokemonMenuRow1StartingY : pokemonMenuRow2StartingY;
}

void drawPokemonMenuCursor(int prevSelection, int newSelection) {
  auto& tft = *gTft;
  int x, y, w, h;
  if (prevSelection >= 0 && prevSelection <= 3) {
    getCursorRect(prevSelection, x, y, w, h);
    tft.drawRect(x - 2, y - 2, w, h, WHITE);
  }
  if (newSelection >= 0 && newSelection <= 3) {
    getCursorRect(newSelection, x, y, w, h);
    tft.drawRect(x - 2, y - 2, w, h, BLACK);
  }
}

void drawPokemonBattlerUI() {
  auto& tft = *gTft;
  tft.fillRect(0, 190, tft.width(), tft.height() - 190, WHITE);

  std::string opt0, opt1, opt2, opt3;
  if (pokemonMenuSelection == 1) {
    opt0 = playerPokemon.getMove(0).getName();
    opt1 = playerPokemon.getMove(1).getName();
    opt2 = playerPokemon.getMove(2).getName();
    opt3 = playerPokemon.getMove(3).getName();
  } else {
    opt0 = pokemonMenuItems.at(pokemonMenuSelection).at(0);
    opt1 = pokemonMenuItems.at(pokemonMenuSelection).at(1);
    opt2 = pokemonMenuItems.at(pokemonMenuSelection).at(2);
    opt3 = pokemonMenuItems.at(pokemonMenuSelection).at(3);
  }

  tft.setFont();
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

void drawFilledEllipse(int cx, int cy, int rx, int ry, uint16_t color) {
  auto& tft = *gTft;
  for (int y = -ry; y <= ry; y++) {
    int width = (int)(2.0 * rx * sqrt(1.0 - ((float)(y * y) / (float)(ry * ry))));
    tft.drawFastHLine(cx - width / 2, cy + y, width, color);
  }
}

void drawHUD(int x, int y, const char* name, int level, int currentHP, int maxHP, bool isPlayer) {
  auto& tft = *gTft;
  int w = 140;
  int h = 45;

  tft.fillRect(x, y, w, h, HUD_BG);
  tft.drawRect(x, y, w, h, BLACK);

  tft.setFont();
  tft.setTextColor(BLACK);
  tft.setTextSize(1);

  tft.setCursor(x + 10, y + 5);
  if (strlen(name) > 7) {
    tft.setCursor(x + 10, y + 10);
    tft.setTextSize(1);
  } else {
    tft.setTextSize(2);
  }
  tft.print(name);

  tft.setTextSize(1);
  tft.setCursor(x + 100, y + 10);
  tft.print("Lv");
  tft.print(level);

  int barX = x + 30;
  int barY = y + 25;
  int barW = 100;
  int barH = 6;

  tft.fillRect(barX, barY, barW, barH, 0x52AA);

  float hpPercent = (maxHP > 0) ? ((float)currentHP / maxHP) : 0.0f;
  int fillW = (int)(barW * hpPercent);

  uint16_t hpColor = HP_GREEN;
  if (hpPercent < 0.2f) hpColor = RED;
  else if (hpPercent < 0.5f) hpColor = YELLOW;

  tft.fillRect(barX, barY, fillW, barH, hpColor);

  if (isPlayer) {
    tft.setCursor(x + 50, y + 35);
    tft.print(currentHP);
    tft.print("/");
    tft.print(maxHP);
  }
}

void drawHPUI() {
  drawHUD(10, 15, enemyPokemon.getName(), enemyPokemon.getLevel(),
          enemyPokemon.getHealth(), enemyPokemon.getMaxHealth(), false);
  drawHUD(170, 135, playerPokemon.getName(), playerPokemon.getLevel(),
          playerPokemon.getHealth(), playerPokemon.getMaxHealth(), true);
}

void drawBattleScene() {
  auto& tft = *gTft;
  tft.fillRect(0, 0, 320, 80, SKY_BLUE);
  tft.fillRect(0, 80, 320, 110, GRASS_GREEN);

  int enemyBaseX = 230;
  int enemyBaseY = 90;
  drawFilledEllipse(enemyBaseX, enemyBaseY, 70, 25, PLATFORM_COL);

  int playerBaseX = 90;
  int playerBaseY = 180;
  drawFilledEllipse(playerBaseX, playerBaseY, 80, 30, PLATFORM_COL);

  drawHPUI();

  sdk::Sprite enemySpr = {
    enemyPokemon.getSprite(),
    (uint16_t)enemyPokemon.getSpriteWidth(),
    (uint16_t)enemyPokemon.getSpriteHeight(),
    0x0000,
    true
  };
  sdk::Sprite playerSpr = {
    playerPokemon.getSprite(),
    (uint16_t)playerPokemon.getSpriteWidth(),
    (uint16_t)playerPokemon.getSpriteHeight(),
    0x0000,
    true
  };
  gConsole->display.drawSprite(enemySpr, enemyBaseX - 30, enemyBaseY - 30, 1);
  gConsole->display.drawSprite(playerSpr, playerBaseX - 30, playerBaseY - 60, 1);
}

void drawBattleMessage(const char* message) {
  auto& tft = *gTft;
  tft.fillRect(0, 190, tft.width(), tft.height() - 190, WHITE);
  tft.drawRect(10, 200, 300, 35, BLACK);
  tft.setCursor(20, 210);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print(message);
}

bool performAttack(Pokemon &attacker, Pokemon &defender, Move move) {
  String msg = String(attacker.getName()) + " used " + move.getName() + "!";
  drawBattleMessage(msg.c_str());
  delay(1500);

  int damage = move.getDamage();
  if (damage < 0) {
    attacker.heal(abs(damage));
  } else {
    defender.takeDamage(damage);
  }

  if (defender.getHealth() < 0) {
    defender.heal(abs(defender.getHealth()));
  }

  drawHPUI();
  delay(1000);

  if (defender.getHealth() <= 0) {
    char* defenderName = defender.getNonConstName();
    msg = String(defenderName) + " fainted!";
    drawBattleMessage(msg.c_str());
    delay(2000);
    return true;
  }
  return false;
}

void endBattle(bool playerWon) {
  auto& tft = *gTft;
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
    tft.print(pokemonName);
    tft.print(" fainted.");
  }

  tft.setTextSize(1);
  tft.setCursor(30, 180);
  tft.print("Press Home or B to Exit");
  tft.setCursor(30, 200);
  tft.print("Press Select to Play Again");
}

void executeTurn(Move playerMove) {
  battlePhase = PHASE_EXECUTING_TURN;

  bool enemyFainted = performAttack(playerPokemon, enemyPokemon, playerMove);
  if (enemyFainted) {
    endBattle(true);
    return;
  }

  int randIndex = random(0, 4);
  Move enemyMove = enemyPokemon.getMove(randIndex);

  bool playerFainted = performAttack(enemyPokemon, playerPokemon, enemyMove);
  if (playerFainted) {
    endBattle(false);
    return;
  }

  battlePhase = PHASE_PLAYER_CHOICE;
  drawPokemonBattlerUI();
}

int pickDifferentPokemonIndex(int excludeIndex) {
  int idx = random(0, 9);
  while (idx == excludeIndex) {
    idx = random(0, 9);
  }
  return idx;
}

void resetPokemonBattler() {
  pokemonMenuSelection = 0;
  pokemonSubMenuSelection = 0;
  battlePhase = PHASE_PLAYER_CHOICE;

  int randomNum = random(0, 9);
  playerPokemon = createRandomPokemon(1, randomNum);
  playerPokemonIndex = randomNum;

  enemyPokemonIndex = pickDifferentPokemonIndex(randomNum);
  enemyPokemon = createRandomPokemon(1, enemyPokemonIndex);

  gTft->fillScreen(BLACK);
  drawBattleScene();
  drawPokemonBattlerUI();
}

} // namespace

sdk::Sprite PokemonGame::icon() const {
  return POKEMON_ICON;
}

void PokemonGame::onEnter(sdk::Console& console) {
  gConsole = &console;
  gTft = &console.display.rawTft();
  resetPokemonBattler();
}

void PokemonGame::onUpdate(sdk::Console& console, float dt) {
  (void)dt;
  static unsigned long lastNavTime = 0;
  static unsigned long lastButtonTime = 0;
  const unsigned long navDelay = 200;
  const unsigned long buttonDelay = 300;
  unsigned long currentTime = millis();

  if (battlePhase == PHASE_GAME_OVER) {
    if (digitalRead(PIN_BUTTONB) == LOW) {
      console.requestExit();
      delay(300);
    } else if (digitalRead(PIN_SELECT) == LOW) {
      resetPokemonBattler();
      delay(300);
    }
    return;
  }

  if (battlePhase == PHASE_EXECUTING_TURN) {
    return;
  }

  if (currentTime - lastNavTime >= navDelay) {
    bool moved = false;
    previousPokemonSubMenuSelection = pokemonSubMenuSelection;

    if (digitalRead(PIN_UP) == LOW) {
      if (pokemonSubMenuSelection >= 2) { pokemonSubMenuSelection -= 2; moved = true; }
    } else if (digitalRead(PIN_DOWN) == LOW) {
      if (pokemonSubMenuSelection <= 1) { pokemonSubMenuSelection += 2; moved = true; }
    } else if (digitalRead(PIN_LEFT) == LOW) {
      if (pokemonSubMenuSelection % 2 != 0) { pokemonSubMenuSelection -= 1; moved = true; }
    } else if (digitalRead(PIN_RIGHT) == LOW) {
      if (pokemonSubMenuSelection % 2 == 0) { pokemonSubMenuSelection += 1; moved = true; }
    }

    if (moved) {
      lastNavTime = currentTime;
      drawPokemonMenuCursor(previousPokemonSubMenuSelection, pokemonSubMenuSelection);
    }
  }

  if (currentTime - lastButtonTime >= buttonDelay) {
    if (digitalRead(PIN_BUTTONA) == LOW) {
      lastButtonTime = currentTime;
      if (pokemonMenuSelection == 0) {
        pokemonMenuSelection = pokemonSubMenuSelection + 1;
        pokemonSubMenuSelection = 0;
        drawPokemonBattlerUI();
      } else if (pokemonMenuSelection == 1) {
        Move selectedMove = playerPokemon.getMove(pokemonSubMenuSelection);
        executeTurn(selectedMove);
      }
    }

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
