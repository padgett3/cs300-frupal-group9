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