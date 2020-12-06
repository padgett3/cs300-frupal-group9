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
