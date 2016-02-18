#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;

//BFS Seach class for finding shortest path to a point
class BFSSearch {
public:
	BFSSearch(StudentWorld *sw);
	int search(GraphObject *ob, int x, int y, GraphObject::Direction &dir);
	//first updates if lastX/lastY have changed
	//returns length of shortest path (-1 if not possible), sets dir = direction to take
	//use lastX and lastY to decide whether you should recompute the shortest path arrays

	///DEBUGGING: Make function that returns whether within line of sight

	//reset lastX, lastY, and updateMovable because shortest paths much be recalculated now
	void setUpdateMovable() { lastX = -1; lastY = -1; m_updateMovable = false; }

	///DEBUGGING!!!!!!
	#include <iostream>
	void printMovable() {
		for (int i = 63; i >= 0; i--) { 
			for (int j = 0; j < 64; j++)
				std::cout << movable[j][i];
				/*switch (shortestPathDirection[j][i]) {
				case GraphObject::Direction::right: std::cout << 'r'; break;
				case GraphObject::Direction::up: std::cout << 'u'; break;
				case GraphObject::Direction::down: std::cout << 'd'; break;
				case GraphObject::Direction::left: std::cout << 'l'; break;
				case GraphObject::Direction::none: std::cout << 'x'; break;
				}*/
			std::cout << std::endl;
		} 
		for (int i = 0; i < 64; i++) std::cout << '-'; std::cout << std::endl;
	} ///DEBUGGING!!!!
private:
	bool movable[64][64];
	GraphObject::Direction shortestPathDirection[64][64];
	int shortestPathLength[64][64];
	int lastX, lastY;
	bool m_updateMovable;
	StudentWorld *sWorld;

	void update(int x, int y);
	//first updates movable if not yet updated
	//updates shortest path arrays to contain shortest path to position (x,y)
	void updateMovable();
	//updates movable array to show where protesters can move

	//Auxiliary class for queue based search
	class Point {
	public:
		Point(int x, int y) : m_x(x), m_y(y) {}
		int x() const { return m_x; }
		int y() const { return m_y; }
	private:
		int m_x;
		int m_y;
	};
};

//Actor
class Actor : public GraphObject {
public:
	Actor(int ID, int x, int y, Direction dir, float size, int depth, StudentWorld *sw);
	virtual ~Actor() = 0;
	virtual int doSomething() = 0;
	StudentWorld* getStudentWorld();
	enum Return { PLAYER_DIED, SELF_DIED, PICKED_UP, LEVEL_SUCCESS, CONTINUE };
private:
	StudentWorld *sWorld;
};

//Dirt
class Dirt : public Actor {
public:
	Dirt(int x, int y, StudentWorld *sw);
	virtual ~Dirt() {}
	virtual int doSomething() { return CONTINUE; }
};

//Boulder
class Boulder : public Actor {
public:
	Boulder(int x, int y, StudentWorld *sw);
	virtual ~Boulder() {}
	virtual int doSomething();
private:
	enum State {stable, falling, waiting};
	int ticks, state;
};

//Gold Nugget
class GoldNugget : public Actor {
public:
	GoldNugget(int x, int y, bool isVisible, bool isPlayerPickable, bool isPermanent, StudentWorld *sw);
	virtual ~GoldNugget() {}
	virtual int doSomething();
private:
	enum State {stable, temporary};
	int ticks, state;
};

//Barrel of Oil
class OilBarrel : public Actor {
public:
	OilBarrel(int x, int y, StudentWorld *sw);
	virtual ~OilBarrel() {}
	virtual int doSomething();
};

//Squirt
class Squirt : public Actor {
public:
	Squirt(int x, int y, Direction dir, StudentWorld *sw);
	virtual ~Squirt() {}
	virtual int doSomething();
private:
	int ticks;
};

//Goodie
class Goodie : public Actor {
public:
	Goodie(int ID, int x, int y, int t, StudentWorld *sw);
	virtual ~Goodie() {}
	virtual int doSomething();
private:
	int ticks;
};

//Water Pool
class WaterPool : public Goodie {
public:
	WaterPool(int x, int y, int t, StudentWorld *sw);
	virtual ~WaterPool() {}
};

//Sonar Kit
class SonarKit : public Goodie {
public:
	SonarKit(int x, int y, int t, StudentWorld *sw);
	virtual ~SonarKit() {}
};

//Protester
class Protester : public Actor {
public:
	Protester(int ID, int x, int y, int t, int h, StudentWorld *sw);
	virtual ~Protester() = 0;
	virtual int doSomething();
	virtual bool setResting(int t);
	void decHealth(int h);
private:
	virtual bool tryChasingFrackman() = 0;
	int health, state, ticks, waitingTicks;
};

//Regular Protester
class RegularProtester : public Protester {
public:
	RegularProtester(int x, int y, int t, StudentWorld *sw);
	virtual ~RegularProtester() {}
	virtual int doSomething();
private:
	virtual bool tryChasingFrackman();
};

//Hardcore Protester
class HardcoreProtester : public Protester {
public:
	HardcoreProtester(int x, int y, int t, StudentWorld *sw);
	virtual ~HardcoreProtester() {}
	virtual int doSomething(); 
private:
	virtual bool tryChasingFrackman();
};

//Dead Protester
class DeadProtester : public Protester {
public:
	DeadProtester(int x, int y, StudentWorld *sw);
	virtual ~DeadProtester() {}
	virtual int doSomething();
	virtual bool setResting(int t) { return false; } //nothing can affect a dead protester
private:
	virtual bool tryChasingFrackman() { return false; } //dead protester doesn't chase frackman
};

class FrackMan : public GraphObject {
public:
	FrackMan(StudentWorld *sw);
	virtual ~FrackMan() {}
	virtual int doSomething();

	//accessor functions
	int getHealth() { return health; }
	int getWater() { return water; }
	int getSonar() { return sonar; }
	int getGold() { return gold; }
	//mutator functions
	bool decHealth(int h) { health-=h; return health <= 0; } //return false if frackman dies
	void setHealth(int h) { health = h; }
	void setWater(int w) { water = w; }
	void setSonar(int s) { sonar = s; }
	void setGold(int g) { gold = g; }
private:
	StudentWorld *sWorld;
	int health, water, sonar, gold;
};

#endif // ACTOR_H_
