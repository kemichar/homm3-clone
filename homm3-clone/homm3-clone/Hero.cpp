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

void Hero::smartArmySetup(Hero* enemy) {
	if (HERO_UNIT_SLOTS != 6) {
		printf("WARNING change the army setup in Hero.cpp\n");
	}

	float power = totalUniversalPower();
	float enemyPower = enemy->totalUniversalPower();

	int stacks = 1;
	if (power >= 2 * enemyPower) {
		stacks = 6;
	}
	else if (power >= 1.5 * enemyPower) {
		stacks = 5;
	}
	else if (power >= 1.0 * enemyPower) {
		stacks = 4;
	}
	else if (power >= 0.5 * enemyPower) {
		stacks = 3;
	}
	else if (power >= 0.25 * enemyPower) {
		stacks = 2;
	}
	else {
		stacks = 1;
	}

	// just in case
	stacks = mmin(stacks, HERO_UNIT_SLOTS);

	stackArmyNeatly();
	int unjoinableStacks = 0;
	while (unjoinableStacks < HERO_UNIT_SLOTS && creatures[unjoinableStacks] != nullptr) {
		unjoinableStacks++;
	}
	if (unjoinableStacks == 0 || unjoinableStacks == HERO_UNIT_SLOTS || unjoinableStacks >= stacks) {
		return;
	}

	for (int i = unjoinableStacks; i < stacks; i++) {
		int splitIndex = strongestStackIndex();
		if (creatures[splitIndex]->count == 1) {
			continue; // TODO
		}
		creatures[i] = new Creature(creatures[splitIndex], creatures[splitIndex]->count / 2, this->getFactionId());
		creatures[splitIndex]->count -= creatures[splitIndex]->count / 2; // intentionally, for even / odd amounts
	}
}

int Hero::strongestStackIndex() {
	int ret = -1;

	float bestPower = 0;
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		if (creatures[i] != nullptr && creatures[i]->universalPower() > bestPower) {
			ret = i;
			bestPower = creatures[i]->universalPower();
		}
	}

	return ret;
}

void Hero::stackArmyNeatly() {
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		if (creatures[i] == nullptr) {
			for (int j = i + 1; j < HERO_UNIT_SLOTS; j++) {
				if (creatures[j] != nullptr) {
					creatures[i] = creatures[j];
					creatures[j] = nullptr;
					break;
				}
			}
		}
		if (creatures[i] != nullptr) {
			int moveCount = 0;
			for (int j = i + 1; j < HERO_UNIT_SLOTS; j++) {
				if (creatures[j] != nullptr && creatures[j]->name == creatures[i]->name) {
					moveCount += creatures[j]->count;
					delete creatures[j];
					creatures[j] = nullptr;
				}
			}
			creatures[i]->count += moveCount;
		}
		else {
			break;
		}
	}
}

float Hero::totalUniversalPower() {
	float ret = 0;
	for (Creature* creature : creatures) {
		if (creature != nullptr) {
			ret += creature->universalPower();
		}
	}
	return ret;
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

bool Hero::canMove(int distance) {
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