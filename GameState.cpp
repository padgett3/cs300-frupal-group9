#include "GameState.h"

GameState::GameState(string MapsrcFile) : map(MapsrcFile) {
  // Where should the player start?
  heroX = 0;
  heroY = 0;

  // Should start at Hero position
  cursorX = heroX;
  cursorY = heroY;
}

// Main travel function
void GameState::travel(char &direction, WINDOW *win, WINDOW *menu) {
  // Only Time this doesn't enter is when we
  // move the cursor or exit the program due to not having
  // enough energy (Should Also not enter when Diamond is found)
  if (HeroTravel(direction, menu)) {

    // Enter if want to continue to explore the map
    if (ExpandMap()) {
      // Clear screen
      for (int i = 0; i < MaxScreenY; ++i) {
        for (int j = 0; j < MenuBorder; ++j) {
          attron(COLOR_PAIR(3));
          mvwprintw(win, i, j, " ");
        }
      }
    }

    // What the hero can see
    HeroVision();

    map.displayMap(win);

    UI.WhiffEn(theHero.whiffles(), theHero.energy());

    wattron(win, COLOR_PAIR(6));
    mvwprintw(win, heroY, heroX, "@");
    wrefresh(win);
  }

  wmove(win, cursorY, cursorX);
  wrefresh(win);
}

