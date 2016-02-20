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

	bool isDirt(int x, int y) const { return dirt[x][y] != nullptr; }
	void removeDirt(int x, int y) { delete dirt[x][y]; dirt[x][y] = nullptr; }
	bool struckOil() { playSound(SOUND_FOUND_OIL); increaseScore(1000); nOilBarrels--; return nOilBarrels == 0; }

	FrackMan* getFrackMan() const { return frackman; } //used for checking protester collisions
	BFSSearch* getSearch() const { return search; }
	void setUpdateSearch() const { search->setUpdateMovable(); }
	void updateMovable(bool movable[64][64]);
	bool collides(GraphObject *ob1, GraphObject *ob2, double radius) const;

	int boulderCollisions(Boulder *b);
	int goldNuggetCollisions(GoldNugget *gn, bool isPlayerPickable);
	int oilBarrelCollisions(OilBarrel *ob);
	int squirtCollisions(Squirt *s);
	int goodieCollisions(Goodie *g);
	void frackmanCollisions(FrackMan *f, int ox, int oy);

	///DEBUGGING: HAX functions

	//helper function for SuperSquirt
	Actor* getFirstProtester() {
		Actor *target = nullptr;
		for (int i = 0; i < actors.size(); i++) {
			if (actors[i]->getType() == Actor::REGPROTESTER || actors[i]->getType() == Actor::HCOREPROTESTER) {
				target = actors[i]; break;
			}
		}
		return target;
	}
	void superSquirt() {
		playSound(SOUND_PLAYER_SQUIRT);
		GraphObject::Direction dir = frackman->getDirection();
		int x = frackman->getX(), y = frackman->getY();		
		actors.push_back(new SuperSquirt(x, y, this));
	}
	void superSonar() {
		playSound(SOUND_SONAR);
		for (unsigned int i = 0; i < actors.size(); i++) {
			if (collides(frackman, actors[i], 100.0)) {
				actors[i]->setVisible(true);
			}
		}
	}
	void goToOil() {
		for (int i = 0; i < actors.size(); i++) {
			if (actors[i]->getType() == Actor::OILBARREL) {
				frackman->moveTo(actors[i]->getX(), actors[i]->getY());
				break;
			}
		}
	}
	int nextLevel() { return Actor::LEVEL_SUCCESS; }

	void drawPen15() {
		for (int i = 0; i < 64; i++) {
			for (int j = 0; j < 64; j++) {
				removeDirt(i, j);
			}
		}
		bool draw[6][8] = { {0,1,0,0,0,0,0,0},{1,0,1,1,1,1,1,0},{0,1,0,0,0,0,0,1},{ 0,1,0,0,0,0,0,1 },{1,0,1,1,1,1,1,0},{ 0,1,0,0,0,0,0,0 } };
		for (int k = 0; k < 20; k++) {
			int x = rand()%60, y = rand()%60;
			for (int i = y; i < y + 6; i++) {
				for (int j = x; j < x + 8; j++) {
					if (draw[i - y][j - x]) {
						dirt[i][j] = new Dirt(i, j, this);
					}
				}
			}
		}
		///update movable?
	}
	void callAirStrike() {
		for (int i = 0; i < 20; i++) {
			actors.push_back(new AirStrikeSquirt(3*i, this));
		}
	}

	void createPortal(int x, int y, int color) {
		Portal *newPortal = new Portal(x, y, color, this, color == 1 ? orangePortal : bluePortal);
		if (newPortal->getNext() && collides(newPortal->getNext(), newPortal, 6.0)) {
			delete newPortal; return;
		}
		if (color == 1) {
			//creating blue
			if(bluePortal) delete bluePortal;
			bluePortal = newPortal;
		} else if (color == 2) {
			//creating orange
			if (orangePortal) delete orangePortal;
			orangePortal = newPortal;
		}
		if (newPortal->getNext()) newPortal->getNext()->setNext(newPortal);
	}
	int portalCollisions(Portal *p) {
		Portal *next;
		if (p->getNext() == nullptr) return Actor::CONTINUE;
		else next = p->getNext();
		for (int i = 0; i < actors.size(); i++) {
			if (collides(p, actors[i], 3.0)) {
				actors[i]->moveTo(next->getX(), next->getY());
				next->setWaiting(60);
			}
		}
		if (collides(p, frackman, 3.0)) {
			//frackman->setX(next->getX()); frackman->setY(next->getY());
			frackman->moveTo(next->getX(), next->getY());
			next->setWaiting(60);
		}
		return Actor::CONTINUE;
	}
	Portal *bluePortal;
	Portal *orangePortal;
	///DEBUGGING: end HAX functions


private:
	std::vector<Actor*> actors; //Vector of actors
	FrackMan *frackman; //Player pointer
	Dirt *dirt[VIEW_WIDTH][VIEW_HEIGHT]; //Array of dirt
	BFSSearch *search;
	int ticks, nProtesters, nOilBarrels;

	//function for distributing nuggets, oil, and boulders. ONLY USED IN INIT
	void addInitialActor(Actor::Type actorType);


	//pads text to targetLength with spaces
	std::string widenText(std::string ret, unsigned int targetLength) const {
		while (ret.length() < targetLength) ret = ' ' + ret;
		return ret;
	}
	//helper functions for stat text
	std::string getScoreText() const {
		std::string ret = std::to_string(getScore());
		while (ret.length() < 6) ret = '0' + ret; ///DEBUGGING? 6 or 8?
		return ret;
	}
	std::string getLevelText() const { return widenText(std::to_string(getLevel()), 2); }
	std::string getLivesText() const { return std::to_string(getLives()); }
	std::string getHealthText() const { return widenText(std::to_string(frackman->getHealth()*10), 3) + "%"; }
	std::string getWaterText() const { return widenText(std::to_string(frackman->getWater()), 2); }
	std::string getGoldText() const { return widenText(std::to_string(frackman->getGold()), 2); }
	std::string getSonarText() const { return widenText(std::to_string(frackman->getSonar()), 2); }
	std::string getOilLeftText() const { return widenText(std::to_string(nOilBarrels), 2); }


	//miscellaneous helper functions
	int playerDied() {
		playSound(SOUND_PLAYER_GIVE_UP);
		decLives();
		return GWSTATUS_PLAYER_DIED;
	}
	int min(int a, int b) const { return a < b ? a : b; }
	int max(int a, int b) const { return a > b ? a : b; }
};

#endif // STUDENTWORLD_H_
