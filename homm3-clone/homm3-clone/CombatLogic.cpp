#include "CombatLogic.h"
#include <iostream>
#include "Utility.h"

CombatLogic::CombatLogic() {
	combatActive = false;
	creatures.resize(0);
}

bool CombatLogic::isMoveValid(vec2 moveTarget) {
	bool valid = false;
	for (vec2 validPos : validMoves) {
		if (validPos == moveTarget) {
			valid = true;
			break;
		}
	}
	return valid;
}

void CombatLogic::calcValidMoves() {
	validMoves.clear();

	bool bio[COMBAT_COLS][COMBAT_ROWS];
	memset(bio, 0, sizeof bio);
	vec2 start = creatures[currentCreature]->combatPos;
	bio[(int)start.x][(int)start.y] = true;

	queue<vec3> bfsq;
	bfsq.push(vec3(start.x, start.y, 0));
	while (!bfsq.empty()) {
		vec3 curr = bfsq.front();
		bfsq.pop();
		validMoves.push_back(vec2(curr.x, curr.y));
		if (curr.z < creatures[currentCreature]->moveRange) {
			for (int k = 0; k < 8; k++) {
				vec3 next(curr.x + COMBAT_MOVE_DIR[k].x, curr.y + COMBAT_MOVE_DIR[k].y, curr.z + 1);
				if (next.x < 0 || next.x >= COMBAT_COLS || next.y < 0 ||
					next.y >= COMBAT_ROWS || bio[(int)next.x][(int)next.y]) {
					continue;
				}
				bool blocked = false;
				for (Creature* creature : creatures) {
					if (creature != nullptr && creature->count > 0 && creature->combatPos == vec2(next.x, next.y)) {
						blocked = true;
						break;
					}
				}
				if (blocked) {
					continue;
				}

				bio[(int)next.x][(int)next.y] = true;
				bfsq.push(next);
			}
		}
	}
}

void CombatLogic::setupCombat(MapObject * _attacker, MapObject * _defender) {
	attacker = _attacker;
	defender = _defender;

	creatures.clear();

	// retrieve the creatures involved in combat
	attackerStacks = attacker->creatureCount();
	for (int i = 0, j = 0; i < HERO_UNIT_SLOTS; i++) {
		if (attacker->creatures[i] != nullptr && attacker->creatures[i]->count > 0) {
			attacker->creatures[i]->combatPos = COMBAT_POS[attackerStacks - 1][j++];
			creatures.push_back(attacker->creatures[i]);
			attackerFaction = attacker->creatures[i]->getFactionId();
		}
	}

	// (a new loop to preserve ordering in case of speed ties)
	defenderStacks = defender->creatureCount();
	for (int i = 0, j = 0; i < HERO_UNIT_SLOTS; i++) {
		if (defender->creatures[i] != nullptr && defender->creatures[i]->count > 0) {
			defender->creatures[i]->combatPos =
				vec2(COMBAT_COLS - COMBAT_POS[defenderStacks - 1][j].x - 1,
					COMBAT_POS[defenderStacks - 1][j].y);
			j++;
			creatures.push_back(defender->creatures[i]);
			defenderFaction = defender->creatures[i]->getFactionId();
		}
	}

	// sort by creature speed (initiative)
	stable_sort(creatures.begin(), creatures.end());
	reverse(creatures.begin(), creatures.end());
	for (int i = 0; i < (int)creatures.size(); i++) {
		creatures[i]->refresh();
	}

	// initialize the creatures' last active turn
	lastActive.resize(creatures.size());
	fill(lastActive.begin(), lastActive.end(), -1);

	currentCreature = 0;
	calcValidMoves();
	roundCount = 0;
	combatActive = true;
}

void CombatLogic::nextCreature() {
	do {
		currentCreature++;
	} while (currentCreature < (int)creatures.size() && (creatures[currentCreature] == nullptr || creatures[currentCreature]->count == 0));
	if (currentCreature == creatures.size()) {
		roundCount++;
		currentCreature = 0;
		while (currentCreature < (int)creatures.size() && (creatures[currentCreature] == nullptr || creatures[currentCreature]->count == 0)) {
			currentCreature++;
		}
	}

	lastActive[currentCreature] = roundCount;
	calcValidMoves();

	// TODO change (homm3-clone.cpp too) AI faction?
	if (creatures[currentCreature]->getFactionId() == 0) {
		simpleCombatAI();
	}
}

void CombatLogic::move(vec2 targetPos, int direction) {
	move(targetPos, COMBAT_ATTACK_DIR[direction]);
}

void CombatLogic::move(vec2 targetPos, vec2 direction) {
	if (!isMoveValid(targetPos)) {
		return;
	}

	creatures[currentCreature]->combatPos = targetPos;
	vec2 lookAt = targetPos + direction;
	for (int i = 0; i < (int)creatures.size(); i++) {
		if (creatures[i] != nullptr && creatures[i]->count > 0 && creatures[i]->combatPos == lookAt &&
			creatures[i]->getFactionId() != creatures[currentCreature]->getFactionId()) {
			creatures[i]->takeDamageFrom(creatures[currentCreature]);
			cout << "Attacked a stack, " << creatures[i]->count << " remaining: " << endl;

			if (creatures[i]->count <= 0) {
				if (creatures[i]->getFactionId() == attackerFaction) {
					attackerStacks--;
					cout << "Left stacks: " << attackerStacks << endl;
					if (attackerStacks <= 0) {
						endCombat(defenderFaction);
						return;
					}
				}
				else {
					defenderStacks--;
					cout << "Right stacks: " << defenderStacks << endl;
					if (defenderStacks <= 0) {
						endCombat(attackerFaction);
						return;
					}
				}
			}

			break; //
		}
	}

	nextCreature();
}

void CombatLogic::endCombat(int winnerFaction) {
	combatActive = false;

	CombatResult result(attackerFaction, defenderFaction, winnerFaction, 0); // TODO experience
	attacker->onCombatEnd(result);
	defender->onCombatEnd(result);
}

void CombatLogic::simpleCombatAI() {
	printf("AI's combat turn\n");

	vec2 currPos = getActiveCreature()->combatPos;
	float bestDist = oo;
	int bestMove = 0;
	for (int i = 0; i < (int)creatures.size(); i++) {
		// TODO change AI faction id
		if (creatures[i]->getFactionId() > 0 && creatures[i]->count > 0) {
			vec2 enemyPos = creatures[i]->combatPos;
			for (int j = 0; j < (int)validMoves.size(); j++) {
				vec2 nextPos = validMoves[j];

				// TODO ranged
				if (abs(nextPos.x - enemyPos.x) <= 1 + eps && abs(nextPos.y - enemyPos.y) <= 1 + eps) {
					move(validMoves[j], enemyPos - nextPos);
					printf("Moving and attacking %f %f\n", (enemyPos - nextPos).x, (enemyPos - nextPos).y);
					return;
				}
				else {
					float manDist = manhattan(nextPos, enemyPos);
					if (manDist < bestDist) {
						bestMove = j;
						bestDist = manDist;
					}
				}
			}
		}
	}

	move(validMoves[bestMove], 0);
}

Creature * CombatLogic::getActiveCreature() {
	return creatures[currentCreature];
}

int CombatLogic::getActiveFaction() {
	return creatures[currentCreature]->getFactionId();
}

/*
CombatLogic& CombatLogic::instance() {
	static CombatLogic COMBATLOGIC_INSTANCE;

	return COMBATLOGIC_INSTANCE;
}
*/