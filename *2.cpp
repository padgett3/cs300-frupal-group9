#include "CsvToOccupant.h"

TileOccupant *newOccupant(const string &type, const string &csv) {
  /* The right operand would convert to string implicitly
   * just fine but still doing explicit conversion.
   */
  if (type == string("Food"))
    return csvToFood(csv);
  else if (type == string("Binoculars"))
    return csvToBinoculars(csv);
  else if (type == string("Ship"))
    return csvToShip(csv);
  else if (type == string("Treasure"))
    return csvToTreasure(csv);
  else if (type == string("Clue"))
    return csvToClue(csv);
  else if (type == string("Diamond"))
    return csvToDiamond(csv);
  else if (type == string("Obstacle"))
    return csvToObstacle(csv);
  else if (type == string("Tool"))
    return csvToTool(csv);
  else
    return 0;
}

TileOccupant *csvToFood(const string &csv) {
  stringstream input(csv);
  string name, temp;
  int price, energy;

  getline(input, name, ',');
  getline(input, temp, ',');
  price = stoi(temp);
  getline(input, temp, ',');
  energy = stoi(temp);

  return new Food(name, price, energy);
}

TileOccupant *csvToBinoculars(const string &csv) {
  stringstream input(csv);
  int price;

  input >> price;

  return new Binoculars(price);
}

TileOccupant *csvToShip(const string &csv) {
  stringstream input(csv);
  int price;

  input >> price;

  return new Ship(price);
}

TileOccupant *csvToTreasure(const string &csv) {
  stringstream input(csv);
  int worth;

  input >> worth;

  return new Treasure(worth);
}

TileOccupant *csvToClue(const string &csv) { return new Clue(csv); }

TileOccupant *csvToDiamond(const string &csv) { return new Diamond; }

TileOccupant *csvToObstacle(const string &csv) {
  stringstream input(csv);
  string name, temp;
  int energyCost;

  getline(input, name, ',');
  getline(input, temp, ',');
  energyCost = stoi(temp);

  return new Obstacle(name, energyCost);
}

TileOccupant *csvToTool(const string &csv) {
  stringstream input(csv);
  string name, temp;
  int price, rating, vecLen;

  getline(input, name, ',');
  getline(input, temp, ',');
  price = stoi(temp);
  getline(input, temp, ',');
  rating = stoi(temp);
  getline(input, temp, ',');
  vecLen = stoi(temp);

  vector<string> usableOn(vecLen, "");

  for (int i = 0; i < vecLen; ++i) {
    getline(input, temp, ',');
    usableOn.at(i) = temp;
  }

  return new Tool(name, price, rating, usableOn);
}

vector<string> inputFile(string src) {
  ifstream fin;

  fin.open(src);

  if (!fin)
    throw std::runtime_error("Failed to load " + src);

  vector<string> contents;
  string temp;

  while (getline(fin, temp)) {
    contents.push_back(temp);
  }

  return contents;
}

vector<string> cleanFile(vector<string> src) {
  vector<string> lines;

  // Erase comments
  for (auto &line : src) {
    size_t n = line.find("//");
    if (n != string::npos)
      line.erase(n);
  }

  // Read in the entire file, excluding empty lines
  for (auto &line : src) {
    if (line.size() != 0 && line.find_first_not_of(" \n\t") !=
                                string::npos) { // line[0] != ' '/*line !=
                                                // string(' ', line.size())*/) {
      lines.push_back(line);
    }
  }

  return lines;
}
#include "GameState.h"
#include "TileType.h"

GameState::GameState(string mapFile) : map(mapFile, heroX, heroY) {
  // string MapsrcFile = "Frupal.txt";

  int x;
  x = getmaxx(stdscr);
  UI.initialize(3 * x / 10);
  message = {"E, I",  "S, J", "D, K",  "F, L", "H",
             "NORTH", "WEST", "SOUTH", "EAST", "INVENTORY"};
  while (map.MaxX <= heroX) {
    map.MinX = map.MaxX - (map.MenuBorder / 2);
    map.MaxX = map.MaxX + (map.MenuBorder / 2);
    if (map.MaxX > (map.MAPSIZE - 1)) {
      map.MaxX = map.MAPSIZE;
      map.MinX = map.MAPSIZE - map.MenuBorder;
    }
  }
  while (map.MaxY <= heroY) {
    map.MinY = map.MaxY - (map.MaxScreenY / 2);
    map.MaxY = map.MaxY + (map.MaxScreenY / 2);
    if (map.MaxY > (map.MAPSIZE - 1)) {
      map.MaxY = map.MAPSIZE;
      map.MinY = map.MAPSIZE - map.MaxScreenY;
    }
  }
  heroX = abs(heroX - map.MinX);
  heroY = abs(heroY - map.MinY);

  ExpandMap();

  if (heroX == map.MenuBorder - 1) {
    heroX -= 1;
    ExpandMap();
    heroX += 1;
  } else if (heroX == 0) {
    heroX += 2;
    ExpandMap();
    heroX -= 2;
  }
  if (heroY == map.MaxScreenY - 1) {
    heroY -= 1;
    ExpandMap();
    heroY += 1;
  } else if (heroY == 0) {
    heroY += 2;
    ExpandMap();
    heroY -= 2;
  }
  cursorX = heroX;
  cursorY = heroY;
  flagCursor = 0;
}

GameState::~GameState() { map.saveFile("SavedFile.txt", heroX, heroY); }

// Main travel function
void GameState::travel(int &direction, WINDOW *win) {
  // Only Time this doesn't enter is when we
  // move the cursor or exit the program due to not having
  // enough energy (Should Also not enter when Diamond is found)
  if (HeroTravel(direction)) {
    if (flagCursor == 0)
      flagCursor = 1;
    // Enter if want to continue to explore the map
    if (ExpandMap()) {
      cursorX = heroX;
      cursorY = heroY;
      CursorInspect();
      // Clear screen
      for (int i = 0; i < map.MaxScreenY; ++i) {
        for (int j = 0; j < map.MenuBorder; ++j) {
          attron(COLOR_PAIR(3));
          mvwprintw(win, i, j, " ");
        }
      }
    }
    // What the hero can see
    HeroVision();

    map.displayMap(win);

    UI.actions(message);

    UI.whifflesEnergy(theHero.whiffles(), theHero.energy());

    wattron(win, COLOR_PAIR(6));
    mvwprintw(win, heroY, heroX, "@");
  }
  wmove(win, cursorY, cursorX);
  wrefresh(win);
}

// The hero traveling
bool GameState::HeroTravel(int &direction) {
  TileType *temp_type = NULL;

  switch (direction) {
  // These four cases is when user wants
  // to move the Hero.
  case 'i':
    if (heroY - 1 > -1) {
      if (heroY - 1 == -1)
        --heroY;
      else {
        // Move hero up
        --heroY;
        temp_type = map.tileTypeAt(heroX + map.MinX, heroY + map.MinY);
        // Check if we can enter
        if (temp_type->canEnter(theHero)) {
          // Make sure take away the energy needed to get here
          theHero.addEnergy((temp_type->energyCost() * -1));
          // Check our occupant
          return occupantCheck(direction);
        } else {
          ++heroY;
          return false;
        }
      }
    } else
      heroY = 0;
    break;

  case 'k':
    if (heroY + 1 < map.MaxScreenY) {
      if (heroY + 1 == map.MaxScreenY)
        ++heroY;
      else {
        // Same as above but move down
        ++heroY;
        temp_type = map.tileTypeAt(heroX + map.MinX, heroY + map.MinY);
        if (temp_type->canEnter(theHero)) {
          theHero.addEnergy((temp_type->energyCost() * -1));
          return occupantCheck(direction);

        } else {
          --heroY;
          return false;
        }
      }

    } else
      heroY = map.MaxScreenY - 1;
    break;

  case 'l':
    if (heroX + 1 < map.MenuBorder) {
      if (heroX + 1 == map.MenuBorder)
        ++heroX;
      else {
        // Move right
        ++heroX;
        temp_type = map.tileTypeAt(heroX + map.MinX, heroY + map.MinY);
        if (temp_type->canEnter(theHero)) {
          theHero.addEnergy((temp_type->energyCost() * -1));
          return occupantCheck(direction);

        } else {
          --heroX;
          return false;
        }
      }

    } else
      heroX = map.MenuBorder - 1;

    break;
  case 'j':
    if (heroX - 1 > -1) {
      if (heroX - 1 == -1)
        --heroX;
      else {
        // Move left
        --heroX;
        temp_type = map.tileTypeAt(heroX + map.MinX, heroY + map.MinY);
        if (temp_type->canEnter(theHero)) {
          theHero.addEnergy((temp_type->energyCost() * -1));
          return occupantCheck(direction);

        } else {
          ++heroX;
          return false;
        }
      }
    } else
      heroX = 0;
    break;
  default:
    cursorTravel(direction);
    return false;
  }
  return true;
}

