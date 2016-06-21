#pragma once

#include <algorithm>
#include <vector>
#include <queue>
#include "MapObject.h"
#include "Hero.h"
#include "Creature.h"

class CombatLogic {

public:
	// TEMP
	void threadSleep();

	void setupCombat(MapObject* _attackerObject, MapObject* _defenderObject);
	void nextCreature();
	void move(intp targetPos, int direction);
	void move(intp targetPos, intp direction);
	void endCombat(int winnerFaction);

	// TODO move to a separate class
	void simpleCombatAI();

	Creature* getActiveCreature();
	int getActiveFaction();
	bool isAiActive();

	// TODO add ranged attack, add flying
	// void attack(vec2 target);

	bool combatActive;
	int roundCount;
	int attackerFaction;
	int defenderFaction;

	int currentCreature;
	std::vector<intp> validMoves;

	std::vector<int> lastActive;
	std::vector<Creature*> creatures;

	static CombatLogic& instance();
	CombatLogic(CombatLogic const&) = delete;
	void operator = (CombatLogic const &) = delete;

private:
	CombatLogic();

	bool isMoveValid(intp moveTarget);

	void calcValidMoves();

	MapObject* attackerObject;
	MapObject* defenderObject;
	Hero* attacker;
	Hero* defender;
	int attackerStacks;
	int defenderStacks;
	int attackerPotentialReward;
	int defenderPotentialReward;
};