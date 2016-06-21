#pragma once

#include "MapObject.h"
#include "Map.h"

class MOBuilding : public MapObject {

public:
	MOBuilding(Map* map, intp _pos, int _factionId = 0);

	bool isBlocking() override;

	virtual bool isHolding() override;

	virtual void interact() override = 0;

	virtual void draw(float size, bool mapDependency = true) override;

protected:
	floatp modelOffset;
	floatp entranceAngleOffset;

};