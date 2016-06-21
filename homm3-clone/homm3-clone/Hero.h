#pragma once

#include "Constants.h"
#include "FactionMember.h"
#include "Creature.h"

class Hero : public FactionMember {

public:
	Hero(int _factionId, bool _isReal = true);
	~Hero();

	void* getCreature(int index);

	void smartArmySetup(Hero * enemy);

	int strongestStackIndex();

	void stackArmyNeatly();

	float totalUniversalPower();

	int stackCount();

	bool canMove(int distance);

	bool move(int distance);

	void changeMovementPoints(int change);

	void refresh();

	Creature* creatures[HERO_UNIT_SLOTS];
	bool isReal; // is this just a creature container or a real hero
	bool isDead;
	int movementPoints;
	int uniqueId;
	int experience;
};