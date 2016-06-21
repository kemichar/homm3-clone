#pragma once

#include "MOEmpty.h"
#include "Map.h"

class MOPath : public MOEmpty {

public:
	MOPath(Map* map, intp _pos);

	// TEMP scribbling some paths
	virtual void draw(float size, bool mapDependency);

private:
	int tempShape; // left, down, right, up

};