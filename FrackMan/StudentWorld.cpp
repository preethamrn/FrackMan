///DEBUGGING///
//YOU'RE IN HAX NOW
//DON'T ADD IMPORTANT FUNCTIONALITY
///DEBUGGING///


#include "StudentWorld.h"
#include "Actor.h"
#include "GraphObject.h"
#include <string>
#include <cmath>

GameWorld* createStudentWorld(std::string assetDir) {
	return new StudentWorld(assetDir);
}

int StudentWorld::init() {
	search = new BFSSearch(this); //creating new search

	//initializing dirt and open locations in search
	for (int i = 0; i < VIEW_WIDTH; i++) {
		for (int j = 0; j < VIEW_HEIGHT; j++) {
			if (j >= 60) dirt[i][j] = nullptr; //top layer
			else if (j >= 4 && i >= 30 && i <= 33) dirt[i][j] = nullptr; //mineshaft
			else dirt[i][j] = new Dirt(i, j, this);
		}
	}
	//initializing barrels of oil, boulders, and gold nuggets
	int nBoulders = min(getLevel() / 2 + 2, 6);
	int nBarrels = min(2 + getLevel(), 20);
	int nNuggets = max(5 - getLevel() / 2, 2);
	for (int i = 0; i < nBoulders; i++) addInitialActor(Actor::BOULDER);
	for (int i = 0; i < nBarrels; i++) addInitialActor(Actor::OILBARREL);
	for (int i = 0; i < nNuggets; i++) addInitialActor(Actor::GOLDNUGGET);

	ticks = max(25, 200 - getLevel()); //initial ticks (to start off with a protestor on first tick)
	nOilBarrels = nBarrels; //set number of barrels
	nProtesters = 0; //set initial number of protesters
	frackman = new FrackMan(this); //create new FrackMan

	return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
	//display stat text at top of screen
	setGameStatText("Scr: " + getScoreText() + 
		"  Lvl: " + getLevelText() + 
		"  Lives: " + getLivesText() + 
		"  Hlth: " + getHealthText() + 
		"  Wtr: " + getWaterText() +  ///DEBUGGING? Wtr or Water?
		"  Gld: " + getGoldText() + 
		"  Sonar: " + getSonarText() + 
		"  Oil Left: " + getOilLeftText());
	//setGameStatText(std::to_string(ticks)); ///DEBUGGING
	int ret = frackman->doSomething();
	if (ret == Actor::PLAYER_DIED) {
		decLives();
		return GWSTATUS_PLAYER_DIED;
	} else if (ret == Actor::LEVEL_SUCCESS) {
		return GWSTATUS_FINISHED_LEVEL;
	}

	for (unsigned int i = 0; i < actors.size(); i++) {
		int ret = actors[i]->doSomething(); //store value returned by actor after doing something
		if (ret == Actor::PLAYER_DIED) { //actor makes player give up
			return playerDied();
		} else if (ret == Actor::SELF_DIED) { //actor gets destroyed
			if (actors[i]->getType() == Actor::DEADPROTESTER) nProtesters--;
			delete actors[i];
			actors.erase(actors.begin() + i);
			i--;
		} else if (ret == Actor::LEVEL_SUCCESS) { //level success (ie. if collected all barrels)
			playSound(SOUND_FINISHED_LEVEL);
			return GWSTATUS_FINISHED_LEVEL;
		}
	}

	//checking to add protester
	if (ticks >= max(25, 200 - getLevel())) {
		if (nProtesters < min(15, 2 + getLevel()*1.5)) {
			int prob = min(90, getLevel() * 10 + 30);
			if (rand() % 100 < prob) actors.push_back(new HardcoreProtester(60, 60, max(0, 3 - getLevel() / 4), 16 + 2*getLevel(), this));
			else actors.push_back(new RegularProtester(60, 60, max(0, 3 - getLevel() / 4), this));
			nProtesters++;
			ticks = 0;
		}
	}
	//checking to add waterpool or sonar kit
	if ((rand() % (getLevel() * 25 + 300)) == 0) {
		if ((rand() % 5) == 0) actors.push_back(new SonarKit(0, 60, max(100, 300 - 10 * getLevel()), this)); //adding sonar kit
		else {
			//finding position for waterpool
			int x, y;
			do {
				x = rand() % 61;
				y = rand() % 57; ///DEBUGGING? position of waterpool on surface possible?
				bool flag = false;
				for (int i = x; i < x + 4; i++) {
					for (int j = y; j < y + 4; j++) if (dirt[i][j] != nullptr) {
						flag = true;
						break;
					}
					if (flag) break;
				}
				if (!flag) break;
			} while (true);
			actors.push_back(new WaterPool(x, y, max(100, 300 - 10 * getLevel()), this)); //adding waterpool
		}
	}
	ticks++;
	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp() {
	//deleting dirt
	for (int i = 0; i < VIEW_WIDTH; i++) {
		for (int j = 0; j < VIEW_HEIGHT; j++) {
			delete dirt[i][j];
		}
	}
	//deleting all actors
	while (!actors.empty()) {
		delete actors[0];
		actors.erase(actors.begin());
	}
	//deleting frackman
	delete frackman;

	//deleting search
	delete search; 
}

void StudentWorld::useSonar() {
	playSound(SOUND_SONAR);
	for (unsigned int i = 0; i < actors.size(); i++) {
		if (collides(frackman, actors[i], 12.0)) {
			actors[i]->setVisible(true);
		}
	}
}

void StudentWorld::useWater() {
	playSound(SOUND_PLAYER_SQUIRT);
	GraphObject::Direction dir = frackman->getDirection();
	int x = frackman->getX(), y = frackman->getY();
	switch (dir) {
	case GraphObject::up: y += 3; break;
	case GraphObject::down: y -= 3; break;
	case GraphObject::right: x += 3; break;
	case GraphObject::left: x -= 3; break;
	default:;
	}
	actors.push_back(new Squirt(x, y, dir, this));
}

void StudentWorld::useGold() {
	int x = frackman->getX(), y = frackman->getY();
	actors.push_back(new GoldNugget(x, y, true, false, false, this));
}

//constant time collision detection for two objects within (<=) radius blocks other each other
//return true when one graph object is inside the radius wrt another
bool StudentWorld::collides(GraphObject *ob1, GraphObject *ob2, double radius) const {
	if (ob1 == ob2) return false; //check for aliasing. In this universe I don't collide with myself. Deal with it. (because boulder would always break immediately otherwise)
	double dx = ob1->getX() - ob2->getX(), dy = ob1->getY() - ob2->getY();
	return (dx*dx + dy*dy <= radius*radius);
}

//function called by a BFSSearch to update its movable positions. Processed by StudentWorld to get positions of dirt and boulders
void StudentWorld::updateMovable(bool movable[][64]) {
	for (int x = 0; x <= 60; x++) {
		for (int y = 0; y <= 60; y++) {
			bool open = true;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (dirt[x + i][y + j] != nullptr) { open = false; break; }
				}
				if (!open) break;
			}
			movable[x][y] = open;
		}
	}
	for (int i = 0; i < actors.size(); i++) {
		if (actors[i]->getType() == Actor::BOULDER) {
			for (int x = -4; x <= 4; x++) {
				for (int y = -4; y <= 4; y++) {
					if (x*x + y*y <= 9.0) movable[actors[i]->getX() + x][actors[i]->getY() + y] = false; //position is within boulder's hitbox
				}
			}
		}
	}
}

