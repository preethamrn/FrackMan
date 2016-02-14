#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>

#include "Dirt.h"
// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class FrackMan;
class Actor;

class StudentWorld : public GameWorld {
public:
    StudentWorld(std::string assetDir) : GameWorld(assetDir) {}
	virtual ~StudentWorld() {}

    virtual int init() {
		//initializing dirt
		for (int i = 0; i < VIEW_WIDTH; i++) {
			for (int j = 0; j < VIEW_HEIGHT; j++) {
				if (j >= 60) dirt[i][j] = nullptr; //top layer
				else if (j >= 4 && i >= 30 && i <= 33) dirt[i][j] = nullptr; //mineshaft
				else dirt[i][j] = new Dirt(i,j);
			}
		}
		//initializing barrels of oil, boulders, and gold nuggets
		int nBarrels = (2 + getLevel() < 20 ? 2 + getLevel() : 20);
		int nBoulders = (getLevel() / 2 + 2 < 6 ? getLevel() / 2 + 2 : 6);
		int nNuggets = (5 - getLevel() / 2 > 2 ? 5 - getLevel() / 2 : 2);
		for (int i = 0; i < nBarrels; i++) {

		}
		for (int i = 0; i < nBoulders; i++) {

		}
		for (int i = 0; i < nNuggets; i++) {

		}

        return GWSTATUS_CONTINUE_GAME;
    }

    virtual int move() {
		setGameStatText("DUMMY TEXT HERE. MUST CHANGE");

        // This code is here merely to allow the game to build, run, and terminate after you hit enter a few times.
        // Notice that the return value GWSTATUS_PLAYER_DIED will cause our framework to end the current level.
        decLives();
        return GWSTATUS_PLAYER_DIED;
    }

    virtual void cleanUp() {
		for (int i = 0; i < VIEW_WIDTH; i++) {
			for (int j = 0; j < VIEW_HEIGHT; j++) {
				delete dirt[i][j];
			}
		}
    }

private:
	std::vector<Actor*> actors; //Vector of actors (may change from GraphObjects to Actors)
	FrackMan* frackman; //Player pointer
	Dirt *dirt[VIEW_WIDTH][VIEW_HEIGHT]; //Array of dirt
};

#endif // STUDENTWORLD_H_
