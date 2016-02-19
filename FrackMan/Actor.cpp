#include "Actor.h"
#include "StudentWorld.h"
#include <queue>

//BFSSearch functions
BFSSearch::BFSSearch(StudentWorld *sw) {
	sWorld = sw;
	lastX = -1; lastY = -1;
	m_updatedMovable = false;
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++)
			movable[i][j] = false;
}

bool BFSSearch::search(GraphObject *ob, int x, int y, GraphObject::Direction &dir, int &length) {
	if (lastX != x || lastY != y) update(x, y); //if the last search was different, reupdate the search
	dir = shortestPathDirection[ob->getX()][ob->getY()];
	length = shortestPathLength[ob->getX()][ob->getY()];
	return straightLine[ob->getX()][ob->getY()];
}

bool BFSSearch::isMovable(int x, int y) {
	updateMovable();
	if (x < 0 || x > 60 || y < 0 || y > 60) return false;
	return movable[x][y];
}

void BFSSearch::update(int sx, int sy) {
	if (!m_updatedMovable) updateMovable(); //if movable hasn't been updated, update it
	//init directions and shortest paths
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			shortestPathDirection[i][j] = GraphObject::Direction::none;
			shortestPathLength[i][j] = -1;
			straightLine[i][j] = false;
		}
	}

	std::queue<Point> pointQueue;
	int nRows = 61, nCols = 61;
	
	if (sx < 0 || sx > nRows || sy < 0 || sy > nCols) return;
	if (!movable[sx][sy]) return;

	pointQueue.push(Point(sx, sy));
	shortestPathDirection[sx][sy] = GraphObject::Direction::none;
	shortestPathLength[sx][sy] = 0;
	straightLine[sx][sy] = true;

	//Queue based search for most optimal path
	while (!pointQueue.empty()) {
		Point curr = pointQueue.front(); pointQueue.pop();
		int x = curr.x(), y = curr.y();

		if (y < nCols - 1 && movable[x][y + 1] && shortestPathDirection[x][y + 1] == GraphObject::Direction::none) {
			//if the current point is reachable in a straight line and the point after is in the same direction then it is also reachable in a straight line
			if (straightLine[x][y] && (shortestPathDirection[x][y] == GraphObject::Direction::none || shortestPathDirection[x][y] == GraphObject::Direction::down)) straightLine[x][y + 1] = true;
			pointQueue.push(Point(x, y + 1)); //up
			shortestPathDirection[x][y + 1] = GraphObject::Direction::down;
			shortestPathLength[x][y + 1] = shortestPathLength[x][y] + 1;
		}
		if (y > 0 && movable[x][y - 1] && shortestPathDirection[x][y - 1] == GraphObject::Direction::none) {
			if (straightLine[x][y] && (shortestPathDirection[x][y] == GraphObject::Direction::none || shortestPathDirection[x][y] == GraphObject::Direction::up)) straightLine[x][y - 1] = true;
			pointQueue.push(Point(x, y - 1)); //down
			shortestPathDirection[x][y - 1] = GraphObject::Direction::up;
			shortestPathLength[x][y - 1] = shortestPathLength[x][y] + 1;
		}
		if (x < nRows - 1 && movable[x + 1][y] && shortestPathDirection[x + 1][y] == GraphObject::Direction::none) {
			if (straightLine[x][y] && (shortestPathDirection[x][y] == GraphObject::Direction::none || shortestPathDirection[x][y] == GraphObject::Direction::left)) straightLine[x + 1][y] = true;
			pointQueue.push(Point(x + 1, y)); //right
			shortestPathDirection[x + 1][y] = GraphObject::Direction::left;
			shortestPathLength[x + 1][y] = shortestPathLength[x][y] + 1;
		}
		if (x > 0 && movable[x - 1][y] && shortestPathDirection[x - 1][y] == GraphObject::Direction::none) {
			if (straightLine[x][y] && (shortestPathDirection[x][y] == GraphObject::Direction::none || shortestPathDirection[x][y] == GraphObject::Direction::right)) straightLine[x - 1][y] = true;
			pointQueue.push(Point(x - 1, y)); //left
			shortestPathDirection[x - 1][y] = GraphObject::Direction::right;
			shortestPathLength[x - 1][y] = shortestPathLength[x][y] + 1;
		}
	}
	shortestPathDirection[sx][sy] = GraphObject::Direction::none;
}

void BFSSearch::updateMovable() {
	if (m_updatedMovable) return;
	sWorld->updateMovable(movable); //asks for Student World to update movable for it (since BFSSearch doesn't know anything about oil field's contents)
	m_updatedMovable = true;
}


//Actor functions
Actor::Actor(int ID, int x, int y, Direction dir, float size, int depth, Type type, StudentWorld *sw) : GraphObject(ID, x, y, dir, size, depth), sWorld(sw), m_type(type) {}
inline Actor::~Actor() {}
StudentWorld* Actor::getStudentWorld() { return sWorld; }