//when falling, check if it hits a player (PLAYER_DIED), protester (set protester to annoyed), dirt/another boulder (SELF_DIED)
int StudentWorld::boulderCollisions(Boulder* b) {
	if (collides(b, frackman, 3.0)) return Actor::PLAYER_DIED;
	for (unsigned int i = 0; i < actors.size(); i++) {
		if (collides(b, actors[i], 3.0)) {
			if (actors[i]->getType() == Actor::BOULDER) return Actor::SELF_DIED; //hit a boulder
			else if (actors[i]->getType() == Actor::REGPROTESTER || actors[i]->getType() == Actor::HCOREPROTESTER) {
				//hit a protester
				playSound(SOUND_PROTESTER_GIVE_UP);
				if (actors[i]->getType() == Actor::REGPROTESTER) increaseScore(100);
				else if (actors[i]->getType() == Actor::HCOREPROTESTER) increaseScore(250);

				int x = actors[i]->getX(), y = actors[i]->getY();
				actors.push_back(new DeadProtester(actors[i]->getID(), x, y, max(0, 3 - getLevel() / 4), this)); //add a dead protester to the list of actors
				dynamic_cast<Protester*>(actors[i])->setDead(); //set this protester who has given up to dead
				increaseScore(500);
			}
		}
	}
	return Actor::CONTINUE;
}

int StudentWorld::goldNuggetCollisions(GoldNugget *gn, bool isPlayerPickable) {
	if(isPlayerPickable) {
		if (collides(gn, frackman, 3.0)) {
			playSound(SOUND_GOT_GOODIE); //play sound
			frackman->setGold(frackman->getGold() + 1); //give frackman gold
			increaseScore(10); //increase score
			return Actor::SELF_DIED;
		}
	} else {
		//check if collided with a protester
		for (unsigned int i = 0; i < actors.size(); i++) {
			if (collides(gn, actors[i], 3.0)) {
				if (actors[i]->getType() == Actor::REGPROTESTER) {
					playSound(SOUND_PROTESTER_FOUND_GOLD);
					int x = actors[i]->getX(), y = actors[i]->getY();
					actors.push_back(new DeadProtester(actors[i]->getID(), x, y, max(0, 3 - getLevel() / 4), this)); //add a dead protester to the list of actors
					dynamic_cast<Protester*>(actors[i])->setDead(); //set this protester who has given up to dead
					increaseScore(25);
					return Actor::SELF_DIED;
				} else if (actors[i]->getType() == Actor::HCOREPROTESTER) {
					playSound(SOUND_PROTESTER_FOUND_GOLD);
					dynamic_cast<HardcoreProtester*>(actors[i])->setResting(max(50, 100-getLevel()*10));
					increaseScore(50);
					return Actor::SELF_DIED;
				}
			}
		}
	}
	return Actor::CONTINUE;
}

