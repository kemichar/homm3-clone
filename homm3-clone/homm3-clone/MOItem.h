#pragma once

#include "MapObject.h"

using namespace glm;

class MOItem : public MapObject {

public:
	MOItem(intp _pos);

	bool isBlocking() override;

	bool isHolding() override;

	virtual void interact() override;

	virtual void draw(float size, bool mapDependency) override;

};