// What occupies the tile
bool GameState::occupantCheck(int &direction) {

  /* "Row" does not correspond to the horizontal axis, so this is questionable
   * at best.
   */
  int r = heroX + map.MinX;
  int c = heroY + map.MinY;
  TileOccupant *occ = map.occupantAt(r, c);

  bool debarkShip = (theHero.hasShip() && map.isDebarkSafe(r, c));

  // End the game if the Hero is out of energy.
  if (theHero.energy() <= 0) {
    char response;
    do {
      response = UI.popup(string("You ran out of energy and can no longer") +
                              "move! Game over. Press 'q' to quit. ",
                          vector<string>{});
    } while (response != 'q');
    direction = 'q';
    return false;
  }

  // Not NULL, we have an occupant
  if (occ) {
    char response = 0;

    // Keep prompting user until they provide a valid response.
    do {
      // If encountering an Obstacle, the user needs to see their tool choices.
      if (occ->typeStr() == "Obstacle") {
        Obstacle *o = dynamic_cast<Obstacle *>(occ);
        response = UI.popup(occ->promptMsg(theHero), occ->getDetails(),
                            theHero.getToolOptions(*o));
      } else {
        response = UI.popup(occ->promptMsg(theHero), occ->getDetails());
      }

    } while (!occ->interact(response, theHero));

    /* End the game if the Hero found a diamond. The user has been
     * notified via pop-up already.
     */
    if (occ->typeStr() == "Diamond") {
      direction = 'q';
      return false;
    }

    /* Check whether the tileOccupant should still exist after the interaction.
     * If not, remove it from the map.
     */
    if (!occ->permanent()) {
      map.setOccupantAt(r, c, 0); // also deletes.
      occ = 0;
    }
  }

  /* The ship can only be placed if there isn't already a TileOccupant. Do not
   * use 'else'! The previous if statement modifies occ.
   */
  if (debarkShip && !occ) {
    map.setOccupantAt(r, c, new Ship(0, true));
    theHero.setHasShip(false);
  }

  return true;
}

void GameState::HeroVision() {
  // With Binoculars = 2 without = 1
  if (theHero.visionRange() == 1)
    HeroVision(heroY, heroX);
  else {
    HeroVision(heroY, heroX);
    int temp = heroX - 1;
    for (int i = 0; i < 4; ++i) {
      if (i > 1) {
        HeroVision(heroY + 1, temp);
        temp = heroX - 1;
      } else {
        HeroVision(heroY - 1, temp);
        temp = heroX + 1;
      }
    }
  }
}

// How much the hero can see on his journey
void GameState::HeroVision(int tempHeroY, int tempHeroX) {

  if ((tempHeroY < map.MaxScreenY && tempHeroY >= 0) &&
      (tempHeroX < map.MenuBorder && tempHeroX >= 0)) {
    // This is for the hero later on don't need to worry about it now
    int checkJ = tempHeroX - 1;
    int checkI = tempHeroY;
    int i = tempHeroY;
    int j = tempHeroX;
    for (int k = 0; k < 8; k++) {
      // If we are at 2 or 4 then
      // Go up or down 2D array.
      if (k == 2 || k == 4) {
        if (k == 4)
          // Up 2D array
          checkI = i - 1;
        else
          // Down 2D array
          checkI = i + 1;
        // Left
        checkJ = j - 1;
      } else if (k == 6) {
        // Check upper cell from original cell we are checking
        checkI = i - 1;
        // Stay same column
        checkJ = j;
      }
      // Don't go outside the boundries of array
      if ((checkI >= 0 && checkI < map.MaxScreenY) &&
          (checkJ >= 0 && checkJ < map.MenuBorder)) {
        // Tile is discovered, set it to true
        map.tile_revealed(checkI + map.MinY, checkJ + map.MinX);
        // array[checkI+map.MinY][checkJ+map.MinX].used = true;
      }
      if (k == 6)
        // check down
        checkI = i + 1;
      else
        // check right
        checkJ = j + 1;
    }
  }
  map.tile_revealed(heroY + map.MinY, heroX + map.MinX);
}

void GameState::revealMap() { map.revealAll(); }

// Inspect tiles with cursor
void GameState::cursorTravel(int direction) {
  if (flagCursor == 1) {
    cursorX = heroX;
    cursorY = heroY;
    flagCursor = 0;
  }
  switch (direction) {
  // These four cases is when user wants
  // to move cursor, and these cases move the
  // cursor accordingly.

  // WHat Function to call to get details of Tile
  case 'e':
    if (cursorY - 1 > 0)
      --cursorY;
    else
      cursorY = 0;
    break;

  case 'd':
    if (cursorY + 1 < map.MaxScreenY)
      ++cursorY;
    else
      cursorY = map.MaxScreenY - 1;

    break;

  case 'f':
    if (cursorX + 1 < map.MenuBorder)
      ++cursorX;
    else
      cursorX = map.MenuBorder - 1;
    break;

  case 's':
    if (cursorX - 1 > 0)
      --cursorX;
    else
      cursorX = 0;
    break;
  case 'h':
    if (theHero.GetInventory().size())
      UI.displayInventory(theHero.GetInventory());
    break;
  default:
    return;
  }
  // Pass in tileType and Occupant to inspect
  CursorInspect();
}

void GameState::CursorInspect() {
  TileType *temp_type = NULL;
  TileOccupant *occupant_temp = NULL;
  string temp;
  vector<string> details;

  if (map.isTileDiscovered(cursorX + map.MinX, cursorY + map.MinY)) {
    temp_type = map.tileTypeAt(cursorX + map.MinX, cursorY + map.MinY);
    occupant_temp = map.occupantAt(cursorX + map.MinX, cursorY + map.MinY);

    if (occupant_temp) {
      details = occupant_temp->getDetails();
      //	if(details.size() % 2 == 0)
      //	{
      //	I think that details should give an even vector.
      int i = details.size() / 2 + 1;
      temp = temp_type->toString();
      details.insert(details.begin(), temp);
      details.insert(details.begin() + i, "Grovnick");
      UI.tileInspect(details);

      //	}

    } else {
      temp = temp_type->toString();
      details.push_back(temp);
      details.push_back("Grovnick");
      UI.tileInspect(details);
    }
  } else {
    temp = "Undiscovered";
    details.push_back(temp);
    details.push_back("Grovnick");
    UI.tileInspect(details);
  }
}

bool GameState::ExpandMap() {
  int temp;

  // Explore down the map
  if (heroY == map.MaxScreenY - 2 && map.MaxY != map.MAPSIZE) {
    //--heroY;

    temp = heroY + map.MinY;

    map.MinY = map.MaxY - (map.MaxScreenY / 2);
    map.MaxY = map.MaxY + (map.MaxScreenY / 2);

    // Account for ODD #
    if (map.MaxY - map.MinY < map.MaxScreenY)
      ++map.MaxY;

    if (map.MaxY > (map.MAPSIZE - 1)) {
      map.MaxY = map.MAPSIZE;
      map.MinY = map.MAPSIZE - map.MaxScreenY;
    }

    heroY = abs((temp - map.MinY));
    heroX = heroX;

    return true;

  }
  // Go back up
  else if (heroY == 2 && map.MinY != 0) {
    //++heroY;
    temp = heroY + map.MinY;

    map.MinY -= (map.MaxScreenY / 2);
    map.MaxY -= (map.MaxScreenY / 2);

    if (map.MinY <= -1) {
      map.MaxY = map.MaxScreenY;
      map.MinY = 0;
    }

    heroY = abs((temp - map.MinY));
    heroX = heroX;

    return true;

  }

  // Explore right
  else if (heroX == map.MenuBorder - 2 && map.MaxX != map.MAPSIZE) {
    //--heroX;

    temp = heroX + map.MinX;

    map.MinX = map.MaxX - (map.MenuBorder / 2);
    map.MaxX = map.MaxX + (map.MenuBorder / 2);

    // Account for ODD #
    if (map.MaxX - map.MinX < map.MenuBorder)
      ++map.MaxX;

    if (map.MaxX > (map.MAPSIZE - 1)) {
      map.MaxX = map.MAPSIZE;
      map.MinX = map.MAPSIZE - map.MenuBorder;
    }

    heroX = abs(temp - map.MinX);
    heroY = heroY;

    return true;
  }

  // explore left
  else if (heroX == 2 && map.MinX != 0) {
    //++heroX;

    if (map.MinX != 0) {

      /*
          temp = heroY + map.MinY;

          map.MinY = map.MaxY - (map.MaxScreenY / 2);
          map.MaxY = map.MaxY + (map.MaxScreenY / 2);

          // Account for ODD #
          if (map.MaxY - map.MinY < map.MaxScreenY)
            ++map.MaxY;

          if (map.MaxY > (map.MAPSIZE - 1)) {
            map.MaxY = map.MAPSIZE;
            map.MinY = map.MAPSIZE - map.MaxScreenY;
          }

          heroY = abs((temp - map.MinY));
          heroX = heroX;
          return true;

        }
        // Go back up
        else if (heroY == -1) {
          ++heroY;
          temp = heroY + map.MinY;

          map.MinY -= (map.MaxScreenY / 2);
          map.MaxY -= (map.MaxScreenY / 2);

          if (map.MinY <= -1) {
            map.MaxY = map.MaxScreenY;
            map.MinY = 0;
          }

          heroY = abs((temp - map.MinY));
          heroX = heroX;
          return true;

        }

        // Explore right
        else if (heroX == map.MenuBorder) {
          --heroX;

          temp = heroX + map.MinX;

          map.MinX = map.MaxX - (map.MenuBorder / 2);
          map.MaxX = map.MaxX + (map.MenuBorder / 2);

          // Account for ODD #
          if (map.MaxX - map.MinX < map.MenuBorder)
            ++map.MaxX;

          if (map.MaxX > (map.MAPSIZE - 1)) {
            map.MaxX = map.MAPSIZE;
            map.MinX = map.MAPSIZE - map.MenuBorder;
          }

          heroX = abs(temp - map.MinX);
          heroY = heroY;
          return true;
        }

        // explore left
        else if (heroX == -1) {
          ++heroX;

          if (map.MinX != 0) {
      */
      temp = heroX + map.MinX;

      map.MinX -= (map.MenuBorder / 2);
      map.MaxX -= (map.MenuBorder / 2);

      if (map.MinX <= -1) {
        map.MaxX = map.MenuBorder;
        map.MinX = 0;
      }

      heroX = abs(temp - map.MinX);
      heroY = heroY;

      return true;
    }
  }
  return false;
}

