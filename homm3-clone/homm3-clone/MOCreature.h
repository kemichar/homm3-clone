#pragma once

#include "MapObject.h"
#include "Hero.h"

class MOCreature : public MapObject {

public:
	MOCreature(intp _pos, string _creatureName, vector<int> _counts);

	MOCreature(intp _pos, string _creatureName, int _count);

	MOCreature(intp _pos, Creature* _creatureOriginal, int _count);

	bool isBlocking() override;

	virtual bool isHolding() override;

	virtual void interact() override;

	virtual void draw(float size) override;

};