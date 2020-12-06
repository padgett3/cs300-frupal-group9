#include "Map.h"
#include "CsvToOccupant.h"
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
  if (MenuBorder > 182)
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
