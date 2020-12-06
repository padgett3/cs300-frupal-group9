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