void GameState::RunGame(WINDOW *win) {
  int choice = 'a';

  HeroVision();
  map.displayMap(win);

  UI.whifflesEnergy(theHero.whiffles(), theHero.energy());
  wattron(win, COLOR_PAIR(6));
  mvwprintw(win, heroY, heroX, "@");
  CursorInspect();
  UI.actions(message);
  wmove(win, cursorY, cursorX);
  wrefresh(win);

  while (choice != 'q') {
    choice = wgetch(stdscr);
    travel(choice, win);
  }
}
/*
   Hero class implementation
   November 2020
 */
#include "Hero.h"
using namespace std;

// constructor
Hero::Hero() {
  hasBinoculars_ = false;
  hasShip_ = false;
  whiffles_ = 1000;
  energy_ = 100;
}

Hero::Hero(const Hero &orig) {
  this->hasBinoculars_ = orig.hasBinoculars_;
  this->hasShip_ = orig.hasShip_;
  this->inventory_ = orig.inventory_;
  this->whiffles_ = orig.whiffles_;
  this->energy_ = orig.energy_;
}

vector<vector<string>> Hero::GetInventory() const {
  vector<vector<string>> options;

  for (unsigned int i = 0; i < inventory_.size(); ++i) {
    // Get the details of the
    vector<string> details = inventory_.at(i)->getDetails();

    // Put the name of the tool in the right column
    details.at(details.size() / 2) = details.at(0);

    // Set the left column to the choice for selecting it
    details.at(0) = string(1, choiceIndexToChar(i));

    // Add the details array to the array of details arrays
    options.push_back(details);
  }

  return options;
}

// returns list of tools usable on Obstacle
vector<Tool *> Hero::getUsableTools(Obstacle &current) {
  vector<Tool *> usableTools_;

  for (auto i = inventory_.cbegin(); i != inventory_.cend(); ++i) {
    if ((*i)->usableOn(current)) {
      usableTools_.push_back(*i);
    }
  }

  return usableTools_;
}

vector<vector<string>> Hero::getToolOptions(Obstacle &current) {
  vector<Tool *> tools = getUsableTools(current);
  vector<vector<string>> options;

  for (unsigned int i = 0; i < tools.size(); ++i) {
    // Get the details of the
    vector<string> details = tools.at(i)->getDetails();

    // Put the name of the tool in the right column
    details.at(details.size() / 2) = details.at(0);

    // Set the left column to the choice for selecting it
    details.at(0) = string(1, choiceIndexToChar(i));

    // Add the details array to the array of details arrays
    options.push_back(details);
  }

  return options;
}

void Hero::addInventory(Tool *toAdd) { inventory_.push_back(toAdd); }

bool Hero::consumeTool(Tool *xtool) {
  bool success = false;
  auto end = inventory_.end();
  for (auto i = inventory_.begin(); i != end; ++i) {
    if (*i == xtool) {
      inventory_.erase(i);
      success = true;
    }
  }
  return success;
}

int Hero::addWhiffles(int value) {
  whiffles_ += value;
  return whiffles_;
}

int Hero::addEnergy(int value) {
  energy_ += value;
  return energy_;
}

void Hero::setHasBinoculars(bool hasBinoculars) {
  hasBinoculars_ = hasBinoculars;
}

void Hero::setHasShip(bool hasShip) { hasShip_ = hasShip; }

// getter functions
int Hero::visionRange(void) const {
  if (hasBinoculars_)
    return 2;
  else
    return 1;
}

bool Hero::hasShip(void) const { return hasShip_; }

int Hero::whiffles(void) const { return whiffles_; }

int Hero::energy(void) const { return energy_; }
#include "GameState.h"
#include <iostream>

int main(int argc, char **argv) {
  initscr();
  noecho();
  keypad(stdscr, true);
  start_color();

  // getch();

  init_pair(1, COLOR_WHITE, COLOR_CYAN);
  init_pair(2, COLOR_BLACK, COLOR_GREEN);
  init_pair(3, COLOR_BLACK, COLOR_BLACK);
  init_pair(4, COLOR_WHITE, COLOR_BLUE);
  init_pair(5, COLOR_BLACK, COLOR_MAGENTA);
  init_pair(6, COLOR_YELLOW, COLOR_RED);
  init_pair(7, COLOR_BLACK, COLOR_WHITE);

  if (argc == 1) {
    GameState Game;
    Game.RunGame(stdscr);
  } else {
    GameState Game(argv[1]);

    // Remove this to remove cheat
    if (argc >= 3) {
      Game.revealMap();
    }

    Game.RunGame(stdscr);
  }

  endwin();

  return 0;
}
#include "CsvToOccupant.h"
#include "Map.h"
#include "TileOccupant.h"
#include "TileType.h"
#include <stdexcept>

Tile::Tile() : revealed(false), type(0), occupant(0) {}

Tile::~Tile() {
  if (type)
    delete type;
  if (occupant)
    delete occupant;
}

Map::Map(string srcFile, int &heroX, int &heroY) {
  // MaxScreenY = 0; //LINES

  // The max we can go on the screen
  // MenuBorder = 0; // MaxScreenX for frupal Map

  getmaxyx(stdscr, MaxScreenY, MenuBorder);

  MaxY = MaxScreenY;
  if (MenuBorder > 170)
    MenuBorder = MAPSIZE;
  else
    MenuBorder = MenuBorder - (3 * MenuBorder / 10);

  MaxX = MenuBorder;

  MinX = 0;
  MinY = 0;

  if (!(loadFile(srcFile, heroX, heroY))) {
    throw runtime_error("File cannot open");
  }

  if (!(loadOccupants("occupantFile.txt"))) {
    throw runtime_error("File cannot open");
  }
}

// Read in the map
bool Map::loadFile(string src, int &heroX, int &heroY) {
  string temp;
  int i = 0;
  int j = 0;

  // if(read_file==0)
  ifstream infile;
  // Open the designated file
  // infile.open("practice.txt");
  infile.open(src);

  // If file was open sucessfully then eneter
  if (infile) {
    infile >> heroX;
    infile.ignore(100, ',');
    infile >> heroY;
    infile.ignore(100, '\n');

    // If end of file is not triggered then enter the loop
    while (getline(infile, temp)) {
      for (unsigned int k = 0; k < temp.size(); ++k) {

        if (j == MAPSIZE) {
          ++i;
          j = 0;
        }

        switch (temp[k]) {
        case 'W':
          tileArray[i][j].type = new Water; // Color num 4
          break;
        case 'M':
          tileArray[i][j].type = new Wall; // Color num 7
          break;
        case 'S':
          tileArray[i][j].type = new Swamp; // Color num 5
          break;
        case 'G':
          tileArray[i][j].type = new Meadow; // Color num 2
          break;
        }

        ++j;

        // How should we go about occupants?
      }
    }
    infile.close();

  } else {
    return false;
  }

  return true; // placeholder for better things
}

bool Map::loadOccupants(string src) {
  string temp;

  ifstream fin;

  fin.open(src);

  if (fin) {
    // Convert the file to a vector of strings
    vector<string> contents = inputFile(src);

    // Remove empty lines and comments
    contents = cleanFile(contents);

    // The first (used) line must be the occupant count
    int qty = stoi(contents[0]);

    for (int i = 1; i <= qty; ++i) {
      // Convert line to stream
      stringstream thisLine(contents[i]);

      // Read coordinates of tileOccupant
      getline(thisLine, temp, ',');
      int col = stoi(temp);
      getline(thisLine, temp, ',');
      int row = stoi(temp);

      // Read tileOccupant type string (without trailing whitespace)
      getline(thisLine, temp, ',');
      string type = temp;

      // Read tileOccupant data as comma-separated values
      getline(thisLine, temp);

      // If the tile already has an occupant, remove it.

      // TODO: IDEALLY MAKE THIS WORK
      // if (tileArray[row][col].occupant != 0)
      //  delete tileArray[row][col].occupant;

      tileArray[row][col].occupant = newOccupant(type, temp);
    }
  } else {
    return false;
  }

  return true;
}

