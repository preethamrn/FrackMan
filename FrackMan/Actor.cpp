#include "Actor.h"
#include "StudentWorld.h"

//Actor functions
Actor::Actor(int ID, int x, int y, Direction dir, float size, int depth, StudentWorld *sw) : GraphObject(ID, x, y, dir, size, depth), sWorld(sw) {}
inline Actor::~Actor() {}

//Dirt functions
Dirt::Dirt(int x, int y) : GraphObject(IID_DIRT, x, y, right, 0.25, 3) { setVisible(true); }


//Boulder functions
Boulder::Boulder(int x, int y, StudentWorld *sw) : Actor(IID_BOULDER, x, y, down, 1.0, 1, sw), falling(false) { setVisible(true); }
int Boulder::doSomething() {
	return CONTINUE;
}

//GoldNugget functions
GoldNugget::GoldNugget(int x, int y, bool isVisible, bool isPlayerPickable, bool isPermanent, StudentWorld *sw) : Actor(IID_GOLD, x, y, right, 1.0, 2, sw) {}
int GoldNugget::doSomething() {
	return CONTINUE;
}


//OilBarrel functions
OilBarrel::OilBarrel(int x, int y, StudentWorld *sw) : Actor(IID_BARREL, x, y, right, 1.0, 2, sw) {}
int OilBarrel::doSomething() {
	return CONTINUE;
}

//Goodie functions
Goodie::Goodie(int ID, int x, int y, int t, StudentWorld *sw) : Actor(ID, x, y, right, 1.0, 2, sw), ticks(t) { setVisible(true); }
int Goodie::doSomething() {
	if (!ticks) return SELF_DIED;
	//check if picked up and increases score by points
	ticks--;
	return CONTINUE;
}

//WaterPool functions
WaterPool::WaterPool(int x, int y, int t, StudentWorld *sw) : Goodie(IID_WATER_POOL, x, y, t, sw) {}
int WaterPool::doSomething() {
	int ret = Goodie::doSomething();
	if (ret == PICKED_UP) {
		//give frackman waterpool and increase score
		return SELF_DIED;
	}
	return ret;
}

//SonarKit functions
SonarKit::SonarKit(int x, int y, int t, StudentWorld *sw) : Goodie(IID_SONAR, x, y, t, sw) {}
int SonarKit::doSomething() {
	int ret = Goodie::doSomething();
	if (ret == PICKED_UP) {
		//give frackman sonarkit and increase score
		return SELF_DIED;
	}
	return ret;
}

//Protester functions
Protester::Protester(int ID, int x, int y, StudentWorld *sw) : Actor(ID, x, y, left, 1.0, 0, sw) { setVisible(true); }
inline Protester::~Protester() {}

//Regular Protester functions
RegularProtester::RegularProtester(int x, int y, StudentWorld *sw) : Protester(IID_PROTESTER, x, y, sw) {}
int RegularProtester::doSomething() {
	return CONTINUE;
}

//HardcoreProtester functions
HardcoreProtester::HardcoreProtester(int x, int y, StudentWorld *sw) : Protester(IID_HARD_CORE_PROTESTER, x, y, sw) {}
int HardcoreProtester::doSomething() {
	return CONTINUE;
}

//FrackMan functions
FrackMan::FrackMan(StudentWorld *sw) : GraphObject(IID_PLAYER, 30, 60, right, 1.0, 0), health(10), water(5), sonar(1), gold(0), sWorld(sw) {
	setVisible(true);
}
int FrackMan::doSomething() {
	int ch;
	if (sWorld->getKey(ch)) {
		switch (ch) {
		case KEY_PRESS_DOWN: break;
		case KEY_PRESS_LEFT: break;
		case KEY_PRESS_RIGHT: break;
		case KEY_PRESS_UP: break;
		case KEY_PRESS_TAB: break;
		case KEY_PRESS_SPACE: break;
		case KEY_PRESS_ESCAPE: return Actor::PLAYER_DIED;
		default: break;
		}
	}
	return Actor::CONTINUE;
}