#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

//Actor
class Actor : public GraphObject {
public:
	Actor(int ID, int x, int y, Direction dir, float size, int depth);
	virtual ~Actor() {}
};

//Dirt
class Dirt : public GraphObject {
public:
	Dirt(int x, int y);
	virtual ~Dirt() {}
};

//Boulder
class Boulder : public Actor {
public:
	Boulder(int x, int y);
	virtual ~Boulder() {}
};

//Gold Nugget
class GoldNugget : public Actor {
public:
	GoldNugget(int x, int y, bool isVisible, bool isPlayerPickable, bool isPermanent);
	virtual ~GoldNugget() {}
};

//Barrel of Oil
class OilBarrel : public Actor {
public:
	OilBarrel(int x, int y);
	virtual ~OilBarrel() {}
};


class Squirt : public Actor {};
class WaterPool : public Actor {};
class SonarKit : public Actor {};

//Protester
class Protester : public Actor {
public:
	Protester(int ID, int x, int y);
	virtual ~Protester() {}
};

//Hardcore Protester
class HardcoreProtester : public Protester {
public:
	HardcoreProtester(int x, int y);
	virtual ~HardcoreProtester() {}
};


class FrackMan : public Actor {
public:
	FrackMan();
	virtual ~FrackMan() {}
};

#endif // ACTOR_H_