//Dirt functions
Dirt::Dirt(int x, int y, StudentWorld *sw) : Actor(IID_DIRT, x, y, right, 0.25, 3, DIRT, sw) { setVisible(true); }


//Boulder functions
Boulder::Boulder(int x, int y, StudentWorld *sw) : Actor(IID_BOULDER, x, y, down, 1.0, 1, BOULDER, sw), state(stable), ticks(30) { setVisible(true); }
int Boulder::doSomething() {
	//if all waiting ticks have elapsed, start falling
	if (!ticks) {
		getStudentWorld()->playSound(SOUND_FALLING_ROCK);
		state = falling;
		ticks = 30;
	}
	//check if waiting
	if (state == waiting) ticks--;
	else if (state == stable) {
		state = waiting;
		for (int i = 0; i < 4; i++) {
			if (getStudentWorld()->isDirt(getX() + i, getY() - 1)) state = stable;
		} //state becomes waiting if there's no dirt underneath rock
	}
	if (state == falling) {
		moveTo(getX(), getY() - 1);

		getStudentWorld()->setUpdateSearch(); //search space must be updated

		if (getY() < 0) return SELF_DIED; //hit the bottom
		for (int i = 0; i < 4; i++) {
			if (getStudentWorld()->isDirt(getX() + i, getY())) return SELF_DIED; //hit dirt
		}
		//check collisions with other objects
		return getStudentWorld()->boulderCollisions(this);
	}
	return CONTINUE;
}

//GoldNugget functions
GoldNugget::GoldNugget(int x, int y, bool isVisible, bool isPlayerPickable, bool isPermanent, StudentWorld *sw) : Actor(IID_GOLD, x, y, right, 1.0, 2, GOLDNUGGET, sw), ticks(100) { 
	setVisible(isVisible); 
	if (isPlayerPickable) state = stable;
	else state = temporary;
}
int GoldNugget::doSomething() {
	if (!ticks) return SELF_DIED;
	//check if collided with frackman
	if (state == stable) {
		return getStudentWorld()->goldNuggetCollisions(this, true); //check if collided with frackman
	}
	else if (state == temporary) {
		ticks--; //count down ticks
		return getStudentWorld()->goldNuggetCollisions(this, false); //check if collided with a protester
	}
	return CONTINUE;
}

//OilBarrel functions
OilBarrel::OilBarrel(int x, int y, StudentWorld *sw) : Actor(IID_BARREL, x, y, right, 1.0, 2, OILBARREL, sw) { setVisible(false); }
int OilBarrel::doSomething() {
	//if it collides with frackman then tell the level that he struck oil
	return getStudentWorld()->oilBarrelCollisions(this); //return appropriate value depending on whether it collided and found all oil
}

//Squirt functions
Squirt::Squirt(int x, int y, Direction dir, StudentWorld *sw) : Actor(IID_WATER_SPURT, x, y, dir, 1.0, 2, SQUIRT, sw), ticks(5) { setVisible(true); }
int Squirt::doSomething() {
	if (!ticks) return SELF_DIED;
	
	//go through all protester collisions before deciding whether to kill the squirt
	Direction dir = getDirection();
	int x = getX(), y = getY();
	switch (dir) {
	case up: moveTo(x, y + 1); break;
	case down: moveTo(x, y - 1); break;
	case right: moveTo(x + 1, y); break;
	case left: moveTo(x - 1, y); break;
	default:;
	}

	//check dirt/edge collisions
	if (getX() < 0 || getY() < 0 || getX() > 60 || getY() > 60) return SELF_DIED;
	for (int i = getX(); i < getX() + 4; i++)
		for (int j = getY(); j < getY() + 4; j++)
			if (getStudentWorld()->isDirt(i, j)) return SELF_DIED;
	//check boulder/other collisions
	if (getStudentWorld()->squirtCollisions(this) == SELF_DIED) return SELF_DIED;

	ticks--;
	return CONTINUE;
}

//Goodie functions
Goodie::Goodie(int ID, int x, int y, int t, Type type, StudentWorld *sw) : Actor(ID, x, y, right, 1.0, 2, type, sw), ticks(t) { setVisible(true); }
int Goodie::doSomething() {
	if (!ticks) return SELF_DIED;
	//if frackman picks up goodie
	if (getStudentWorld()->goodieCollisions(this) == SELF_DIED) return SELF_DIED;
	ticks--;
	return CONTINUE;
}

//WaterPool functions
WaterPool::WaterPool(int x, int y, int t, StudentWorld *sw) : Goodie(IID_WATER_POOL, x, y, t, WATERPOOL, sw) {}