bool Map::saveFile(string dest, int heroX, int heroY) {
  // Variable: Outfile
  ofstream outfile;
  // Open the data.txt files
  outfile.open(dest);
  // Clear what was in function
  outfile.clear();
  // Close the file
  outfile.close();
  // Reopen another file
  outfile.open(dest, ios::app);

  outfile << "Last Position of Hero: " << heroX << "," << heroY << endl;
  // Loop through list.

  for (int i = 0; i < MAPSIZE; ++i) {
    for (int j = 0; j < MAPSIZE; ++j) {
      if (tileArray[i][j].type->toString() == "Meadow")
        outfile << "G";
      else if (tileArray[i][j].type->toString() == "Water")
        outfile << "W";
      else if (tileArray[i][j].type->toString() == "Wall")
        outfile << "M";
      else if (tileArray[i][j].type->toString() == "Swamp")
        outfile << "S";
    }
    outfile << endl;
  }
  // Close the file
  outfile.close();

  return true;
}

bool Map::saveOccupants(string dest) {
  ofstream fout("CustomOccupants.txt");

  // Collect occupants (in order to count) before saving to file.
  vector<TileOccupant *> occupants;
  vector<int> rows;
  vector<int> cols;

  for (int i = 0; i < MAPSIZE; ++i) {
    for (int j = 0; j < MAPSIZE; ++j) {
      TileOccupant *occ = tileArray[i][j].occupant;

      if (occ) {
        occupants.push_back(occ);
        rows.push_back(i);
        cols.push_back(j);
      }
    }
  }

  if (fout) {
    fout << occupants.size() << "\n";

    for (unsigned int i = 0; i < occupants.size(); ++i) {
      fout << "\n"
           << rows[i] << "," << cols[i] << "," << occupants[i]->typeStr() << ","
           << occupants[i]->dataAsCsv() << "\n";
    }
  } else {
    return false;
  }

  return true;
}

// Return what type of tile it is
TileType *Map::tileTypeAt(int col, int row) { return tileArray[row][col].type; }

// Return what  occupies the tile
TileOccupant *Map::occupantAt(int col, int row) {
  return tileArray[row][col].occupant;
}
// Reveal the tile(Discovered)
void Map::tile_revealed(int row, int col) {
  tileArray[row][col].revealed = true;
}

void Map::revealAll() {
  for (int i = 0; i < MAPSIZE; ++i) {
    for (int j = 0; j < MAPSIZE; ++j) {
      tileArray[i][j].revealed = true;
    }
  }
}

// Remove a tileOccupant, typicaly after it is bought/consumed/looted
void Map::setOccupantAt(int col, int row, TileOccupant *newOccupant) {
  TileOccupant *&temp = tileArray[row][col].occupant;

  if (temp)
    delete temp;

  temp = newOccupant;
}

// Can a ship be left on the tile at the provided coordinates
bool Map::isDebarkSafe(int col, int row) {

  // Can never leave ship in water
  if (tileTypeAt(col, row)->toString() == "Water")
    return false;

  const Hero withoutShip{};

  // Check all adjacent tiles
  for (int i = 0; i < 9; ++i) {
    int r = row + i % 3 - 1;
    int c = col + i / 3 - 1;

    if ((r >= 0) && (c >= 0) && (r < MAPSIZE) && (c < MAPSIZE) &&
        (r != row || c != col)) {
      if (tileTypeAt(c, r)->canEnter(withoutShip)) {
        // Allow ship debarking if even one adjacent tile is walkable.
        return true;
      }
    }
  }

  return false;
}

// Have we been at tile before
bool Map::isTileDiscovered(int col, int row) {
  if (tileArray[row][col].revealed) {
    return true;
  }

  return false;
}

// Display what is discovered
void Map::displayMap(WINDOW *win) {
  string MarkerDisplay;
  // Print the Grovnicks that are used
  for (int i = MinY; i < MaxY; ++i) {
    for (int j = MinX; j < MaxX; ++j) {
      if (tileArray[i][j].revealed) {
        if (tileArray[i][j].occupant)
          MarkerDisplay = string(1, tileArray[i][j].occupant->marker());
        else
          MarkerDisplay = " ";

        if (tileArray[i][j].occupant != NULL &&
            tileArray[i][j].occupant->typeStr() == "Diamond") {

          wattron(win, COLOR_PAIR(1));
          mvwprintw(win, i - MinY, j - MinX, MarkerDisplay.data());
        } else {
          wattron(win, COLOR_PAIR(tileArray[i][j].type->color()));

          mvwprintw(win, i - MinY, j - MinX, MarkerDisplay.data());
        }
      }
    }
  }
}
/*
FileName:	TileOccupant.cpp
Author:		Drew McKinney
Group:		9
Description:
                TileOccupant and derived classes implementation
*/

#include "TileOccupant.h"
#include <cmath>

using namespace std;

int charToChoiceIndex(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'Z')
    return c - 'A' + 36;

  return -1; /* if ( (c < '0' && c > '9') || (c < 'a' && c > 'z') ||
      (c < 'A' && c > 'Z') )
      */
}

char choiceIndexToChar(int ind) {
  char i = static_cast<char>(ind);

  if (i >= 0 && i < 10)
    return '0' + i;
  if (i >= 10 && i < 36)
    return 'a' + i;
  if (i >= 36 && i < 62)
    return 'A' + i;

  return -1; // if (i < 0 || i >= 62)
}

/*
TileOccupant abstract base class
*/
TileOccupant::TileOccupant() {}

TileOccupant::~TileOccupant() {}

/* Interact is a virtual function. For all classes that override it, returning
 * "true" indicates a valid "promptResponse." If interact returns "false", then
 * the caller must prompt the user for a new reponse and call interact again.
 */
bool TileOccupant::interact(char promptResponse, Hero &theHero) {
  // This serves as a default case: only allow response of y/n
  if (promptResponse != 'y' &&
      (promptResponse != 'Y' &&
       (promptResponse != 'n' && promptResponse != 'N'))) {
    return false;
  }

  return true;
}

/*
/////////////////////////////////////////////////////////////////
Treasure class derived from TileOccupant
/////////////////////////////////////////////////////////////////
*/

Treasure::Treasure() : worth_(0) {}

Treasure::Treasure(int worth) : worth_(worth) {}

bool Treasure::permanent() { return false; }

/*
Function:	color
Description:	return appropriate color for derived class,
                black? Not sure
Arguments:	none
Return:		int - color for this class, treasure
*/
int Treasure::color() { return COLOR_BLACK; }

/*
Function:	marker
Description:	return appropriate marker for derived class
Arguments:	none
Return:		char - marker for this class, treasure
*/
char Treasure::marker() { return '$'; }

/*
Function:	getDetails
Description:	returns data in the form of strings to pass to ui
Arguments:
Return:		vector<string> - object data to pass to ui
*/
vector<string> Treasure::getDetails() {
  vector<string> data;
  data.push_back("");
  data.push_back(to_string(worth_));
  data.push_back("Treasure");
  data.push_back("Worth");

  return data;
}

/*
Function:	promptMsg
Description:	returns message specific to this class, treasure
Arguments:	none
Return:		string - message to display to player
*/
string Treasure::promptMsg(Hero &theHero) {
  string msg = string("A treasure chest has been found ") +
               "containing: " + to_string(worth_) + " Whiffles!";
  return msg;
}

/*
Function:	interact
Description:	occupant interacts with player according to
                promptResponse
Arguments:	char - player response to prompt
                Hero& - player
Return:		none
*/
bool Treasure::interact(char promptResponse, Hero &theHero) {
  theHero.addWhiffles(worth_);

  // Treasure does not use promptResponse, so the value is always valid.
  return true;
}

string Treasure::typeStr() const { return "Treasure"; }

string Treasure::dataAsCsv() const { return to_string(worth_); }

/*
/////////////////////////////////////////////////////////////////
 Ship class derived from TileOccupant
/////////////////////////////////////////////////////////////////
*/

Ship::Ship() : whiffleCost_(0), bought_(false), toRemove_(false) {}

Ship::Ship(int whiffleCost, bool bought)
    : whiffleCost_(whiffleCost), bought_(bought), toRemove_(false) {}

/*
Function:	permanent
Description:	returns if occupant is permanent or not?
Arguments:	none
Return:		none
*/
bool Ship::permanent() { return !toRemove_; }

/*
Function:	color
Description:	return appropriate color for derived class
Arguments:	none
Return:		int - color for this class, ship
*/
int Ship::color() { return COLOR_BLACK; }

/*
Function:	marker
Description:	return appropriate marker for derived class
Arguments:	none
Return:		char - marker for this class, ship
*/
char Ship::marker() { return 'S'; }

/*
Function:	getDetails
Description:	returns data in the form of strings to pass to ui
Arguments:
Return:		vector<string> - object data to pass to ui
*/
vector<string> Ship::getDetails() {
  vector<string> data;
  data.push_back("");
  data.push_back(to_string(whiffleCost_));
  if (bought_)
    data.push_back("True");
  else
    data.push_back("False");
  data.push_back("Ship");
  data.push_back("Price");
  data.push_back("Bought");

  return data;
}

/*
Function:	promptMsg
Description:	returns message specific to this class, treasure
Arguments:	none
Return:		string - message to display to player
*/
string Ship::promptMsg(Hero &theHero) {
  if (theHero.hasShip()) {
    return string("You can't board this ship since you are already") +
           "riding one! ";
  }

  if (bought_) {
    return string("Welcome back! Sail in ship? (Y/N)");
  }

  string msg = "A ship has been found! ";

  if (theHero.whiffles() < whiffleCost_) {
    msg = msg + "But you don't have enough Whiffles!" + "Sorry!";
  } else {
    msg = msg + "Purchase? (Y/N)"; // ship for "
                                   // + to_string(whiffleCost_)
                                   // + " Whiffles?(Y/N):";
  }
  return msg;
}

