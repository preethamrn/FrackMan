#include "Actor.h"
#include "StudentWorld.h"

//BFSSearch functions
BFSSearch::BFSSearch() {
	lastX = -1; lastY = -1;
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++)
			movable[i][j] = false;
}



//Actor functions
Actor::Actor(int ID, int x, int y, Direction dir, float size, int depth, StudentWorld *sw) : GraphObject(ID, x, y, dir, size, depth), sWorld(sw) {}
inline Actor::~Actor() {}
StudentWorld* Actor::getStudentWorld() { return sWorld; }

//Dirt functions
Dirt::Dirt(int x, int y, StudentWorld *sw) : Actor(IID_DIRT, x, y, right, 0.25, 3, sw) { setVisible(true); }


//Boulder functions
Boulder::Boulder(int x, int y, StudentWorld *sw) : Actor(IID_BOULDER, x, y, down, 1.0, 1, sw), state(stable), ticks(30) { setVisible(true); }
int Boulder::doSomething() {
	//if all waiting ticks have elapsed, start falling
	if (!ticks) {
		getStudentWorld()->playSound(SOUND_FALLING_ROCK);
		state = falling;
		ticks = 30;
	}
	//check if waiting
	if (state == waiting) ticks--;
	else if (state == stable) {
		state = waiting;
		for (int i = 0; i < 4; i++) {
			if (getStudentWorld()->isDirt(getX() + i, getY() - 1)) state = stable;
		} //state becomes waiting if there's no dirt underneath rock
	}
	if (state == falling) {
		bool dead = false;



		moveTo(getX(), getY() - 1);
		if (getY() < 0) dead = true; //hit the bottom
		for (int i = 0; i < 4; i++) {
			if (getStudentWorld()->isDirt(getX() + i, getY())) dead = true; //hit dirt
		}
		//check collisions with other objects
		int ret = getStudentWorld()->boulderCollisions(this);
		if (ret == SELF_DIED) dead = true; //hit boulder
		else if (ret == PLAYER_DIED) return PLAYER_DIED; //hit frackman

		if (dead) {
			updateSearch(); return SELF_DIED; //hit something to cause it to die. must update movable space.
		}
		else {
			getStudentWorld()->getSearch()->setNotMovable(getX(), getY()); //set current position to not movable (occupied by boulder)
			//decide whether to set spot above it to movable
			bool movable = true;
			for (int i = getX(); i < getX() + 4; i++) {
				for (int j = getY() + 4; j < getY() + 8; j++) {
					if (getStudentWorld()->isDirt(i, j)) {
						movable = false;
						break;
					}
				}
				if (!movable) break;
			}
			if (movable) getStudentWorld()->getSearch()->setMovable(getX(), getY()+4);
		}
	}
	return CONTINUE;
}

void Boulder::updateSearch() {
	int x = getX(), y = getY();
	for (int i = 1; i <= 4; i++)
		getStudentWorld()->getSearch()->setMovable(x, y + i);
	
	#include <iostream>
	getStudentWorld()->getSearch()->printMovable(); ///DEBUGGING
}

//GoldNugget functions
GoldNugget::GoldNugget(int x, int y, bool isVisible, bool isPlayerPickable, bool isPermanent, StudentWorld *sw) : Actor(IID_GOLD, x, y, right, 1.0, 2, sw), ticks(100) { 
	setVisible(true); 
	if (isPlayerPickable) state = stable;
	else state = temporary;
}
int GoldNugget::doSomething() {
	if (!ticks) return SELF_DIED;
	//check if collided with frackman
	if (state == stable) {
		return getStudentWorld()->goldNuggetCollisions(this, true); //check if collided with frackman
	}
	else if (state == temporary) {
		ticks--; //count down ticks
		return getStudentWorld()->goldNuggetCollisions(this, false); //check if collided with a protester
	}
	return CONTINUE;
}
///DEBUGGING
//nuggets visibility depends on isVisible
//barrel visibility always false

//OilBarrel functions
OilBarrel::OilBarrel(int x, int y, StudentWorld *sw) : Actor(IID_BARREL, x, y, right, 1.0, 2, sw) { setVisible(true); }
int OilBarrel::doSomething() {
	//if it collides with frackman then tell the level that he struck oil
	return getStudentWorld()->oilBarrelCollisions(this); //return appropriate value depending on whether it collided and found all oil
}

