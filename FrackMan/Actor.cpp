#include "Actor.h"
#include "StudentWorld.h"


//Actor functions
Actor::Actor(int ID, int x, int y, Direction dir, float size, int depth, StudentWorld *sw) : GraphObject(ID, x, y, dir, size, depth), sWorld(sw) {}
inline Actor::~Actor() {}

//Dirt functions
Dirt::Dirt(int x, int y, StudentWorld *sw) : Actor(IID_DIRT, x, y, right, 0.25, 3, sw) { setVisible(true); }


//Boulder functions
Boulder::Boulder(int x, int y, StudentWorld *sw) : Actor(IID_BOULDER, x, y, down, 1.0, 1, sw), state(stable), ticks(30) { setVisible(true); }
int Boulder::doSomething() {
	//if all waiting ticks have elapsed, start falling
	if (!ticks) {
		sWorld->playSound(SOUND_FALLING_ROCK);
		state = falling;
		ticks = 30;
	}
	//check if waiting
	if (state == waiting) ticks--;
	else if (state == stable) {
		state = waiting;
		for (int i = 0; i < 4; i++) {
			if (sWorld->isDirt(getX() + i, getY() - 1)) state = stable;
		} //state becomes waiting if there's no dirt underneath rock
	}
	if (state == falling) {
		moveTo(getX(), getY() - 1);
		if (getY() < 0) return SELF_DIED; //hit the bottom
		for (int i = 0; i < 4; i++) {
			if (sWorld->isDirt(getX() + i, getY())) return SELF_DIED; //hit dirt
		}
		return sWorld->boulderCollisions(this);
	}
	return CONTINUE;
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
		FrackMan *f = sWorld->getFrackMan();
		if (sWorld->collides(this, f, 3.0)) {
			sWorld->playSound(SOUND_GOT_GOODIE);
			f->setGold(f->getGold() + 1);
			sWorld->increaseScore(10);
			return SELF_DIED;
		}
	}
	else if (state == temporary) {
		ticks--; //count down ticks
		return sWorld->goldNuggetCollisions(this); //check if collided with a protester
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
	if (sWorld->collides(this, sWorld->getFrackMan(), 3.0)) {
		if(sWorld->struckOil()) return LEVEL_SUCCESS; //striking oil wins the level
		return SELF_DIED; //if the level hasn't been won, then kill this object
	}
	return CONTINUE;
}

//Squirt functions
Squirt::Squirt(int x, int y, Direction dir, StudentWorld *sw) : Actor(IID_WATER_SPURT, x, y, dir, 1.0, 2, sw), ticks(5) { setVisible(true); }
int Squirt::doSomething() {
	if (!ticks) return SELF_DIED;

	//check dirt/edge collisions
	if (getX() < 0 || getY() < 0 || getX() > 60 || getY() > 60) return SELF_DIED;
	for (int i = getX(); i < getX() + 4; i++)
		for (int j = getY(); j < getY() + 4; j++)
			if (sWorld->isDirt(i, j)) return SELF_DIED;
	//check boulder/other collisions
	if (sWorld->squirtCollisions(this) == SELF_DIED) return SELF_DIED;
	
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
	ticks--;
	return CONTINUE;
}

//Goodie functions
Goodie::Goodie(int ID, int x, int y, int t, StudentWorld *sw) : Actor(ID, x, y, right, 1.0, 2, sw), ticks(t) { setVisible(true); }
int Goodie::doSomething() {
	if (!ticks) return SELF_DIED;
	//if frackman picks up goodie
	if (sWorld->collides(this, sWorld->getFrackMan(), 3.0)) {
		sWorld->playSound(SOUND_GOT_GOODIE);
		return PICKED_UP;
	}
	ticks--;
	return CONTINUE;
}

//WaterPool functions
WaterPool::WaterPool(int x, int y, int t, StudentWorld *sw) : Goodie(IID_WATER_POOL, x, y, t, sw) {}
int WaterPool::doSomething() {
	int ret = Goodie::doSomething();
	if (ret == PICKED_UP) {
		//give frackman waterpool and increase score
		FrackMan *f = sWorld->getFrackMan();
		f->setWater(f->getWater() + 5); 
		sWorld->increaseScore(100);
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
		FrackMan *f = sWorld->getFrackMan();
		f->setSonar(f->getSonar() + 2);
		sWorld->increaseScore(75);
		return SELF_DIED;
	}
	return ret;
}

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
		sWorld->playSound(SOUND_PROTESTER_GIVE_UP);
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
RegularProtester::RegularProtester(int x, int y, StudentWorld *sw) : Protester(IID_PROTESTER, x, y, 10, sw) {}
bool RegularProtester::setGiveUp() { sWorld->increaseScore(100); return Protester::setGiveUp(); }
int RegularProtester::doSomething() {
	int ret = Protester::doSomething();
	return ret;
}

//HardcoreProtester functions
HardcoreProtester::HardcoreProtester(int x, int y, StudentWorld *sw) : Protester(IID_HARD_CORE_PROTESTER, x, y, 25, sw) {}
bool HardcoreProtester::setGiveUp() { sWorld->increaseScore(250); return Protester::setGiveUp(); }
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
	if (removedDirt) sWorld->playSound(SOUND_DIG);

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
		//check collisions with boulders and makes some objects visible if they are close enough
		sWorld->frackmanCollisions(this, originalX, originalY);
	}
	if(health > 0) return Actor::CONTINUE;
	else return Actor::PLAYER_DIED;
}