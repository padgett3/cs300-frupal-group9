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
