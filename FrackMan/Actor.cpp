#include "Actor.h"
#include "StudentWorld.h"


//Actor functions
Actor::Actor(int ID, int x, int y, Direction dir, float size, int depth, StudentWorld *sw) : GraphObject(ID, x, y, dir, size, depth), sWorld(sw), m_alive(true) {}
inline Actor::~Actor() {}
bool Actor::alive() { return m_alive; }
void Actor::setDead() { m_alive = false; }

//Dirt functions
Dirt::Dirt(int x, int y, StudentWorld *sw) : Actor(IID_DIRT, x, y, right, 0.25, 3, sw) { setVisible(true); }


//Boulder functions
Boulder::Boulder(int x, int y, StudentWorld *sw) : Actor(IID_BOULDER, x, y, down, 1.0, 1, sw), state(stable), ticks(30) { setVisible(true); }
void Boulder::doSomething() {
	if (!alive()) return;
	else if (state == stable) {
		state = waiting;
		for (int i = 0; i < 4; i++) {
			if (sWorld->isDirt(getX() + i, getY() - 1)) state = stable;
		} //state becomes waiting if there's no dirt underneath rock
	}
	//check if waiting
	else if (state == waiting) {
		ticks--;
		//if all waiting ticks have elapsed, start falling
		if (!ticks) {
			sWorld->playSound(SOUND_FALLING_ROCK);
			state = falling;
			ticks = 30;
		}
	}
	else if (state == falling) {
		moveTo(getX(), getY() - 1);
		if (getY() < 0) setDead(); //hit the bottom
		for (int i = 0; i < 4; i++) {
			if (sWorld->isDirt(getX() + i, getY())) setDead(); //hit dirt
		}
		//when falling, check if it hits a player (PLAYER_DIED), protester (set protester to annoyed), dirt/another boulder (SELF_DIED)
		if (sWorld->collides(this, sWorld->getFrackMan(), 3.0)) {setDead(); return;} //return PLAYER_DIED; ///DEBUGGING
		std::vector<Actor*> *actors = sWorld->getActors();
		for (int i = 0; i < actors->size(); i++) {
			if (sWorld->collides(this, (*actors)[i], 3.0)) {
				if (dynamic_cast<Boulder*>((*actors)[i])) setDead(); //hit a boulder
				else if (dynamic_cast<Protester*>((*actors)[i])) {
					dynamic_cast<Protester*>((*actors)[i])->setGiveUp(); //hit a protester
					sWorld->increaseScore(500);
				}
			}
		}
	}
}

//GoldNugget functions
GoldNugget::GoldNugget(int x, int y, bool isVisible, bool isPlayerPickable, bool isPermanent, StudentWorld *sw) : Actor(IID_GOLD, x, y, right, 1.0, 2, sw), ticks(100) { 
	setVisible(true); 
	if (isPlayerPickable) state = stable;
	else state = temporary;
}
void GoldNugget::doSomething() {
	if (!ticks) return;
	//check if collided with frackman
	if (state == stable) {
		FrackMan *f = sWorld->getFrackMan();
		if (sWorld->collides(this, f, 3.0)) {
			sWorld->playSound(SOUND_GOT_GOODIE);
			f->setGold(f->getGold() + 1);
			sWorld->increaseScore(10);
			setDead();
		}
	}
	//check if collided with a protester
	else if (state == temporary) {
		std::vector<Actor*> *actors = sWorld->getActors();
		for (int i = 0; i < actors->size(); i++) {
			if (sWorld->collides(this, (*actors)[i], 3.0)) {
				Protester *p = dynamic_cast<Protester*>((*actors)[i]);
				if (p != nullptr) {
					sWorld->playSound(SOUND_PROTESTER_FOUND_GOLD);
					if (dynamic_cast<RegularProtester*>(p)) {
						p->setGiveUp();
						sWorld->increaseScore(25);
					} else if (dynamic_cast<HardcoreProtester*>(p)) { 
						p->setAnnoyed(15); ///DEBUGGING. should be setStaring or something...
						sWorld->increaseScore(50);
					}
					setDead();
				}
			}
		}
		ticks--;
	}
}
///DEBUGGING
//nuggets visibility depends on isVisible
//barrel visibility always false

//OilBarrel functions
OilBarrel::OilBarrel(int x, int y, StudentWorld *sw) : Actor(IID_BARREL, x, y, right, 1.0, 2, sw) { setVisible(true); }
void OilBarrel::doSomething() {
	//if it collides with frackman then tell the level that he struck oil
	if (sWorld->collides(this, sWorld->getFrackMan(), 3.0)) {
		if (sWorld->struckOil()) setDead(); //return LEVEL_SUCCESS; //striking oil wins the level
		return setDead(); //if the level hasn't been won, then kill this object
	}
}

