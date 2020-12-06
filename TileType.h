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
