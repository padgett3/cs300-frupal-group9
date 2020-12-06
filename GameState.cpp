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
  
  // End the game if the Hero is out of energy.
  if (theHero.energy() <= 0) {
    char response;
    do {
      response = UI.popup(string("You ran out of energy and can no longer") +
                              "move! Game over. Press 'q' to quit. ",
                          vector<string>{});
    } while (response != 'q');
    direction = 'q';
  }
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
  /*
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
  */

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