/*
Function:	interact
Description:	occupant interacts with player according to
                promptResponse
Arguments:	char - player response to prompt
                Hero& - player
Return:		none
*/
bool Ship::interact(char promptResponse, Hero &theHero) {
  /* If the hero is already riding a ship, or cannot afford the ship and
   * the ship is new ...
   */
  if ((theHero.whiffles() < whiffleCost_ && !bought_) || theHero.hasShip()) {
    // ... do nothing. Any input is valid, so return true.
    return true;
  }

  // The user chose to purchase or use the ship
  if (promptResponse == 'y' || promptResponse == 'Y') {
    // Price is paid only once.
    if (!bought_) {
      theHero.addWhiffles(-whiffleCost_);
      bought_ = true;
    }
    toRemove_ = true;
    theHero.setHasShip(true);
    return true;
  }

  // Verify that the user chose to not purchase the ship.
  return TileOccupant::interact(promptResponse, theHero);
}

string Ship::typeStr() const { return "Ship"; }

string Ship::dataAsCsv() const { return to_string(whiffleCost_); }

/*
/////////////////////////////////////////////////////////////////
 Tool class derived from TileOccupant
/////////////////////////////////////////////////////////////////
*/

Tool::Tool()
    : name_(0), whiffleCost_(0), rating_(0), forObstacles(0), bought_(false) {}

Tool::Tool(const Tool &toCopy)
    : name_(toCopy.name_), whiffleCost_(toCopy.whiffleCost_),
      rating_(toCopy.rating_), forObstacles(toCopy.forObstacles),
      bought_(toCopy.bought_) {}

Tool::Tool(string name, int whiffleCost, int rating, vector<string> usableOn)
    : name_(name), whiffleCost_(whiffleCost), rating_(rating),
      forObstacles(usableOn), bought_(false) {}

/*
Function:	usableOn
Description:	determines if tool is usable on given obstacle
Arguments:	Obstacle& - obstacle to check against
Return:		bool - true if usable on obstacle,
                        false if not
*/
bool Tool::usableOn(const Obstacle &onObstacle) const {
  // vector<string>::iterator i;
  for (auto i = forObstacles.cbegin(); i < forObstacles.cend(); i++) {
    if (*i == onObstacle.name())
      return true;
  }
  return false;
}

/*
Function:	rating
Description:	returns rating of tool
Arguments:	none
Return:		int - rating of tool
*/
int Tool::rating() { return rating_; }

/*
Function:	permanent
Description:	determines if tool is permanent
Arguments:	none
Return:		bool - true if permanent,
                        false if not
*/
bool Tool::permanent() { return !bought_; }

/*
Function:	color
Description:	gets color of occupant depending on derived class
Arguments:	none
Return:		int - color value for ncurses
*/
int Tool::color() { return COLOR_BLACK; }

/*
Function:	marker
Description:	gets marker for occupant
Arguments:	none
Return:		char - marker for specific occupant class
*/
char Tool::marker() { return 'T'; }

/*
Function:	getDetails
Description:	returns data in the form of strings to pass to ui
Arguments:
Return:		vector<string> - object data to pass to ui
*/
vector<string> Tool::getDetails() {
  vector<string> data;
  // push name of tool to vector
  data.push_back(name_);

  // push all strings from forObstacles
  string obstacleList;
  for (vector<string>::iterator it = forObstacles.begin();
       it != forObstacles.end(); ++it) {
    obstacleList = obstacleList + *it;
  }
  data.push_back(obstacleList);
  // push values for cost and rating
  data.push_back(to_string(whiffleCost_));
  data.push_back(to_string(rating_));

  // push labels
  data.push_back("Tool");
  data.push_back("Obstacle");
  data.push_back("Price");
  data.push_back("Rating");

  return data;
}

/*
Function:	promptMsg
Description:	gets correct prompt for player for class
Arguments:	Hero& - player
Return:		string - message to prompt player
*/
string Tool::promptMsg(Hero &theHero) {
  string msg;

  msg = "Tool found! ";
  if (theHero.whiffles() >= whiffleCost_) {
    /* msg = msg + name_ + "\n";
    // get all obstacles tool works for
    for (auto it = forObstacles.begin()
            ; it != forObstacles.end(); ++it)
    {
            msg = msg + (*it) + " : Obstacle\n";
    }

    // get tool cost and rating
    msg = msg + to_string(whiffleCost_) + " : Cost\n";
    msg = msg + "X" + to_string(rating_)
            + " : Rating\n"; */

    msg = msg + "Would you like to purchase? (Y/N)";
  } else {
    msg = msg + "But you don't have enough Whiffles! " + "Sorry!";
  }
  return msg;
}

/*
Function:	interact
Description:	interacts with player according to player response
Arguments:	char - player response to prompt
                Hero& - player
Return:		none
*/
bool Tool::interact(char promptResponse, Hero &theHero) {
  if (theHero.whiffles() < whiffleCost_) {
    return true;
  }

  if (!TileOccupant::interact(promptResponse, theHero)) {
    return false;
  }

  switch (promptResponse) {
  case 'y':
  case 'Y':
    bought_ = true;
    theHero.addWhiffles(-whiffleCost_);
    theHero.addInventory(new Tool(*this));
    break;
  default:
    return true;
  }

  return true;
}

string Tool::typeStr() const { return "Tool"; }

string Tool::dataAsCsv() const {
  string ret = name_ + "," + to_string(whiffleCost_) + "," +
               to_string(rating_) + "," + to_string(forObstacles.size());

  for (unsigned int i = 0; i < forObstacles.size(); ++i) {
    ret += "," + forObstacles.at(i);
  }

  return ret;
}

/*
/////////////////////////////////////////////////////////////////
 Food class derived from TileOccupant
/////////////////////////////////////////////////////////////////
*/

Food::Food()
    : name_(0), whiffleCost_(0), energyProvided_(0), consumed_(false) {}

Food::Food(string name, int whiffleCost, int energyProvided)
    : name_(name), whiffleCost_(whiffleCost), energyProvided_(energyProvided),
      consumed_(false) {}

/*
Function:	permanent
Description:	determines if occupant is permanent or not
Arguments:	none
Return:		bool - true if perm, false if not
*/
bool Food::permanent() { return !consumed_; }

/*
Function:	color
Description:	gets occupant marker color
Arguments:	none
Return:		int - marker color
*/
int Food::color() { return COLOR_BLACK; }

/*
Function:	marker
Description:	gets occupant marker
Arguments:	none
Return:		char - marker
*/
char Food::marker() { return 'F'; }

/*
Function:	getDetails
Description:	gets data to send to ui
Arguments:	none
Return:		vector<string> - data strings for ui
*/
vector<string> Food::getDetails() {
  vector<string> data;
  data.push_back(name_);
  data.push_back(to_string(whiffleCost_));
  data.push_back(to_string(energyProvided_));

  // labels
  data.push_back("Food");
  data.push_back("Price");
  data.push_back("Energy");

  return data;
}

/*
Function:	promptMsg
Description:	gets message to display to user
Arguments:	Hero& - hero to prompt
Return:		string - message to display
*/
string Food::promptMsg(Hero &theHero) {
  string msg;

  msg = string("You found a delicious ") + name_ + "!";

  if (theHero.whiffles() >= whiffleCost_) {
    msg = msg + " Would you like to purchase (Y/N)?\n";
  } else {
    msg = msg + " But you don't have enough Whiffles! " + "Sorry!";
  }
  return msg;
}

/*
Function:	interact
Description:	interacts with user according to response key
Arguments:	char - prompt response key
                , Hero& - hero to interact with
Return:		string - message to display
*/
bool Food::interact(char promptResponse, Hero &theHero) {
  if (theHero.whiffles() < whiffleCost_) {
    return true;
  }

  if (!TileOccupant::interact(promptResponse, theHero)) {
    return false;
  }

  switch (promptResponse) {
  case 'y':
  case 'Y':
    consumed_ = true;
    theHero.addWhiffles(-whiffleCost_);
    theHero.addEnergy(energyProvided_);
    break;
  default:
    return true;
  }

  return true;
}

string Food::typeStr() const { return "Food"; }

string Food::dataAsCsv() const {
  return name_ + "," + to_string(whiffleCost_) + "," +
         to_string(energyProvided_);
}

/*
/////////////////////////////////////////////////////////////////
 Binoculars class derived from TileOccupant
/////////////////////////////////////////////////////////////////
*/

Binoculars::Binoculars() : whiffleCost_(0), bought_(false) {}

Binoculars::Binoculars(int whiffleCost)
    : whiffleCost_(whiffleCost), bought_(false) {}

/*
Function:	permanent
Description:	determines if occupant is permanent or not
Arguments:	none
Return:		bool - true if perm, false if not
*/
bool Binoculars::permanent() { return !bought_; }

/*
Function:	color
Description:	gets occupant marker color
Arguments:	none
Return:		int - marker color
*/
int Binoculars::color() { return COLOR_BLACK; }

/*
Function:	marker
Description:	gets occupant marker
Arguments:	none
Return:		char - marker
*/
char Binoculars::marker() { return 'B'; }

