#include "StudentWorld.h"
#include <string>
#include <cmath>

GameWorld* createStudentWorld(std::string assetDir) {
	return new StudentWorld(assetDir);
}

int StudentWorld::init() {
	//initializing dirt
	for (int i = 0; i < VIEW_WIDTH; i++) {
		for (int j = 0; j < VIEW_HEIGHT; j++) {
			if (j >= 60) dirt[i][j] = nullptr; //top layer
			else if (j >= 4 && i >= 30 && i <= 33) dirt[i][j] = nullptr; //mineshaft
			else dirt[i][j] = new Dirt(i, j, this);
		}
	}
	//initializing barrels of oil, boulders, and gold nuggets
	int nBoulders = (int)fmin(getLevel() / 2 + 2, 6);
	int nBarrels = (int)fmin(2 + getLevel(), 20);
	int nNuggets = (int)fmax(5 - getLevel() / 2, 2);
	for (int i = 0; i < nBoulders; i++) addInitialActor(boulder);
	for (int i = 0; i < nBarrels; i++) addInitialActor(oilBarrel);
	for (int i = 0; i < nNuggets; i++) addInitialActor(goldNugget);

	ticks = (int)fmin(25, 200 - getLevel()); //initial ticks (to start off with a protestor on first tick
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
		"  Wtr: " + getWaterText() + 
		"  Gld: " + getGoldText() + 
		"  Sonar: " + getSonarText() + 
		"  Oil Left: " + getOilLeftText());
	//setGameStatText(std::to_string(ticks)); ///DEBUGGING
	
	if (frackman->doSomething() == Actor::PLAYER_DIED) {
		return playerDied();
	}

	for (unsigned int i = 0; i < actors.size(); i++) {
		int ret = actors[i]->doSomething(); //returned by actor after doing something
		if (ret == Actor::PLAYER_DIED) { //actor makes player give up
			return playerDied();
		} else if (ret == Actor::SELF_DIED) { //actor gets destroyed
			if (dynamic_cast<Protester*>(actors[i])) nProtesters--;
			delete actors[i];
			actors.erase(actors.begin() + i);
			i--;
		} else if (ret == Actor::LEVEL_SUCCESS) { //level success (ie. if collected all barrels)
			playSound(SOUND_FINISHED_LEVEL);
			return GWSTATUS_FINISHED_LEVEL;
		}
	}

	//checking to add protester
	if (ticks >= (int)fmin(25, 200 - getLevel())) {
		if (nProtesters < (int)fmin(15, 2 + getLevel()*1.5)) {
			if ((rand() % (int)fmin(90, getLevel() * 10 + 30)) == 0) actors.push_back(new HardcoreProtester(60, 60, this));
			else actors.push_back(new RegularProtester(60, 60, this));
			nProtesters++;
			ticks = 0;
		}
	}
	//checking to add waterpool or sonar kit
	if ((rand() % (getLevel() * 25 + 300)) == 0) {
		if ((rand() % 5) == 0) actors.push_back(new SonarKit(0, 60, fmin(100, 300 - 10 * getLevel()), this)); //adding sonar kit
		else {
			//finding position for waterpool
			int x, y;
			do {
				x = rand() % 61;
				y = rand() % 57;
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
			actors.push_back(new WaterPool(x, y, fmin(100, 300 - 10 * getLevel()), this)); //adding waterpool
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
	for (int i = actors.size() - 1; i >= 0; i--) {
		delete actors[i];
		actors.erase(actors.begin() + i);
	}
	//deleting frackman
	delete frackman;
}

void StudentWorld::useSonar() {
	playSound(SOUND_SONAR);
	for (int i = 0; i < actors.size(); i++) {
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
bool StudentWorld::collides(GraphObject *ob1, GraphObject *ob2, double radius) {
	/*int d1 = 4 * ob1->getSize(), d2 = 4 * ob2->getSize();
	if (ob1->getX() + d1 > ob2->getX() && ob1->getX() <= ob2->getX() || ob2->getX() + d2 > ob1->getX() && ob2->getX() <= ob1->getX())
		if (ob1->getY() + d1 > ob2->getY() && ob1->getY() <= ob2->getY() || ob2->getY() + d2 > ob1->getY() && ob2->getY() <= ob1->getY())
			return true;
	return false;*/ ///DEBUGGING? is it fixed?
	if (ob1 == ob2) return false; //check for aliasing. In my universe I don't collide with myself. Deal with it.
	double dx = ob1->getX() - ob2->getX(), dy = ob1->getY() - ob2->getY();
	return (sqrt(dx*dx + dy*dy)) <= radius;
}

//when falling, check if it hits a player (PLAYER_DIED), protester (set protester to annoyed), dirt/another boulder (SELF_DIED)
int StudentWorld::boulderCollisions(Boulder* b) {
	if (collides(b, frackman, 3.0)) return Actor::PLAYER_DIED;
	for (int i = 0; i < actors.size(); i++) {
		if (collides(b, actors[i], 3.0)) {
			if (dynamic_cast<Boulder*>(actors[i])) return Actor::SELF_DIED; //hit a boulder
			else if (dynamic_cast<Protester*>(actors[i])) {
				dynamic_cast<Protester*>(actors[i])->setGiveUp(); //hit a protester
				increaseScore(500);
			}
		}
	}
	return Actor::CONTINUE;
}

int StudentWorld::goldNuggetCollisions(GoldNugget *gn) {
	//check if collided with a protester
	for (int i = 0; i < actors.size(); i++) {
		if (collides(gn, actors[i], 3.0)) {
			Protester *p = dynamic_cast<Protester*>(actors[i]);
			if (p != nullptr) {
				playSound(SOUND_PROTESTER_FOUND_GOLD);
				if (dynamic_cast<RegularProtester*>(p)) {
					p->setGiveUp();
					increaseScore(25);
				}
				else if (dynamic_cast<HardcoreProtester*>(p)) {
					p->setAnnoyed(15); ///DEBUGGING. should be setStaring or something...
					increaseScore(50);
				}
				return Actor::SELF_DIED;
			}
		}
	}
	return Actor::CONTINUE;
}
int StudentWorld::squirtCollisions(Squirt *s) {
	//check boulder/other collisions
	bool dead = false;
	for (int i = 0; i < actors.size(); i++) {
		Actor *actor = actors[i];
		if (collides(s, actor, 3.0)) {
			if (dynamic_cast<Boulder*>(actor)) return Actor::SELF_DIED;
			Protester *p = dynamic_cast<Protester*>(actor);
			if (p != nullptr) {
				if (p->setAnnoyed(15)) { ///DEBUGGING
					dead = true;
					p->decHealth(2);
				}
			}
		}
	}
	if (dead) return Actor::SELF_DIED;
	else return Actor::CONTINUE;
}

void StudentWorld::frackmanCollisions(FrackMan *f, int ox, int oy) {
	for (std::vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
		//check boulders first
		if (dynamic_cast<Boulder*>(*it)) {
			if (collides(*it, f, 3.0)) {
				f->moveTo(ox, oy); //move back to original position if it hits a boulder
				break;
			}
		}
		//check if any objects should be made visible
		else if (collides(*it, f, 4.0)) {
			Actor *actor = *it;
			if (dynamic_cast<OilBarrel*>(actor)) { actor->setVisible(true); }
			else if (dynamic_cast<GoldNugget*>(actor)) { actor->setVisible(true); }
		}
	}
}

void StudentWorld::addInitialActor(ActorType actorType) {
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
			dist = fmin(sqrt(dx*dx + dy*dy), dist);
		}
	} while (dist <= 6.0 || y >= 4 && x >= 27 && x <= 33); //check if not far enough from other objects or inside mineshaft ///DEBUGGING (boulders spawn inside too?)

	//create appropriate actor
	Actor *actor;
	if (actorType == boulder) {
		actor = new Boulder(x, y, this);
		for (int i = x; i < x + 4; i++) {
			for (int j = y; j < y + 4; j++) {
				delete dirt[i][j];
				dirt[i][j] = nullptr;
			}
		}
	}
	else if (actorType == oilBarrel) actor = new OilBarrel(x, y, this);
	else if (actorType == goldNugget) actor = new GoldNugget(x, y, false, true, true, this);
	actors.push_back(actor); //add actor to list of actors
}
