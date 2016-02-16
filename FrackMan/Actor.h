#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
class StudentWorld;

//Actor
class Actor : public GraphObject {
public:
	Actor(int ID, int x, int y, Direction dir, float size, int depth, StudentWorld *sw);
	virtual ~Actor() = 0;
	virtual int doSomething() = 0;
	enum Return { PLAYER_DIED, SELF_DIED, PICKED_UP, LEVEL_SUCCESS, CONTINUE };
protected:
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
	virtual int doSomething();
};

//Sonar Kit
class SonarKit : public Goodie {
public:
	SonarKit(int x, int y, int t, StudentWorld *sw);
	virtual ~SonarKit() {}
	virtual int doSomething();
};

//Protester
class Protester : public Actor {
public:
	Protester(int ID, int x, int y, int h, StudentWorld *sw);
	virtual ~Protester() = 0;
	virtual int doSomething();
	virtual bool setGiveUp();
	bool setAnnoyed(int t);
	void decHealth(int h);
private:
	enum State {resting, ready, annoyed, giveup};
	int health, state, ticks;
};

//Regular Protester
class RegularProtester : public Protester {
public:
	RegularProtester(int x, int y, StudentWorld *sw);
	virtual ~RegularProtester() {}
	virtual int doSomething();
	virtual bool setGiveUp();
};

//Hardcore Protester
class HardcoreProtester : public Protester {
public:
	HardcoreProtester(int x, int y, StudentWorld *sw);
	virtual ~HardcoreProtester() {}
	virtual int doSomething(); 
	virtual bool setGiveUp();
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
	bool decHealth() { health--; return health == 0; } //return false if frackman dies
	void setHealth(int h) { health = h; }
	void setWater(int w) { water = w; }
	void setSonar(int s) { sonar = s; }
	void setGold(int g) { gold = g; }
private:
	StudentWorld *sWorld;
	int health, water, sonar, gold;
};

#endif // ACTOR_H_