//Squirt functions
Squirt::Squirt(int x, int y, Direction dir, StudentWorld *sw) : Actor(IID_WATER_SPURT, x, y, dir, 1.0, 2, sw), ticks(5) { setVisible(true); }
void Squirt::doSomething() {
	//check boulder/other collisions
	std::vector<Actor*> *actors = sWorld->getActors();
	bool dead = false;
	for (int i = 0; i < actors->size(); i++) {
		Actor *actor = (*actors)[i];
		if (sWorld->collides(this, actor, 3.0)) {
			if (dynamic_cast<Boulder*>(actor)) setDead();
			Protester *p = dynamic_cast<Protester*>(actor);
			if (p != nullptr) {
				if (p->setAnnoyed(15)) { ///DEBUGGING
					dead = true;
					p->decHealth(2);
				}
			}
		}
	}
	if (dead) setDead();

	ticks--;
	if (!ticks) {
		setDead();
		return;
	}
	//check dirt/edge collisions
	if (getX() < 0 || getY() < 0 || getX() > 60 || getY() > 60) setDead();
	else for (int i = getX(); i < getX() + 4; i++)
		for (int j = getY(); j < getY() + 4; j++)
			if (sWorld->isDirt(i, j)) setDead();
	//move to next square
	Direction dir = getDirection();
	int x = getX(), y = getY();
	switch (dir) {
	case up: moveTo(x, y + 1); break;
	case down: moveTo(x, y - 1); break;
	case right: moveTo(x + 1, y); break;
	case left: moveTo(x - 1, y); break;
	default:;
	}
}

//Goodie functions
Goodie::Goodie(int ID, int x, int y, int t, StudentWorld *sw) : Actor(ID, x, y, right, 1.0, 2, sw), ticks(t) { setVisible(true); }
void Goodie::doSomething() {
	if (!ticks) return;
	//if frackman picks up goodie
	if (sWorld->collides(this, sWorld->getFrackMan(), 3.0)) {
		sWorld->playSound(SOUND_GOT_GOODIE);
		setDead();
	}
	ticks--;
}

//WaterPool functions
WaterPool::WaterPool(int x, int y, int t, StudentWorld *sw) : Goodie(IID_WATER_POOL, x, y, t, sw) {}
void WaterPool::doSomething() {
	if (!alive()) {
		//give frackman waterpool and increase score
		FrackMan *f = sWorld->getFrackMan();
		f->setWater(f->getWater() + 5); 
		sWorld->increaseScore(100);
	}
}

//SonarKit functions
SonarKit::SonarKit(int x, int y, int t, StudentWorld *sw) : Goodie(IID_SONAR, x, y, t, sw) {}
void SonarKit::doSomething() {
	if () {
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
void Protester::doSomething() {
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
void RegularProtester::doSomething() {
	int ret = Protester::doSomething();
	return ret;
}

//HardcoreProtester functions
HardcoreProtester::HardcoreProtester(int x, int y, StudentWorld *sw) : Protester(IID_HARD_CORE_PROTESTER, x, y, 25, sw) {}
bool HardcoreProtester::setGiveUp() { sWorld->increaseScore(250); return Protester::setGiveUp(); }
void HardcoreProtester::doSomething() {
	int ret = Protester::doSomething();
	return ret;
}

//FrackMan functions
FrackMan::FrackMan(StudentWorld *sw) : GraphObject(IID_PLAYER, 30, 60, right, 1.0, 0), health(10), water(5), sonar(1), gold(0), sWorld(sw) {
	setVisible(true);
}
void FrackMan::doSomething() {
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
		
		//sWorld->checkFrackManCollisions(this);
		std::vector<Actor*> *actors = sWorld->getActors();
		for (std::vector<Actor*>::const_iterator it = actors->begin(); it != actors->end(); it++) {
			//check boulders first
			if (dynamic_cast<Boulder*>(*it)) {
				if (sWorld->collides(*it, this, 3.0)) {
					moveTo(originalX, originalY);
					break;
				}
			}
			//check if any objects should be made visible
			else if (sWorld->collides(*it, this, 4.0)) {
				Actor *actor = *it;
				if (dynamic_cast<OilBarrel*>(actor)) { actor->setVisible(true); }
				else if (dynamic_cast<GoldNugget*>(actor)) { actor->setVisible(true); }
			}
		}
	}
	if(health > 0) return Actor::CONTINUE;
	else return Actor::PLAYER_DIED;
}