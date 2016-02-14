#ifndef DIRT_H_
#define DIRT_H_

#include "GraphObject.h"

class Dirt : public GraphObject {
public:
	Dirt(int sx, int sy) : GraphObject(IID_DIRT, sx, sy, right, 0.25, 3) {}
	virtual ~Dirt() {}
};

#endif