int StudentWorld::oilBarrelCollisions(OilBarrel *ob) {
	if (collides(ob, frackman, 3.0)) {
		if (struckOil()) return Actor::LEVEL_SUCCESS; //striking oil wins the level
		return Actor::SELF_DIED; //if the level hasn't been won, then kill this object
	}
	return Actor::CONTINUE;
}

int StudentWorld::squirtCollisions(Squirt *s) {
	//check boulder/other collisions
	bool dead = false;
	for (unsigned int i = 0; i < actors.size(); i++) {
		if (collides(s, actors[i], 3.0)) {
			if (actors[i]->getType() == Actor::BOULDER) return Actor::SELF_DIED;
			if (actors[i]->getType() == Actor::REGPROTESTER || actors[i]->getType() == Actor::HCOREPROTESTER) {
				Protester *p = dynamic_cast<Protester*>(actors[i]);
				dead = true;
				if (p->decHealth(2)) { //if this protester has died
					playSound(SOUND_PROTESTER_GIVE_UP);
					if (actors[i]->getType() == Actor::REGPROTESTER) increaseScore(100);
					else if (actors[i]->getType() == Actor::HCOREPROTESTER) increaseScore(250);
					
					int x = actors[i]->getX(), y = actors[i]->getY();
					actors.push_back(new DeadProtester(actors[i]->getID(), x, y, max(0, 3 - getLevel() / 4), this)); //add a dead protester to the list of actors
					p->setDead(); //set this protester who has given up to dead
				} else {
					playSound(SOUND_PROTESTER_ANNOYED);
					p->setResting(max(50, 100 - getLevel() * 10));
				}
			}
		}
	}
	if (dead) return Actor::SELF_DIED;
	else return Actor::CONTINUE;
}

int StudentWorld::goodieCollisions(Goodie *g) {
	if (collides(g, frackman, 3.0)) {
		playSound(SOUND_GOT_GOODIE);
		//picked up waterpool
		if (g->getType() == Actor::WATERPOOL) {
			frackman->setWater(frackman->getWater() + 5); //give frackman water
			increaseScore(100); //increase score
		}
		//picked up sonar kit
		else if (g->getType() == Actor::SONARKIT) {
			frackman->setSonar(frackman->getSonar() + 2); //give frackman sonar charges
			increaseScore(75); //increase score
		}
		return Actor::SELF_DIED;
	}
	return Actor::CONTINUE;
}


void StudentWorld::frackmanCollisions(FrackMan *f, int ox, int oy) {
	for (std::vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
		//check boulders first
		if ((*it)->getType() == Actor::BOULDER) {
			if (collides(*it, f, 3.0)) {
				//move back to original position if it hits a boulder
				for (int i = 0; i < 3; i++) f->moveTo(ox, oy); //run moveTo multiple times to run through animation
				break;
			}
		}
		//check if any objects should be made visible
		else if (collides(*it, f, 4.0)) {
			Actor *actor = *it;
			if (actor->getType() == Actor::OILBARREL) { actor->setVisible(true); }
			else if (actor->getType() == Actor::GOLDNUGGET) { actor->setVisible(true); }
		}
	}
}

void StudentWorld::addInitialActor(Actor::Type actorType) {
	int x, y;
	double dist;
	//find a good x, y
	do {
		dist = 1000.0;
		x = rand() % 61;
		y = (rand() % 37) + 20;
		for (std::vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
			Actor *a = *it;
			int dx = x - a->getX();
			int dy = y - a->getY();
			dist = fmin(dx*dx + dy*dy, dist);
		}
	} while (dist <= 36.0 || y >= 4 && x >= 27 && x <= 33); //check if not far enough from other objects or inside mineshaft 
														   ///DEBUGGING (boulders spawn inside minshaft too?) - this doesn't

	//create appropriate actor
	Actor *actor;
	if (actorType == Actor::BOULDER) {
		actor = new Boulder(x, y, this);
		for (int i = x; i < x + 4; i++) {
			for (int j = y; j < y + 4; j++) {
				delete dirt[i][j];
				dirt[i][j] = nullptr;
			}
		}
	}
	else if (actorType == Actor::OILBARREL) actor = new OilBarrel(x, y, this);
	else if (actorType == Actor::GOLDNUGGET) actor = new GoldNugget(x, y, false, true, true, this);
	actors.push_back(actor); //add actor to list of actors
}