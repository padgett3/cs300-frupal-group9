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
