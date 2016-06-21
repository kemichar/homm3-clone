#pragma once

#include <vector>
#include "MapObject.h"
#include "Hero.h"

class MOCreature : public MapObject {

public:
	MOCreature(intp _pos, std::string _creatureName, std::vector<int> _counts);

	MOCreature(intp _pos, std::string _creatureName, int _count);

	MOCreature(intp _pos, Creature* _creatureOriginal, int _count);

	bool isBlocking() override;

	virtual bool isHolding() override;

	virtual void interact() override;

	virtual void draw(float size, bool mapDependency) override;

};