#pragma once

#include <functional>
#include "FactionMember.h"
#include "Constants.h"
#include "CombatResult.h"
#include "Creature.h"

class Hero : public FactionMember {

public:
	Hero(int _factionId, bool _isReal = true);
	~Hero();

	void* getCreature(int index);

	int stackCount();

	inline bool canMove(int distance);

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