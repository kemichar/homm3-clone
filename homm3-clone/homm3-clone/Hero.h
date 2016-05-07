#pragma once

#include "MapObject.h"
#include "FactionMember.h"
#include "Constants.h"

class Hero : public MapObject, public FactionMember {

public:
	Hero(FactionMember* _factionRepresentative, vec2 _position = vec2(0, 0))
		: FactionMember(_factionRepresentative->getFactionId()), MapObject(_position) {
		refresh();
		for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
			creatures[i] = nullptr;
		}

		experience = 0;
	}

	bool hasCreatures() {
		return true;
	}

	void* getCreature(int index) {
		if (index > HERO_UNIT_SLOTS) {
			return nullptr;
		}

		return creatures[index];
	}

	void draw(float size) {
		glutSolidCone(0.1, 0.2, 5, 5);
	}

	void onCombatEnd(CombatResult result) {
		if (result.winnerFaction == getFactionId()) {
			experience += result.experience;
		}
		else {
			// TODO kill the hero etc.
		}
	}

	inline bool canMove(int distance) {
		return movementPoints >= distance;
	}

	bool move(int distance) {
		if (!canMove(distance)) {
			return false;
		}

		movementPoints -= distance;
		return true;
	}

	void changeMovementPoints(int change) {
		movementPoints += change;
	}

	void refresh() {
		movementPoints = HERO_BASE_MOVE_POINTS; // TODO modifiers
	}

private:
	int movementPoints;
	int experience;
	// TODO add creature and item storage
};