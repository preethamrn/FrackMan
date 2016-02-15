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
GoldNugget::GoldNugget(int x, int y, bool isVisible, bool isPlayerPickable, bool isPermanent, StudentWorld *sw) : Actor(IID_GOLD, x, y, right, 1.0, 2, sw) { setVisible(true); }
int GoldNugget::doSomething() {
	return CONTINUE;
}
///DEBUGGING //nuggets and barrels should be invisible

//OilBarrel functions
OilBarrel::OilBarrel(int x, int y, StudentWorld *sw) : Actor(IID_BARREL, x, y, right, 1.0, 2, sw) { setVisible(true); }
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
	int originalX = getX(), originalY = getY();
	if (sWorld->getKey(ch)) {
		switch (ch) {
		case KEY_PRESS_UP: 
			if (getDirection() == up) moveTo(getX(), getY() + 1); 
			else setDirection(up); break;
		case KEY_PRESS_DOWN: 
			if (getDirection() == down) moveTo(getX(), getY() - 1); 
			else setDirection(down); break;
		case KEY_PRESS_RIGHT: 
			if (getDirection() == right) moveTo(getX() + 1, getY()); 
			else setDirection(right); break;
		case KEY_PRESS_LEFT: 
			if (getDirection() == left) moveTo(getX() - 1, getY()); 
			else setDirection(left); break;
		case KEY_PRESS_TAB: 
			if (gold > 0) {
				gold--;
				sWorld->useGold();
			}
			break;
		case KEY_PRESS_SPACE: 
			if (water > 0) {
				water--;
				sWorld->useWater();
			}
			break;
		case 'z':
			if (sonar > 0) {
				sonar--;
				sWorld->useSonar();
			}
			break;
		case KEY_PRESS_ESCAPE: return Actor::PLAYER_DIED;
		default: break;
		}

		//check if collided with edges
		if (getX() < 0 || getY() < 0 || getX() > VIEW_WIDTH - 4 * getSize() || getY() > VIEW_HEIGHT - 4 * getSize()) moveTo(originalX, originalY);
		//check if collided with dirt
		bool removedDirt = false;
		for (int i = getX(); i < getX() + 4; i++) {
			for (int j = getY(); j < getY() + 4; j++) {
				if (sWorld->isDirt(i, j)) {
					sWorld->removeDirt(i, j);
					removedDirt = true;
				}
			}
		}
		if (removedDirt) sWorld->playSound(SOUND_DIG);

		//check if collided with actors
		std::vector<Actor*> *actors = sWorld->getActors();
		for (std::vector<Actor*>::const_iterator it = actors->begin(); it != actors->end(); it++) {
			//check boulders first
			if (dynamic_cast<Boulder*>(*it)) {
				if (sWorld->collides(*it, this)) {
					moveTo(originalX, originalY);
					break;
				}
			}
		}

		///DO ALL THIS STUFF IN THE OTHER DO SOMETHING FUNCTIONS
		///for barrels of oil, struckOil == true when level is won
		/*
		for (int i = 0; i < actors->size(); i++) {
			if (sWorld->collides((*actors)[i], this)) {
				Actor *actor = (*actors)[i];
				if (dynamic_cast<OilBarrel*>(actor)) { sWorld->struckOil(); delete actor; actors->erase(actors->begin() + i); i--; }
				else if (dynamic_cast<WaterPool*>(actor)) { }
				else if (dynamic_cast<SonarKit*>(actor)) { }
				else if (dynamic_cast<Protester*>(actor)) { } //do the protesters do different things??? ///DEBUGGING
				else if (dynamic_cast<GoldNugget*>(actor)) { }
			}
		}
		*/
		//check collisions now and destroy dirt, get goodies, hit protesters, boulders, etc (and move back if the position is solid/edge).
	}
	if(health > 0) return Actor::CONTINUE;
	else return Actor::PLAYER_DIED;
}