//SonarKit functions
SonarKit::SonarKit(int x, int y, int t, StudentWorld *sw) : Goodie(IID_SONAR, x, y, t, SONARKIT, sw) {}


//Protester functions
Protester::Protester(int ID, int x, int y, int t, int h, Type type, StudentWorld *sw) : Actor(ID, x, y, left, 1.0, 0, type, sw), health(h), waitingTicks(t), ticks(t), numStepsInDir(rand() % 53 + 8), stepsSincePerp(0), dead(false) { setVisible(true); }
inline Protester::~Protester() {}
void Protester::setResting(int t) { ticks = t; }
bool Protester::decHealth(int h) { health -= h; return health <= 0; }
void Protester::setDead() { dead = true; }
void Protester::changeDir(Direction d) {
	if ((getDirection() == right || getDirection() == left) && (d == up || d == down) || (getDirection() == up || getDirection() == down) && (d == right || d == left))
		stepsSincePerp = 0; //new direction is perpendicular to current direction
	else stepsSincePerp++; //new direction is parallel to current direction (so increase steps since perpendicular)
	if (getDirection() != d) {
		numStepsInDir = 0; 
	}
	setDirection(d);
}
bool Protester::moveDir(Direction d) {
	int x = getX(), y = getY();
	changeDir(d);
	int dx = 0, dy = 0;
	switch (d) {
	case GraphObject::up: dy = 1; break;
	case GraphObject::down: dy = -1; break;
	case GraphObject::right: dx = 1; break;
	case GraphObject::left: dx = -1; break;
	default: break;
	}
	if (!getStudentWorld()->getSearch()->isMovable(x + dx, y + dy)) return false;
	moveTo(x + dx, y + dy);
	return true;
}
int Protester::doSomething() {
	if (dead) return SELF_DIED;
	if (ticks) {
		ticks--;
		return CONTINUE;
	}
	//if protester isn't waiting, it'll start waiting from the next tick
	ticks = waitingTicks;

	//if protester hits frackman check if it killed him, return PLAYER_DIED from here
	if (getStudentWorld()->collides(this, getStudentWorld()->getFrackMan(), 4.0)) {
		getStudentWorld()->playSound(SOUND_PROTESTER_YELL);
		if(getStudentWorld()->getFrackMan()->decHealth(2)) return PLAYER_DIED;
		ticks = 16 * waitingTicks + 1; //15 non-resting ticks
		return CONTINUE;
	}
	//check if frackman is in a straight line to the protester
	FrackMan *f = getStudentWorld()->getFrackMan();
	Direction dir = none; int length = -1;
	if (getStudentWorld()->getSearch()->search(this, f->getX(), f->getY(), dir, length)) {
		//frackman is reachable in a straight line
		moveDir(dir);
		return CONTINUE;
	}
	//try chasing frackman if protester can trace frackman's cellphone signal
	if(tryChasingFrackman()) return CONTINUE; //if it could chase frackman 

	//try walking in some direction
	if (!numStepsInDir) {
		//looks for a new direction that the protester can move into
		///DEBUGGING: looks for a NEW direction?
		int randInt = -1, dx = 0, dy = 0;
		do {
			randInt = rand() % 4; //0->up, 1->down, 2->right, 3->left
			if (randInt == 0 && getDirection() == up ||
				randInt == 1 && getDirection() == down ||
				randInt == 2 && getDirection() == right ||
				randInt == 3 && getDirection() == left) continue; //choose a new direction so ignore randInt if direction is the same
			dx = randInt == 2 ? 1 : randInt == 3 ? -1 : 0;
			dy = randInt == 0 ? 1 : randInt == 1 ? -1 : 0;
		} while (!getStudentWorld()->getSearch()->isMovable(getX()+dx, getY()+dy));
		switch (randInt) {
		case 0: changeDir(up); break;
		case 1: changeDir(down); break;
		case 2: changeDir(right); break;
		case 3: changeDir(left); break;
		default: break;
		}
		numStepsInDir = rand() % 53 + 8;
	}
	if (stepsSincePerp >= 200) {
		if (getDirection() == up || getDirection() == down) {
			if (getX() < 30) {
				if (getStudentWorld()->getSearch()->isMovable(getX() + 1, getY())) changeDir(right);
			} else if (getStudentWorld()->getSearch()->isMovable(getX() - 1, getY())) changeDir(left);
		} else if (getDirection() == right || getDirection() == left) {
			if (getY() < 30) {
				if (getStudentWorld()->getSearch()->isMovable(getX(), getY() + 1)) changeDir(up);
			} else if (getStudentWorld()->getSearch()->isMovable(getX(), getY() - 1)) changeDir(down);
		}
		numStepsInDir = rand() % 53 + 8;
	}
	if(moveDir(getDirection())) numStepsInDir--; //if it's possible to move in that direction
	else numStepsInDir = 0; //chose a new direction next non-resting tick

	return CONTINUE;
}

