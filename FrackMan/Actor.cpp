#include "Actor.h"
#include "StudentWorld.h"

//Actor functions
Actor::Actor(int ID, int x, int y, Direction dir, float size, int depth) : GraphObject(ID, x, y, dir, size, depth) {}


//Dirt functions
Dirt::Dirt(int x, int y) : GraphObject(IID_DIRT, x, y, right, 0.25, 3) {}


//Boulder functions
Boulder::Boulder(int x, int y) : Actor(IID_BOULDER, x, y, down, 1.0, 1) {}


//GoldNugget functions
GoldNugget::GoldNugget(int x, int y, bool isVisible, bool isPlayerPickable, bool isPermanent) : Actor(IID_GOLD, x, y, right, 1.0, 2) {}

//OilBarrel functions
OilBarrel::OilBarrel(int x, int y) : Actor(IID_BARREL, x, y, right, 1.0, 2) {}


//Protester functions
Protester::Protester(int ID, int x, int y) : Actor(ID, x, y, left, 1.0, 0) {}


//HardcoreProtester functions
HardcoreProtester::HardcoreProtester(int x, int y) : Protester(IID_HARD_CORE_PROTESTER, x, y) {}


//FrackMan functions
FrackMan::FrackMan() : Actor(IID_PLAYER, 30, 60, right, 1.0, 0) {}