#pragma once

#include "MapObject.h"
#include "Map.h"

using namespace glm;

class MOItem : public MapObject {

public:
	MOItem(intp _pos, Map* _map);

	bool isBlocking();

	bool isHolding();

	virtual void interact();

	virtual void draw(float size);

private:
	Map* mapInstance;

};