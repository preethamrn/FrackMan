#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <cmath>
#include <vector>
#include "Actor.h"


class StudentWorld : public GameWorld {
public:
    StudentWorld(std::string assetDir) : GameWorld(assetDir) {}
	virtual ~StudentWorld() {}

	virtual int init();
	virtual int move();
	virtual void cleanUp();

private:
	std::vector<Actor*> actors; //Vector of actors
	FrackMan* frackman; //Player pointer
	Dirt *dirt[VIEW_WIDTH][VIEW_HEIGHT]; //Array of dirt

	enum ActorType {boulder, oilBarrel, goldNugget};

	//function for distributing nuggets, oil, and boulders
	void addInitialActor(ActorType actorType);
};

#endif // STUDENTWORLD_H_