/*
Function:	getDetails
Description:	gets data to send to ui
Arguments:	none
Return:		vector<string> - data strings for ui
*/
std::vector<std::string> Binoculars::getDetails() {
  return vector<string>{"", to_string(whiffleCost_), "Binoculars", "Price"};
}

/*
Function:	promptMsg
Description:	gets message to display to user
Arguments:	Hero& - hero to prompt
Return:		string - message to display
*/
string Binoculars::promptMsg(Hero &theHero) {
  string msg;

  msg = "Binoculars found!\n";
  if (theHero.whiffles() >= whiffleCost_) {
    msg = msg + "Buy Binoculars and double your " + "vision for " +
          to_string(whiffleCost_) + " Whiffles? (Y/N):";
  } else {
    msg = msg + "But you don't have enough Whiffles! " + "Sorry!";
  }
  return msg;
}

/*
Function:	interact
Description:	interacts with user according to response key
Arguments:	char - prompt response key
                , Hero& - hero to interact with
Return:		string - message to display
*/
bool Binoculars::interact(char promptResponse, Hero &theHero) {
  if (theHero.whiffles() < whiffleCost_) {
    return true;
  }

  if (!TileOccupant::interact(promptResponse, theHero)) {
    return false;
  }

  switch (promptResponse) {
  case 'y':
  case 'Y':
    bought_ = true;
    theHero.addWhiffles(-whiffleCost_);
    theHero.setHasBinoculars(true);
    break;
  default:
    return true;
  }

  return true;
}

string Binoculars::typeStr() const { return "Binoculars"; }

string Binoculars::dataAsCsv() const { return to_string(whiffleCost_); }

/*
/////////////////////////////////////////////////////////////////
 clue class derived from TileOccupant
/////////////////////////////////////////////////////////////////
*/

Clue::Clue() : msg_(0) {}

Clue::Clue(string msg) : msg_(msg) {}

/*
Function:	permanent
Description:	determines if occupant is permanent or not
Arguments:	none
Return:		bool - true if perm, false if not
*/
bool Clue::permanent() { return true; }

/*
Function:	color
Description:	gets occupant marker color
Arguments:	none
Return:		int - marker color
*/
int Clue::color() { return COLOR_BLACK; }

/*
Function:	marker
Description:	gets occupant marker
Arguments:	none
Return:		char - marker
*/
char Clue::marker() { return '?'; }

/*
Function:	getDetails
Description:	gets data to send to ui
Arguments:	none
Return:		vector<string> - data strings for ui
*/
vector<string> Clue::getDetails() {
  vector<string> data;
  data.push_back("");
  // data.push_back(msg_);
  data.push_back("Clue");
  // data.push_back("Message");*/
  return data;
}

/*
Function:	promptMsg
Description:	gets message to display to user
Arguments:	Hero& - hero to prompt
Return:		string - message to display
*/
string Clue::promptMsg(Hero &theHero) {
  string msg;

  msg = "Clue found!\n";
  return msg + msg_;
}

/*
Function:	interact
Description:	interacts with user according to response key
Arguments:	char - prompt response key
                , Hero& - hero to interact with
Return:
*/
bool Clue::interact(char promptResponse, Hero &theHero) { return true; }

string Clue::typeStr() const { return "Clue"; }

string Clue::dataAsCsv() const { return msg_; }

/*
/////////////////////////////////////////////////////////////////
 Diamond class derived from TileOccupant
/////////////////////////////////////////////////////////////////
*/

Diamond::Diamond() {}

/*
Function:	permanent
Description:	determines if occupant is permanent or not
Arguments:	none
Return:		bool - true if perm, false if not
*/
bool Diamond::permanent() { return false; }

/*
Function:	color
Description:	gets occupant marker color
Arguments:	none
Return:		int - marker color
*/
int Diamond::color() { return COLOR_WHITE; }

/*
Function:	marker
Description:	gets occupant marker
Arguments:	none
Return:		char - marker
*/
char Diamond::marker() { return '$'; }

/*
Function:	getDetails
Description:	gets data to send to ui
Arguments:	none
Return:		vector<string> - data strings for ui
*/
vector<string> Diamond::getDetails() {
  vector<string> data;

  data.push_back("");
  data.push_back("Royal Diamonds");

  return data;
}

/*
Function:	promptMsg
Description:	gets message to display to user
Arguments:	Hero& - hero to prompt
Return:		string - message to display
*/
string Diamond::promptMsg(Hero &theHero) {
  string msg = "Congratulations! You've found the Royal Diamonds!";
  return msg;
}

/*
Function:	interact
Description:	interacts with user according to response key
Arguments:	char - prompt response key
                , Hero& - hero to interact with
Return:
*/
bool Diamond::interact(char promptResponse, Hero &theHero) { return true; }

string Diamond::typeStr() const { return "Diamond"; }

string Diamond::dataAsCsv() const { return ""; }

/*
/////////////////////////////////////////////////////////////////
Obstacle class derived from TileOccupant
/////////////////////////////////////////////////////////////////
*/

Obstacle::Obstacle(std::string name, int energyCost)
    : name_(name), energyCost_(energyCost) {}

std::string Obstacle::name() const { return name_; }

std::string Obstacle::promptMsg(Hero &theHero) {
  /* The proper operator overloads and implicit constructors are available in
   * <string> for the below statement to compile.
   */
  vector<Tool *> usableTools = theHero.getUsableTools(*this);

  if (usableTools.size() == 0) {
    return string("You destroyed a ") + name_ + " with your bare hands! " +
           "(You have no appropriate tools.)";
  }

  return string("You must remove a ") + name_ + " to continue. Select a tool" +
         " or press \"space\" for no tool.";
}

bool Obstacle::interact(char promptResponse, Hero &theHero) {
  // Get list of all tools that the user can use to break this obstacle
  vector<Tool *> usableTools = theHero.getUsableTools(*this);

  // Convert the tool choice to an index in the tool array
  int toolInd = charToChoiceIndex(promptResponse);

  // All input is valid if the user has no applicable tools.
  if (usableTools.size() == 0 || promptResponse == ' ') {
    theHero.addEnergy(-energyCost_);
    return true;
  }
  // A tool was chosen. Check if the promptResponse is invalid.
  if (toolInd < 0 || toolInd >= static_cast<int>(usableTools.size())) {
    return false;
  }

  // Check if the pointer to the chosen tool is null
  Tool *chosenTool = usableTools.at(toolInd);

  if (!chosenTool)
    throw std::runtime_error("missing tool");

  // Cost is reduced by a factor of the rating, rounding up.
  energyCost_ = ceil(static_cast<float>(energyCost_) / chosenTool->rating());

  theHero.addEnergy(-energyCost_);

  // Remove the tool from the Hero's inventory.
  theHero.consumeTool(chosenTool);
  return true;

  /* Like for any other TileOccupant, the caller will remove this Obstacle
   * from the map after verifying that it is not permanent. Similarly, caller
   * should check whether the hero has died.
   */
}

bool Obstacle::permanent() { return false; }

int Obstacle::color() { return COLOR_BLACK; }

char Obstacle::marker() { return '!'; }

std::vector<std::string> Obstacle::getDetails() {
  std::vector<std::string> details;

  details.push_back(name_);
  details.push_back(std::to_string(energyCost_));
  details.push_back("Obstacle");
  details.push_back("Energy cost");

  return details;
}

string Obstacle::typeStr() const { return "Obstacle"; }

string Obstacle::dataAsCsv() const {
  return name_ + "," + to_string(energyCost_);
}
#include "TileType.h"
#include <ncurses.h>

bool TileType::canEnter(const Hero &theHero) const {
  // By default, a tile can be entered.
  return true;
}
int TileType::energyCost() const {
  // By default, traveling through a tile costs 1 energy.
  return 1;
}

int Meadow::color() const { return COLOR_GREEN; }

std::string Meadow::toString() const { return "Meadow"; }

bool Wall::canEnter(const Hero &theHero) const { return false; }
int Wall::color() const { return COLOR_WHITE; }

std::string Wall::toString() const { return "Wall"; }

int Swamp::energyCost() const { return 2; }

int Swamp::color() const { return COLOR_MAGENTA; }

std::string Swamp::toString() const { return "Swamp"; }

bool Water::canEnter(const Hero &theHero) const { return theHero.hasShip(); }

int Water::energyCost() const {
  // Travel is only possible using ships, but does not cost energy.
  return 0;
}

int Water::color() const { return COLOR_BLUE; }

std::string Water::toString() const { return "Water"; }
#include "UI.h"