//Squirt functions
Squirt::Squirt(int x, int y, Direction dir, StudentWorld *sw) : Actor(IID_WATER_SPURT, x, y, dir, 1.0, 2, sw), ticks(5) { setVisible(true); }
int Squirt::doSomething() {
	if (!ticks) return SELF_DIED;
	
	//go through all protester collisions before deciding whether to kill the squirt
	Direction dir = getDirection();
	int x = getX(), y = getY();
	switch (dir) {
	case up: moveTo(x, y + 1); break;
	case down: moveTo(x, y - 1); break;
	case right: moveTo(x + 1, y); break;
	case left: moveTo(x - 1, y); break;
	default:;
	}

	//check dirt/edge collisions
	if (getX() < 0 || getY() < 0 || getX() > 60 || getY() > 60) return SELF_DIED;
	for (int i = getX(); i < getX() + 4; i++)
		for (int j = getY(); j < getY() + 4; j++)
			if (getStudentWorld()->isDirt(i, j)) return SELF_DIED;
	//check boulder/other collisions
	if (getStudentWorld()->squirtCollisions(this) == SELF_DIED) return SELF_DIED;

	ticks--;
	return CONTINUE;
}

//Goodie functions
Goodie::Goodie(int ID, int x, int y, int t, StudentWorld *sw) : Actor(ID, x, y, right, 1.0, 2, sw), ticks(t) { setVisible(true); }
int Goodie::doSomething() {
	if (!ticks) return SELF_DIED;
	//if frackman picks up goodie
	if (getStudentWorld()->goodieCollisions(this) == SELF_DIED) return SELF_DIED;
	ticks--;
	return CONTINUE;
}

//WaterPool functions
WaterPool::WaterPool(int x, int y, int t, StudentWorld *sw) : Goodie(IID_WATER_POOL, x, y, t, sw) {}

//SonarKit functions
SonarKit::SonarKit(int x, int y, int t, StudentWorld *sw) : Goodie(IID_SONAR, x, y, t, sw) {}


//Protester functions
Protester::Protester(int ID, int x, int y, int h, StudentWorld *sw) : Actor(ID, x, y, left, 1.0, 0, sw), state(ready), health(h), ticks(15) { setVisible(true); }
inline Protester::~Protester() {}
bool Protester::setGiveUp() { state = giveup; return true; }
bool Protester::setAnnoyed(int t) { 
	if (state == giveup) return false; 
	state = resting; ticks = t; 
	return true;
}
void Protester::decHealth(int h) { health -= h; }
int Protester::doSomething() {
	///DEBUGGING
	///HO LE FUC this isn't anywhere near what I'm supposed to do.
	if (!ticks) {
		getStudentWorld()->playSound(SOUND_PROTESTER_GIVE_UP);
		state = ready;
		ticks = 15;
	}
	//when protester hits frackman check if it killed him, return PLAYER_DIED from here
	if(state == annoyed) ticks--;
	else if (state == ready) {}
	else if (state == giveup) {}
	return CONTINUE;
}

//Regular Protester functions
RegularProtester::RegularProtester(int x, int y, StudentWorld *sw) : Protester(IID_PROTESTER, x, y, 5, sw) {}
bool RegularProtester::setGiveUp() { getStudentWorld()->increaseScore(100); return Protester::setGiveUp(); }
int RegularProtester::doSomething() {
	int ret = Protester::doSomething();
	return ret;
}

//HardcoreProtester functions
HardcoreProtester::HardcoreProtester(int x, int y, StudentWorld *sw) : Protester(IID_HARD_CORE_PROTESTER, x, y, 20, sw) {}
bool HardcoreProtester::setGiveUp() { getStudentWorld()->increaseScore(250); return Protester::setGiveUp(); }
int HardcoreProtester::doSomething() {
	int ret = Protester::doSomething();
	return ret;
}

//FrackMan functions
FrackMan::FrackMan(StudentWorld *sw) : GraphObject(IID_PLAYER, 30, 60, right, 1.0, 0), health(10), water(5), sonar(1), gold(0), sWorld(sw) {
	setVisible(true);
}
int FrackMan::doSomething() {
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
	if (removedDirt) {
		sWorld->playSound(SOUND_DIG);
		sWorld->getSearch()->setMovable(getX(), getY());
	}
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
		default: moveTo(originalX, originalY); break;
		}

		//check if collided with edges
		if (getX() < 0 || getY() < 0 || getX() > VIEW_WIDTH - 4 * getSize() || getY() > VIEW_HEIGHT - 4 * getSize())
			for (int i = 0; i < 4; i++) moveTo(originalX, originalY); //run moveTo multiple times to run through animation
		//check collisions with boulders and makes some objects visible if they are close enough
		sWorld->frackmanCollisions(this, originalX, originalY);
	}
	if(health > 0) return Actor::CONTINUE;
	else return Actor::PLAYER_DIED;
}