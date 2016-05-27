#include "Hero.h"

Hero::Hero(int _factionId, bool _isReal)
	: FactionMember(_factionId), isReal(_isReal){
	refresh();
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		creatures[i] = nullptr;
	}

	experience = 0;
	isDead = false;

	static int uniqueIdCounter = 0;
	uniqueId = uniqueIdCounter++;
}

Hero::~Hero() {
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		delete creatures[i];
	}
}

void * Hero::getCreature(int index) {
	if (index > HERO_UNIT_SLOTS) {
		return nullptr;
	}

	return creatures[index];
}


int Hero::stackCount() {
	int ret = 0;
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		if (creatures[i] != nullptr && creatures[i]->count > 0) {
			ret++;
		}
	}
	return ret;
}

/* refactored
void Hero::onCombatEnd(CombatResult result) {
	if (result.winnerFaction == getFactionId()) {
		experience += result.experience;
	}
	else {
		isDead = true;
	}
}*/

inline bool Hero::canMove(int distance) {
	return movementPoints >= distance;
}

bool Hero::move(int distance) {
	if (!canMove(distance)) {
		return false;
	}

	movementPoints -= distance;
	return true;
}

void Hero::changeMovementPoints(int change) {
	movementPoints += change;
}

void Hero::refresh() {
	movementPoints = HERO_BASE_MOVE_POINTS; // TODO modifiers
}