// Initializes all of the necessary windows
bool UserInterface::initialize(unsigned int width) {
  // Sets up the UI. Returns false if the screen is the wrong size.
  int scrY, scrX;
  int xp;
  int y, x;
  int yl, xl;

  getmaxyx(stdscr, scrY, scrX);
  if (scrY < MINY || scrX < MINX)
    return false;

  // xp = scrX * 3 / 10;
  xp = width;
  y = scrY;
  x = xp;
  yl = 0;
  xl = scrX - xp;

  // Create the windows.
  // b windows are borders only.
  bmain = newwin(y, x, yl, xl);

  // These are the side windows.
  y -= 2;
  x -= 4;
  yl += 1;
  xl += 2;
  info = newwin(y, x, yl, xl);
  inven = newwin(y, x, yl, xl);

  // Popup window border.
  float factor = 1.5;
  y = scrY / factor;
  x = (scrX - xp) / factor;
  yl = (scrY - y) / 2;
  xl = (scrX - xp - x) / 2;
  bpop = newwin(y, x, yl, xl);

  // Popup window.
  y -= 2;
  x -= 4;
  yl += 1;
  xl += 2;
  pop = newwin(y, x, yl, xl);

  // Add them to the panels.
  pbmain = new_panel(bmain);
  pinfo = new_panel(info);
  pinven = new_panel(inven);
  pbpop = new_panel(bpop);
  ppop = new_panel(pop);

  // Set borders.
  wborder(bmain, BVER, BVER, BHOR, BHOR, BVER, BVER, BVER, BVER);
  wborder(bpop, BVER, BVER, BHOR, BHOR, BVER, BVER, BVER, BVER);

  // Hide inventory and popup.
  hide_panel(pinven);
  hide_panel(pbpop);
  hide_panel(ppop);

  // Refresh the screen.
  update_panels();
  doupdate();
  return true;
}

///// Information window functions.

// Actions
void UserInterface::actions(std::vector<std::string> values) {
  // Prints the actions a player can take at the current time.
  int ypos = 0;
  int xoffset = -3;
  printTitle(info, ypos, xoffset, LABEL_Actions);
  printDualCol(info, ++ypos, xoffset, values);
  return;
}

// Tile inspect
void UserInterface::tileInspect(std::vector<std::string> values) {
  // Prints the information of a grovenik and its occupant.
  int ypos = getmaxy(info) / 2 - 2;
  int xoffset = -1;
  clearInspect();
  printTitle(info, ypos, xoffset, LABEL_Inspect);
  printDualCol(info, ++ypos, xoffset, values);
  return;
}
// Clears the inspect tile portion of the info window
void UserInterface::clearInspect() {
  int maxY;
  int ypos;

  maxY = getmaxy(info) - 3;
  ypos = getmaxy(info) / 2 - 2;

  while (ypos < maxY) {
    wmove(info, ypos, 0);
    wclrtoeol(info);
    ++ypos;
  }

  return;
}

// Whiffles & energy
void UserInterface::whifflesEnergy(int whiffles, int energy) {
  // Prints the number of whiffles and energy of the hero.
  std::vector<std::string> values;
  int ypos, xoffset;

  values = {std::to_string(whiffles), std::to_string(energy), LABEL_Whiffles,
            LABEL_Energy};
  ypos = getmaxy(info) - 3;
  xoffset = 0;

  printDualCol(info, ypos, xoffset, values);
  return;
}

// Display inventory
char UserInterface::displayInventory(
    std::vector<std::vector<std::string>> tools, bool getInput) {
  char ch;
  int ist;
  int y, cy, cx;
  int ts;

  show_panel(pinven);
  werase(inven);

  ist = 0;
  y = getmaxy(inven);
  cy = 0;
  ts = tools.size();

  do {
    // Print all the tools.
    werase(inven);
    for (int i = ist; i < ts; i++) {
      printDualCol(inven, cy, 0, tools[i]);
      getyx(inven, cy, cx);
      cy += 2;
      if (cy > y)
        break;
    }
    cy = 0;
    ch = getch();

    // Scroll upwards.
    if (ch == CTRL_UP) {
      ist -= 1;
      if (ist < 0)
        ist = 0;
    }
    // Scroll downwards.
    else if (ch == CTRL_DOWN) {
      if (ist < ts)
        ist++;
    } else if (getInput && ch != CTRL_INV) {
      break;
    }
  } while (ch != CTRL_INV);

  hide_panel(pinven);
  update_panels();
  doupdate();
  return ch;
}

//////// Popups

// Popup with single string
char UserInterface::popup(std::string message) {
  char ch;

  openPop();
  printTitle(pop, 0, 0, message);
  ch = getch();
  closePop();

  return ch;
}

// Popup with a string and an occupant
char UserInterface::popup(std::string message,
                          std::vector<std::string> values) {
  char ch;
  int ypos;

  openPop();
  // Print message
  printTitle(pop, 0, 0, message);
  // Print occupant details
  ypos = getmaxy(pop) - values.size() / 2 - 1;
  printDualCol(pop, ypos, 0, values);
  // Wait for input
  ch = getch();
  closePop();

  return ch;
}

// Popup with a string, obstacle, and list of tools
char UserInterface::popup(std::string message,
                          std::vector<std::string> obstacle,
                          std::vector<std::vector<std::string>> tools) {
  char ch;
  int ypos;

  openPop();
  // Print message
  printTitle(pop, 0, 0, message);
  // Print occupant
  ypos = getmaxy(pop) - obstacle.size() / 2 - 1;
  printDualCol(pop, ypos, 0, obstacle);
  // Display inventory and get input.
  ch = displayInventory(tools, true);
  closePop();

  return ch;
}

//////// Private functions

// Opens the popup and erases all text.
void UserInterface::openPop() {
  show_panel(pbpop);
  show_panel(ppop);
  werase(pop);
  return;
}
// Hides the popup and updates panels.
void UserInterface::closePop() {
  hide_panel(pbpop);
  hide_panel(ppop);
  update_panels();
  doupdate();
  return;
}

// Prints a string in the width-center of the window
void UserInterface::printTitle(WINDOW *win, int ypos, int hor_offset,
                               std::string title) {
  // Prints the title in the center.
  int start = (getmaxx(win) - title.length()) / 2;

  if (start < 0)
    start = 0;

  mvwaddstr(win, ypos, start, title.data());
  return;
}

// Print dual columns of information
void UserInterface::printDualCol(
    // The main method of printing information to the UI elements.
    WINDOW *win, int ypos, int hor_offset, std::vector<std::string> values) {
  int maxY, maxX;
  int center, start;
  int vcen;
  std::string str;

  // Find center of window and vector.
  getmaxyx(win, maxY, maxX);
  center = maxX / 2;
  vcen = values.size() / 2;

  // Prints through the vector.
  for (int i = 0; i < vcen; i++) {
    if (ypos >= maxY)
      break;

    // Clear the line of old text.
    wmove(win, ypos, 0);
    wclrtoeol(win);

    // Create the string and center it correctly.
    str = values[i] + " : " + values[vcen + i];
    start = center - str.find_first_of(':') + hor_offset;

    int s = str.length();
    if (start < 0)
      start = 0;

    // Print the string.
    mvwaddstr(win, ypos, start, str.data());
    // If string is too wide, update ypos.
    while (s > maxX) {
      ++ypos;
      s -= maxX;
    }
    ++ypos;
  }

  update_panels();
  doupdate();
  return;
}
#ifndef CsvToOccupant_CLASS
#define CsvToOccupant_CLASS

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "TileOccupant.h"

    using std::getline;
using std::ifstream;
using std::stoi;
using std::string;
using std::stringstream;
using std::vector;

TileOccupant *newOccupant(const string &type, const string &csv);

TileOccupant *csvToFood(const string &csv);

TileOccupant *csvToBinoculars(const string &csv);

TileOccupant *csvToShip(const string &csv);

TileOccupant *csvToTreasure(const string &csv);

TileOccupant *csvToClue(const string &csv);

TileOccupant *csvToDiamond(const string &csv);

TileOccupant *csvToObstacle(const string &csv);

TileOccupant *csvToTool(const string &csv);

vector<string> inputFile(string src);

vector<string> cleanFile(vector<string> src);

#endif
/*
 GameState Class
 Author: Mohamed-Amin Cheaito
 November 2020
*/
#ifndef GameState_CLASS
#define GameState_CLASS

#include "Hero.h"
#include "Map.h"
#include "UI.h"
#include <ncurses.h>

// Map coordinates (within array)
class GameState {
public:
  GameState(string mapFile = "Frupal.txt");
  ~GameState();

  void travel(int &direction, WINDOW *win);
  void cursorTravel(int direction);
  bool HeroTravel(int &direction);
  bool ExpandMap();
  bool occupantCheck(int &direction);
  void HeroVision();
  void HeroVision(int tempHeroY, int tempHeroX);
  void revealMap();
  void RunGame(WINDOW *win);
  void CursorInspect();

private:
  Hero theHero;
  Map map;
  UserInterface UI;
  vector<string> message;
  int cursorX;
  int cursorY;
  int heroX;
  int heroY;
  int flagCursor;
};
#endif
/*
Hero class header
November 2020
*/

#ifndef Hero_CLASS
#define Hero_CLASS

#include "Map.h"
#include "TileOccupant.h"
#include <vector>

using namespace std;

class Obstacle;
class Tool;

class Hero {
public:
  Hero();
  Hero(const Hero &);
  vector<Tool *> getUsableTools(Obstacle &);
  vector<vector<string>> getToolOptions(Obstacle &);
  void addInventory(Tool *);
  bool consumeTool(Tool *);
  int visionRange(void) const;
  bool hasShip(void) const;
  int whiffles(void) const;
  int energy(void) const;
  int addEnergy(int);
  int addWhiffles(int);
  void setHasBinoculars(bool);
  void setHasShip(bool);
  vector<vector<string>> GetInventory() const;

private:
  vector<Tool *> inventory_;
  bool hasBinoculars_;
  bool hasShip_;
  int whiffles_;
  int energy_;
};

#endif
/*
 Map Class
 Author: Mohamed-Amin Cheaito
 November 2020
*/

