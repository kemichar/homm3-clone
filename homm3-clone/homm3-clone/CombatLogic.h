#pragma once

#include <algorithm>
#include <vector>
#include <queue>
#include "MapObject.h"
#include "Hero.h"
#include "Creature.h"
#include "CombatResult.h"

using namespace std;

class CombatLogic {

public:
	void setupCombat(MapObject* _attackerObject, MapObject* _defenderObject);
	void nextCreature();
	void move(vec2 targetPos, int direction);
	void move(vec2 targetPos, vec2 direction);
	void endCombat(int winnerFaction);

	// TODO move to a separate class
	void simpleCombatAI();

	Creature* getActiveCreature();
	int getActiveFaction();

	// TODO add ranged attack, add flying
	// void attack(vec2 target);

	bool combatActive;
	int roundCount;
	int attackerFaction;
	int defenderFaction;

	int currentCreature;
	vector<vec2> validMoves;

	vector<int> lastActive;
	vector<Creature*> creatures;

	static CombatLogic& instance();
	CombatLogic(CombatLogic const&) = delete;
	void operator = (CombatLogic const &) = delete;

private:
	CombatLogic();

	bool isMoveValid(vec2 moveTarget);

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