#pragma once

#include <algorithm>
#include <vector>
#include <queue>
#include "MapObject.h"
#include "Creature.h"
#include "CombatResult.h"

using namespace std;

class CombatLogic {

public:
	CombatLogic();

	void setupCombat(MapObject* _attacker, MapObject* _defender);
	void nextCreature();
	void move(vec2 targetPos, int direction);
	void move(vec2 targetPos, vec2 direction);
	void endCombat(int winnerFaction);

	// TODO move to a separate class
	void simpleCombatAI();

	Creature* getActiveCreature();
	int getActiveFaction();

	// TODO add ranged attack
	// void attack(vec2 target);

	bool combatActive;
	int roundCount;

	MapObject* attacker;
	MapObject* defender;

	int attackerFaction;
	int defenderFaction;

	int attackerStacks;
	int defenderStacks;

	int currentCreature;
	vector<vec2> validMoves;

	vector<int> lastActive;
	vector<Creature*> creatures;
/*
	static CombatLogic& instance();
	CombatLogic(CombatLogic const&) = delete;
	void operator = (CombatLogic const &) = delete;

private:
	CombatLogic();*/
private:

	bool isMoveValid(vec2 moveTarget);

	void calcValidMoves();
};