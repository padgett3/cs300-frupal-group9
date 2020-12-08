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

  msg = "Binoculars found!";
  if (theHero.whiffles() >= whiffleCost_) {
    msg = msg + " Buy Binoculars and double your " + "vision for " +
          to_string(whiffleCost_) + " Whiffles? (Y/N):";
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