// The hero traveling
bool GameState::HeroTravel(char &direction, WINDOW *menu) {
  TileType *temp_type = NULL;

  switch (direction) {
  // These four cases is when user wants
  // to move the Hero.
  case 'i':
    if (heroY - 1 >= -1) {
      if (heroY - 1 == -1)
        --heroY;
      else {
        // Move hero up
        --heroY;
        temp_type = map.tileTypeAt(heroX + MinX, heroY + MinY);
        // Check if we can enter
        if (temp_type->CanEnter(theHero)) {
          // Make sure take away the energy needed to get here
          temp_type->energyCost(theHero);
          // Make sure we still have enough energy
          if (theHero.energy() <= 0) {
            // If we don't then exit the program
            direction = 'q';
            return false;
          }
          // Check our occupant
          return occupantCheck(direction, menu);
        } else {
          ++heroY;
          return false;
        }
      }
    } else
      heroY = 0;
    break;

  case 'k':
    if (heroY + 1 < MaxScreenY + 1) {
      if (heroY + 1 == MaxScreenY)
        ++heroY;
      else {
        // Same as above but move down
        ++heroY;
        temp_type = map.tileTypeAt(heroX + MinX, heroY + MinY);
        if (temp_type->CanEnter(theHero)) {
          temp_type->energyCost(theHero);
          if (theHero.energy() <= 0) {
            direction = 'q';
            return false;
          }
          return occupantCheck(direction, menu);

        } else {
          --heroY;
          return false;
        }
      }

    } else
      heroY = MaxScreenY - 1;
    break;

  case 'l':
    if (heroX + 1 < MenuBorder + 1) {
      if (heroX + 1 == MenuBorder)
        ++heroX;
      else {
        // Move right
        ++heroX;
        temp_type = map.tileTypeAt(heroX + MinX, heroY + MinY);
        if (temp_type->CanEnter(theHero)) {
          temp_type->energyCost(theHero);
          if (theHero.energy() <= 0) {
            direction = 'q';
            return false;
          }
          return occupantCheck(direction, menu);

        } else {
          --heroX;
          return false;
        }
      }

    } else
      heroX = MenuBorder - 1;

    break;
  case 'j':
    if (heroX - 1 >= -1) {
      if (heroX - 1 == -1)
        --heroX;
      else {
        // Move left
        --heroX;
        temp_type = map.tileTypeAt(heroX + MinX, heroY + MinY);
        if (temp_type->CanEnter(theHero)) {
          temp_type->energyCost(theHero);
          if (theHero.energy() <= 0) {
            direction = 'q';
            return false;
          }
          return occupantCheck(direction, menu);

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
bool GameState::occupantCheck(char &direction, WINDOW *win) {
  
  /* "Row" does not correspond to the horizontal axis, so this is questionable
   * at best.
   */
  int r = heroX + MinX;
  int c = heroY + MinY;
  TileType *tileType = map.tileAt(r, c);
  TileOccupant *occ = map.occupantAt(r, c);

  // Not NULL, we have an occupant
  if (occ) {
    char response = 0;

    // Keep prompting user until they provide a valid response.
    do {
      /* Give the user the appropriate pop-up for the encounter.
       * TileOccupant::promptMsg() will give an appropriate message if
       * the user cannot afford an item.
       */
      response = UI.popup(occ->promptMsg(), occ->getDetails());

      // If encountering an Obstacle, the user needs to see their tool choices.
      if (occ->typeStr() == "Obstacle") {
        Obstacle *o = dynamic_cast<Obstacle*>(occ);
        UI.displayInventory(theHero.getToolOptions(*o));
      }
      
    } while (!occ->interact(response, theHero);
    
    /* End the game if the Hero is out of energy. The user has been
     * notified via pop-up already.
     */
    if (theHero.energy <= 0) {
      direction = 'q';
      return false;
    }

    /* End the game if the Hero found a diamond. The user has been
     * notified via pop-up already.
     */
    if (occ->typeStr() == "Diamond") {
      direction = 'w'; // 'w' for "win"? Or is that what "return true" is for?
      return false;
    }

    /* Check whether the tileOccupant should still exist after the interaction.
     * If not, remove it from the map.
     */
    if (!occ->permanent()) {
      delete occ;
      map.setOccupantAt(r, c, 0);
    }
 
  }

  /* If the hero is leaving water, then they leave their ship on the shore.
   * Since this ship was already purchased, it has no cost.
   */
  if (theHero.hasShip() && tileType->toString() != "Water") {
      map.setOccupantAt(r, c, new Ship(0, true))
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

  if ((tempHeroY < MaxScreenY && tempHeroY >= 0) &&
      (tempHeroX < MenuBorder && tempHeroX >= 0)) {
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
      if ((checkI >= 0 && checkI < MaxScreenY) &&
          (checkJ >= 0 && checkJ < MenuBorder)) {
        // Tile is discovered, set it to true
        map.tile_revealed(checkI + MinY, checkJ + MinX);
        // array[checkI+MinY][checkJ+MinX].used = true;
      }
      if (k == 6)
        // check down
        checkI = i + 1;
      else
        // check right
        checkJ = j + 1;
    }
  }
}
// Inspect tiles with cursor
void GameState::cursorTravel(char direction) {
  TileType *temp_type = NULL;
  TileOccupant *occupant_temp = NULL;
  string temp;
  vector<string> details;

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
    if (cursorY + 1 < MaxScreenY)
      ++cursorY;
    else
      cursorY = MaxScreenY - 1;

    break;

  case 'f':
    if (cursorX + 1 < MenuBorder)
      ++cursorX;
    else
      cursorX = MenuBorder - 1;
    break;

  case 's':
    if (cursorX - 1 > 0)
      --cursorX;
    else
      cursorX = 0;
    break;
  }
  // Pass in tileType and Occupant to inspect

  if (isTileDiscovered(cursorX + MinX, cursorY + MinY)) {
    temp_type = map.tileTypeAt(cursorX + MinX, cursorY + MinY);
    occupant_temp = map.occupantAt(cursorX + MinX, cursorY + MinY);

    if (occupant_temp) {
      details = occupant_temp->getdetails();
      //  if(details.size() % 2 == 0)
      //  {
      //  I think that details should give an even vector.
      int i = details.size() / 2;
      temp = temp_type.toString();
      details.insert(details.begin(), temp);
      details.insert(details.begin() + i, "Grovnick");
      UI.tileInspect(details);

      //  }

    } else {
      temp = temp_type.toString();
      details.push_back(temp);
      details.push_back("Grovnick");
      UI.tileInspect(details);
    }
  }

  // UI.tileInspect(temp_type, occupant_temp);
}

bool GameState::ExpandMap() {
  int temp;

  // Explore down the map
  if (heroY == MaxScreenY) {
    --heroY;

    temp = heroY + MinY;

    MinY = MaxY - (MaxScreenY / 2);
    MaxY = MaxY + (MaxScreenY / 2);

    // Account for ODD #
    if (MaxY - MinY < MaxScreenY)
      ++MaxY;

    if (MaxY > (MAPSIZE - 1)) {
      MaxY = MAPSIZE;
      MinY = MAPSIZE - MaxScreenY;
    }

    heroY = abs((temp - MinY));
    heroX = heroX;
    return true;

  }
  // Go back up
  else if (heroY == -1) {
    ++heroY;
    temp = heroY + MinY;

    MinY -= (MaxScreenY / 2);
    MaxY -= (MaxScreenY / 2);

    if (MinY <= -1) {
      MaxY = MaxScreenY;
      MinY = 0;
    }

    heroY = abs((temp - MinY));
    heroX = heroX;
    return true;

  }

  // Explore right
  else if (heroX == MenuBorder) {
    --heroX;

    temp = heroX + MinX;

    MinX = MaxX - (MenuBorder / 2);
    MaxX = MaxX + (MenuBorder / 2);

    // Account for ODD #
    if (MaxX - MinX < MenuBorder)
      ++MaxX;

    if (MaxX > (MAPSIZE - 1)) {
      MaxX = MAPSIZE;
      MinX = MAPSIZE - MenuBorder;
    }

    heroX = abs(temp - MinX);
    heroY = heroY;
    return true;
  }

  // explore left
  else if (heroX == -1) {
    ++heroX;

    if (MinX != 0) {

      temp = heroX + MinX;

      MinX -= (MenuBorder / 2);
      MaxX -= (MenuBorder / 2);

      if (MinX <= -1) {
        MaxX = MenuBorder;
        MinX = 0;
      }

      heroX = abs(temp - MinX);
      heroY = heroY;
      return true;
    }
  }
  return false;
}
