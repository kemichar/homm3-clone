#pragma once

#include "MOBuilding.h"
#include "FactionMember.h"
#include "GameLogic.h"

class MOMine : public MOBuilding {

public:
	MOMine(int _type, Map* map, intp _pos, int _factionId = 0);

	virtual void interact() override;

	virtual void draw(float size) override;

	int type;

};