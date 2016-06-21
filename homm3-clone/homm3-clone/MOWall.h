#pragma once

#include "MapObject.h"

class MOWall : public MapObject {

public:
	MOWall(intp _pos, bool invisible = false);

	bool isBlocking() override;

	virtual void draw(float size, bool mapDependency) override;

	virtual void interact() override;

};