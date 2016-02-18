#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include <string>
#include <vector>
#include "Actor.h"


class StudentWorld : public GameWorld {
public:
	StudentWorld(std::string assetDir) : GameWorld(assetDir) { }
	virtual ~StudentWorld() {}

	virtual int init();
	virtual int move();
	virtual void cleanUp();


	void useSonar();
	void useWater();
	void useGold();

	bool isDirt(int x, int y) { return dirt[x][y] != nullptr; }
	void removeDirt(int x, int y) { delete dirt[x][y]; dirt[x][y] = nullptr; }
	bool struckOil() { playSound(SOUND_FOUND_OIL); increaseScore(1000); nOilBarrels--; return nOilBarrels == 0; }

	FrackMan* getFrackMan() { return frackman; } //used for checking protester collisions
	BFSSearch* getSearch() { return search; }
	void setUpdateSearch() { search->setUpdateMovable(); }
	void updateMovable(bool movable[][64]);
	bool collides(GraphObject *ob1, GraphObject *ob2, double radius);

	int boulderCollisions(Boulder *b);
	int goldNuggetCollisions(GoldNugget *gn, bool isPlayerPickable);
	int oilBarrelCollisions(OilBarrel *ob);
	int squirtCollisions(Squirt *s);
	int goodieCollisions(Goodie *g);
	void frackmanCollisions(FrackMan *f, int ox, int oy);

private:
	std::vector<Actor*> actors; //Vector of actors
	FrackMan* frackman; //Player pointer
	Dirt *dirt[VIEW_WIDTH][VIEW_HEIGHT]; //Array of dirt
	BFSSearch *search;
	int ticks, nProtesters, nOilBarrels;

	enum ActorType { boulder, oilBarrel, goldNugget };
	//function for distributing nuggets, oil, and boulders. ONLY USED IN INIT
	void addInitialActor(ActorType actorType);


	//pads text to targetLength with spaces
	std::string widenText(std::string ret, unsigned int targetLength) {
		while (ret.length() < targetLength) ret = ' ' + ret;
		return ret;
	}
	//helper functions for stat text
	std::string getScoreText() {
		std::string ret = std::to_string(getScore());
		while (ret.length() < 6) ret = '0' + ret; ///DEBUGGING? 6 or 8?
		return ret;
	}
	std::string getLevelText() { return widenText(std::to_string(getLevel()), 2); }
	std::string getLivesText() { return std::to_string(getLives()); }
	std::string getHealthText() { return widenText(std::to_string(frackman->getHealth()*10), 3) + "%"; }
	std::string getWaterText() { return widenText(std::to_string(frackman->getWater()), 2); }
	std::string getGoldText() { return widenText(std::to_string(frackman->getGold()), 2); }
	std::string getSonarText() { return widenText(std::to_string(frackman->getSonar()), 2); }
	std::string getOilLeftText() { return widenText(std::to_string(nOilBarrels), 2); }


	//miscellaneous helper functions
	int playerDied() {
		playSound(SOUND_PLAYER_GIVE_UP);
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}
	int min(int a, int b) { return a < b ? a : b; }
	int max(int a, int b) { return a > b ? a : b; }
};

#endif // STUDENTWORLD_H_
