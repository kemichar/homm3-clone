#pragma once

#include "MapObject.h"
#include "FactionMember.h"
#include "GameLogic.h"

class MOCreature : public MapObject, public FactionMember {

public:
	MOCreature(vec2 _pos, string _creatureName, vector<int> _counts)
		: MapObject(_pos), FactionMember(0) {
		objectCode = 4;
		objectName = "Creature: " + _creatureName;

		for (int i = 0; i < (int)_counts.size() && i < HERO_UNIT_SLOTS; i++) {
			creatures[i] = new Creature(_creatureName, _counts[i]);
		}
	}

	MOCreature(vec2 _pos, string _creatureName, int _count)
		: MapObject(_pos), FactionMember(0) {
		objectCode = 4;
		objectName = "Creature: " + _creatureName;

		creatures[0] = new Creature(_creatureName, _count);
	}

	void onCombatEnd(CombatResult result) {
		if (result.winnerFaction != getFactionId()) {
			GameLogic::instance().map->removeObject(pos);
		}
	}

	bool hasCreatures() {
		return true;
	}

	bool isBlocking() {
		return true;
	}

	virtual bool isHolding() {
		return true;
	}

	virtual void interact() {
		// no way to interact because this object is blocking
	}

	virtual void draw(float size) {
		for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
			if (creatures[i] != nullptr && creatures[i]->count > 0) {
				creatures[i]->draw(size);
				return;
			}
		}

		// TODO modify error report(s)
		printf("Creatures not assigned to %s..\n", objectName.c_str());
		glPushMatrix();
		glColor3f(1, 0, 0);
		glutSolidCube(size);
		glPopMatrix();
	}

private:

};