using namespace std;

#ifndef Map_CLASS
#define Map_CLASS

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <vector>

class TileOccupant;
class TileType;

struct Tile {
  Tile();
  ~Tile();

  bool revealed;
  TileType *type;
  TileOccupant *occupant;
};

class Map {
public:
  // Screen size
  // static int MaxScreenX = 0; //COLS

  int MaxScreenY; // LINES

  // The max we can go on the screen
  int MenuBorder; // MaxScreenX for frupal Map

  static const int MAPSIZE = 128;
  int MaxX;
  int MaxY;
  int MinX;
  int MinY;

  Map() = delete;
  Map(string srcFile, int &heroX, int &heroY);

  TileType *tileTypeAt(int col, int row);
  TileOccupant *occupantAt(int col, int row);

  void setOccupantAt(int col, int row, TileOccupant *newOccupant);

  void tile_revealed(int row, int col);
  void revealAll();

  bool isDebarkSafe(int col, int row);
  bool isTileDiscovered(int col, int row);

  void displayMap(WINDOW *win);

  bool loadFile(string src, int &heroX, int &heroY);
  bool loadOccupants(string src);

  // Write to a file
  bool saveFile(string dest, int heroX, int heroY);
  bool saveOccupants(string dest);

private:
  // vector<vector<Tile>> tileArray;
  Tile tileArray[MAPSIZE][MAPSIZE];
};
#endif
/*
FileName:	TileOccupant.h
Author:		Drew McKinney
Group:		9
Description:
                TileOccupant and derived classes header file
*/

#ifndef TileOccupant_CLASS
#define TileOccupant_CLASS

#include "Hero.h"
#include <ncurses.h>
#include <stdexcept>
#include <string>
#include <vector>

class Hero;

using namespace std;

// Used by Obstacle to validate user choice of tool
int charToChoiceIndex(char c);
char choiceIndexToChar(int ind);

class TileOccupant {
public:
  TileOccupant();
  virtual ~TileOccupant();
  virtual bool permanent() = 0;
  virtual int color() = 0;
  virtual char marker() = 0;
  virtual vector<string> getDetails() = 0;
  virtual string promptMsg(Hero &theHero) = 0;
  virtual bool interact(char promptResponse, Hero &theHero);
  virtual string typeStr() const = 0;
  virtual string dataAsCsv() const = 0;

protected:
};

class Treasure : public TileOccupant {
public:
  Treasure();
  Treasure(int worth);
  bool permanent() override;
  int color() override;
  char marker() override;
  vector<string> getDetails() override;
  string promptMsg(Hero &theHero) override;
  bool interact(char promptResponse, Hero &theHero) override;
  string typeStr() const override;
  string dataAsCsv() const override;

protected:
  int worth_;
};

class Ship : public TileOccupant {
public:
  Ship();
  Ship(int whiffleCost, bool bought = false);
  bool permanent() override;
  int color() override;
  char marker() override;
  vector<string> getDetails() override;
  string promptMsg(Hero &theHero) override;
  bool interact(char promptResponse, Hero &theHero) override;
  string typeStr() const override;
  string dataAsCsv() const override;

protected:
  int whiffleCost_;
  bool bought_;
  bool toRemove_;
};

class Obstacle : public TileOccupant {
public:
  Obstacle() = delete;
  Obstacle(string name, int energyCost);

  string name() const;
  string promptMsg(Hero &theHero) override;
  bool interact(char promptResponse, Hero &theHero) override;
  bool permanent() override;
  int color() override;
  char marker() override;
  vector<string> getDetails() override;
  string typeStr() const override;
  string dataAsCsv() const override;

private:
  string name_;
  int energyCost_;
};

class Tool : public TileOccupant {
public:
  Tool();
  Tool(const Tool &obj);
  Tool(string name, int whiffleCost, int rating, vector<string> obst);
  bool usableOn(const Obstacle &onObstacle) const;
  int rating();
  bool permanent() override;
  int color() override;
  char marker() override;
  vector<string> getDetails() override;
  string promptMsg(Hero &theHero) override;
  bool interact(char promptResponse, Hero &theHero) override;
  string typeStr() const override;
  string dataAsCsv() const override;

protected:
  string name_;
  int whiffleCost_;
  int rating_;
  vector<string> forObstacles;
  bool bought_;
};

class Food : public TileOccupant {
public:
  Food();
  Food(string name, int whiffleCost, int energyProvided);
  bool permanent() override;
  int color() override;
  char marker() override;
  vector<string> getDetails() override;
  string promptMsg(Hero &theHero) override;
  bool interact(char promptResponse, Hero &theHero) override;
  string typeStr() const override;
  string dataAsCsv() const override;

protected:
  string name_;
  int whiffleCost_;
  int energyProvided_;
  bool consumed_;
};

class Clue : public TileOccupant {
public:
  Clue();
  Clue(string msg);
  bool permanent() override;
  int color() override;
  char marker() override;
  vector<string> getDetails() override;
  string promptMsg(Hero &theHero) override;
  bool interact(char promptResponse, Hero &theHero) override;
  string typeStr() const override;
  string dataAsCsv() const override;

protected:
  string msg_;
};

class Diamond : public TileOccupant {
public:
  Diamond();
  bool permanent();
  int color() override;
  char marker() override;
  vector<string> getDetails() override;
  string promptMsg(Hero &theHero) override;
  bool interact(char promptResponse, Hero &theHero) override;
  string typeStr() const override;
  string dataAsCsv() const override;

protected:
};

class Binoculars : public TileOccupant {
public:
  Binoculars();
  Binoculars(int whiffleCost);

  string promptMsg(Hero &theHero) override;
  bool interact(char promptResponse, Hero &theHero) override;
  bool permanent() override;
  int color() override;
  char marker() override;
  vector<string> getDetails() override;
  string typeStr() const override;
  string dataAsCsv() const override;

private:
  int whiffleCost_;
  bool bought_;
};

#endif
#ifndef TileType_CLASS
#define TileType_CLASS

#include "Hero.h"
#include <string>

class Hero;

class TileType {
public:
  virtual ~TileType() {}
  virtual bool canEnter(const Hero &theHero) const;
  virtual int energyCost() const;
  virtual int color() const = 0;
  virtual std::string toString() const = 0;
};

class Meadow : public TileType {
  // Destructor not declared since default is sufficient.
  int color() const override;
  std::string toString() const override;
};

class Wall : public TileType {
  bool canEnter(const Hero &theHero) const override;
  int color() const override;
  std::string toString() const override;
};

class Swamp : public TileType {
  int energyCost() const override;
  int color() const override;
  std::string toString() const override;
};

class Water : public TileType {
  bool canEnter(const Hero &theHero) const override;
  int energyCost() const override;
  int color() const override;
  std::string toString() const override;
};

#endif
/*
UI Class
Author: Thomas Abel
Date: 2020-11-19
*/
#ifndef UI_CLASS
#define UI_CLASS
#include <ncurses.h>
#include <panel.h>
#include <string>
#include <vector>

class UserInterface {
public:
  // Starts the UI by creating all necessary windows/panels.
  bool initialize(unsigned int width = WIDTH);
  // Displays information on the side window.
  void actions(std::vector<std::string> values);
  void tileInspect(std::vector<std::string> values);
  void whifflesEnergy(int whiffles, int energy);
  // Opens a new panel to show the player inventory.
  char displayInventory(std::vector<std::vector<std::string>> tools,
                        bool getInput = false);
  char popup(std::string msg);
  char popup(std::string msg, std::vector<std::string> values);
  char popup(std::string msg, std::vector<std::string> obstacle,
             std::vector<std::vector<std::string>> values);

private:
  const char CTRL_UP = 'i';
  const char CTRL_DOWN = 'k';
  const char CTRL_INV = 'h';
  // The minimum screensize allowed by the UI.
  const int MINY = 20;
  const int MINX = 80;
  const static unsigned int WIDTH = 24;
  // The border characters.
  const char BVER = '#';
  const char BHOR = '=';
  // Titles for each of the sections.
  const std::string LABEL_Whiffles = "Whiffles";
  const std::string LABEL_Energy = "Energy";
  const std::string LABEL_Actions = "~Actions~";
  const std::string LABEL_Inspect = "~Inspect~";

  void clearInspect();
  // Popup helpers
  void openPop();
  void closePop();

  // Printing Functions
  void printTitle(WINDOW *win, int ypos, int hor_offset, std::string title);
  void printDualCol(WINDOW *win, int ver_offset, int hor_offset,
                    std::vector<std::string> values);

  // Windows and Panels
  WINDOW *info, *inven, *pop;
  WINDOW *bmain, *bpop;
  PANEL *pbmain, *pinfo, *pinven, *pbpop, *ppop;
};

#endif
/*
initialize() -> Call this when first starting the UI.

~Main Information Panel~
These functions display to the main panel on the right side
of the screen. Update these whenever.

Actions() -> The vector should contain strings in a vector in the order:
    [controls, labels]
    eg : ["E", "S", "D", "F", "North", "West", "South", "East"]

Inspect() -> The vector should contain strings in a vector in the order:
    [values, labels]
    eg : ["Steak", "10", "20", "Food", "Cost", "Energy"]

WhiffEn -> Simply provide the current whiffle and energy values.

~Inventory Panel~

~Popup/Prompt Panel~
*/
