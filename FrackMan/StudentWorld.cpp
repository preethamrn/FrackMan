#include "StudentWorld.h"
#include <string>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

// Students:  Add code to this file (if you wish), StudentWorld.h, Actor.h and Actor.cpp

int StudentWorld::init() {
	//initializing dirt
	for (int i = 0; i < VIEW_WIDTH; i++) {
		for (int j = 0; j < VIEW_HEIGHT; j++) {
			if (j >= 60) dirt[i][j] = nullptr; //top layer
			else if (j >= 4 && i >= 30 && i <= 33) dirt[i][j] = nullptr; //mineshaft
			else dirt[i][j] = new Dirt(i, j);
		}
	}
	//initializing barrels of oil, boulders, and gold nuggets
	int nBarrels = (2 + getLevel() < 20 ? 2 + getLevel() : 20);
	int nBoulders = (getLevel() / 2 + 2 < 6 ? getLevel() / 2 + 2 : 6);
	int nNuggets = (5 - getLevel() / 2 > 2 ? 5 - getLevel() / 2 : 2);
	for (int i = 0; i < nBarrels; i++) addInitialActor(oilBarrel);
	for (int i = 0; i < nBoulders; i++) addInitialActor(boulder);
	for (int i = 0; i < nNuggets; i++) addInitialActor(goldNugget);

	frackman = new FrackMan();

	return GWSTATUS_CONTINUE_GAME;
}


int StudentWorld::move() {
	setGameStatText("DUMMY TEXT HERE. MUST CHANGE");

	// This code is here merely to allow the game to build, run, and terminate after you hit enter a few times.
	// Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
	decLives();
	return GWSTATUS_PLAYER_DIED;
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


void StudentWorld::addInitialActor(ActorType actorType) {
	int x, y;
	double dist = 1000;
	//find a good x, y
	do {
		x = rand() % 61;
		y = (rand() % 37) + 20;
		if (y >= 4 && x >= 27 && x <= 33) continue; //check if inside mineshaft
		for (std::vector<Actor*>::const_iterator it = actors.begin(); it != actors.end(); it++) {
			Actor *a = *it;
			int dx = x - a->getX();
			int dy = y - a->getY();
			dist = sqrt(dx*dx + dy*dy);
		}
	} while (dist <= 6.0);
	Actor *actor;
	if (actorType == boulder) {
		actor = new Boulder(x, y);
		for (int i = x; i < x + 4; i++) {
			for (int j = y; j < y + 4; j++) {
				delete dirt[i][j];
				dirt[i][j] = nullptr;
			}
		}
	}
	else if (actorType == oilBarrel) actor = new OilBarrel(x, y);
	else if (actorType == goldNugget) actor = new GoldNugget(x, y, false, true, true);
	actors.push_back(actor);
}