//Regular Protester functions
RegularProtester::RegularProtester(int x, int y, int t, StudentWorld *sw) : Protester(IID_PROTESTER, x, y, t, 5, REGPROTESTER, sw) {}

//HardcoreProtester functions
HardcoreProtester::HardcoreProtester(int x, int y, int t, int m, StudentWorld *sw) : Protester(IID_HARD_CORE_PROTESTER, x, y, t, 20, HCOREPROTESTER, sw), chaseDist(m) {}
bool HardcoreProtester::tryChasingFrackman() {
	int l = -1; Direction d = none; FrackMan *f = getStudentWorld()->getFrackMan();
	getStudentWorld()->getSearch()->search(this, f->getX(), f->getY(), d, l);
	if (l != -1 && l <= chaseDist) {
		moveDir(d);
		return true;
	}
	return false;
}

//Dead functions
DeadProtester::DeadProtester(int ID, int x, int y, int t, StudentWorld *sw) : Protester(ID, x, y, 0, -1, DEADPROTESTER, sw), waitingTicks(t), ticks(1) { setVisible(false); }
int DeadProtester::doSomething() {
	if (ticks) {
		ticks--;
		return CONTINUE;
	}
	setVisible(true); //don't show up on screen until it starts moving
	if (getX() == 60 && getY() == 60) return SELF_DIED;
	ticks = waitingTicks;
	int l = -1; Direction d = none;
	getStudentWorld()->getSearch()->search(this, 60, 60, d, l);
	moveDir(d);
	return CONTINUE;
}

//FrackMan functions
FrackMan::FrackMan(StudentWorld *sw) : Actor(IID_PLAYER, 30, 60, right, 1.0, 0, FRACKMAN, sw), health(10), water(5), sonar(1), gold(0) {
	setVisible(true);
}
int FrackMan::doSomething() {
	//check if collided with dirt
	bool removedDirt = false;
	for (int i = getX(); i < getX() + 4; i++) {
		for (int j = getY(); j < getY() + 4; j++) {
			if (getStudentWorld()->isDirt(i, j)) {
				getStudentWorld()->removeDirt(i, j);
				removedDirt = true;
			}
		}
	}
	if (removedDirt) {
		getStudentWorld()->playSound(SOUND_DIG);
		getStudentWorld()->setUpdateSearch();
	}
	int ch;
	int originalX = getX(), originalY = getY();
	if (getStudentWorld()->getKey(ch)) {
		switch (ch) {
		case KEY_PRESS_UP: case 'W':
			if (getDirection() == up) moveTo(getX(), getY() + 1);
			else setDirection(up); break;
		case KEY_PRESS_DOWN: case 'S':
			if (getDirection() == down) moveTo(getX(), getY() - 1);
			else setDirection(down); break;
		case KEY_PRESS_RIGHT: case 'D':
			if (getDirection() == right) moveTo(getX() + 1, getY());
			else setDirection(right); break;
		case KEY_PRESS_LEFT: case 'A':
			if (getDirection() == left) moveTo(getX() - 1, getY());
			else setDirection(left); break;
		case KEY_PRESS_TAB:
			if (gold > 0) {
				gold--;
				getStudentWorld()->useGold();
			}
			break;
		case KEY_PRESS_SPACE:
			if (water > 0) {
				water--;
				getStudentWorld()->useWater();
			}
			break;
		case 'z': case 'Z':
			if (sonar > 0) {
				sonar--;
				getStudentWorld()->useSonar();
			}
			break;

		///DEBUGGING: HAX controls
		case 'b': case 'B':
			getStudentWorld()->goToOil();
			break;
		case 'n': case 'N':
			return getStudentWorld()->nextLevel();
			break;
		case 'g': case 'G':
			getStudentWorld()->superSquirt();
			break;
		case 'x': case 'X':
			getStudentWorld()->superSonar();
			break;
		case 'u': water += 20; break;
		case 'i': sonar += 20; break;
		case 'o': gold += 20; break;
		case 'h': health += 100; break;
		///DEBUGGING: end HAX controls


		case KEY_PRESS_ESCAPE: return Actor::PLAYER_DIED;
		default: moveTo(originalX, originalY); break;
		}

		//check if collided with edges
		if (getX() < 0 || getY() < 0 || getX() > VIEW_WIDTH - 4 * getSize() || getY() > VIEW_HEIGHT - 4 * getSize())
			for (int i = 0; i < 4; i++) moveTo(originalX, originalY); //run moveTo multiple times to run through animation
		//check collisions with boulders and makes some objects visible if they are close enough
		getStudentWorld()->frackmanCollisions(this, originalX, originalY);
	}
	if(health > 0) return Actor::CONTINUE;
	else return Actor::PLAYER_DIED;
}