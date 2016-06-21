#include "CombatLogic.h"
#include <iostream>
#include <glm/glm.hpp>
#include "Utility.h"
#include "FactionSetup.h"
#include "GameLogic.h"
#include "ViewManager.h"

CombatLogic::CombatLogic() {
	combatActive = false;
	creatures.resize(0);
	attackerPotentialReward = 0;
	defenderPotentialReward = 0;
}

CombatLogic& CombatLogic::instance() {
	static CombatLogic COMBATLOGIC_INSTANCE;

	return COMBATLOGIC_INSTANCE;
}


bool CombatLogic::isMoveValid(intp moveTarget) {
	bool valid = false;
	for (intp validPos : validMoves) {
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
	intp start = creatures[currentCreature]->combatPos;
	bio[(int)start.x][(int)start.y] = true;

	std::queue<glm::vec3> bfsq;
	bfsq.push(glm::vec3(start.x, start.y, 0));
	while (!bfsq.empty()) {
		glm::vec3 curr = bfsq.front();
		bfsq.pop();
		validMoves.push_back(vec2(curr.x, curr.y));
		if (curr.z < creatures[currentCreature]->speed) {
			for (int k = 0; k < 8; k++) {
				glm::vec3 next(curr.x + COMBAT_MOVE_DIR[k].x, curr.y + COMBAT_MOVE_DIR[k].y, curr.z + 1);
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

void CombatLogic::setupCombat(MapObject * _attackerObject, MapObject * _defenderObject) {
	ViewManager::instance().isActive[CamId::COMBAT] = true;

	attackerObject = _attackerObject;
	defenderObject = _defenderObject;
	attacker = _attackerObject->hero;
	defender = _defenderObject->hero;
	attackerFaction = attacker->getFactionId();
	defenderFaction = defender->getFactionId();

	// rearrange the troops if the defender is faction 0 (map AI)
	if (defender->getFactionId() == 0) {
		defender->smartArmySetup(attacker);
	}
	// TEMP if attacker is AI rearrange the troops
	if (FactionSetup::instance().isAI[attacker->getFactionId()]) {
		attacker->smartArmySetup(defender);
	}

	// retrieve the creatures involved in combat and set their positions
	creatures.clear();
	attackerStacks = attacker->stackCount();
	int posIndex = 0;
	for (Creature* creature: attacker->creatures) {
		if (creature != nullptr && creature->count > 0) {
			creature->combatPos = COMBAT_POS[attackerStacks - 1][posIndex++];
			creatures.push_back(creature);
			defenderPotentialReward += creature->maxHealth * creature->count;
		}
	}

	// (a new loop to preserve ordering in case of speed ties)
	defenderStacks = defender->stackCount();
	posIndex = 0;
	for (Creature* creature : defender->creatures) {
		if (creature != nullptr && creature->count > 0) {
			creature->combatPos =
				vec2(COMBAT_COLS - COMBAT_POS[defenderStacks - 1][posIndex].x - 1,
					COMBAT_POS[defenderStacks - 1][posIndex].y);
			posIndex++;
			creatures.push_back(creature);
			attackerPotentialReward += creature->maxHealth * creature->count;
		}
	}

	// sort by creature speed (initiative)
	stable_sort(creatures.begin(), creatures.end(),
		[this](const Creature * a, const Creature * b) -> bool {
		return a->speed > b->speed;
	});
	for (int i = 0; i < (int)creatures.size(); i++) {
		creatures[i]->refresh();
	}

	// initialize the creatures' last active turn TODO implement waiting
	lastActive.resize(creatures.size());
	fill(lastActive.begin(), lastActive.end(), -1);

	currentCreature = 0;
	calcValidMoves();
	roundCount = 0;
	combatActive = true;

	// check if someone started the fight without any troops (autoloss)
	if (defenderStacks == 0) {
		endCombat(attackerFaction);
	}
	else if (attackerStacks == 0) {
		endCombat(defenderFaction);
	}
	else {
		std::cout << "Combat: Starting with creature (" << creatures[currentCreature]->combatPos.x << ", " <<
			creatures[currentCreature]->combatPos.y << ") faction " << creatures[currentCreature]->getFactionId() << std::endl;

		if (FactionSetup::instance().isAI[creatures[currentCreature]->getFactionId()]) {
			simpleCombatAI();
		}
		// TEMP thread sleep solution, reconsider if more threads are added
		else if (std::this_thread::get_id() == GameLogic::instance().aiThread.get_id()) {
			threadSleep();
		}
	}
}

void CombatLogic::threadSleep() {
	float sleepTime = 0; // DEBUG not total time
	while (combatActive) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		sleepTime += 0.5f;
		if (sleepTime > 5) {
			sleepTime = 0;
			printf("AI thread waiting for combat to end...\n");
		}
	}
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

	std::cout << "	Next creature (" << creatures[currentCreature]->combatPos.x << ", " <<
		creatures[currentCreature]->combatPos.y << ") f: " << creatures[currentCreature]->getFactionId() << " c: "
		<< creatures[currentCreature]->count << std::endl;

	if (FactionSetup::instance().isAI[creatures[currentCreature]->getFactionId()]) {
		simpleCombatAI();
	}
	// TEMP thread sleep solution
	else if (std::this_thread::get_id() == GameLogic::instance().aiThread.get_id()) {
		threadSleep();
	}
}

void CombatLogic::move(intp targetPos, int direction) {
	move(targetPos, COMBAT_ATTACK_DIR[direction]);
}

void CombatLogic::move(intp targetPos, intp direction) {
	if (!isMoveValid(targetPos)) {
		return;
	}

	creatures[currentCreature]->combatPos = targetPos;
	intp lookAt = targetPos + direction;
	for (int i = 0; i < (int)creatures.size(); i++) {
		if (creatures[i] != nullptr && creatures[i]->count > 0 && creatures[i]->combatPos == lookAt &&
			creatures[i]->getFactionId() != creatures[currentCreature]->getFactionId()) {
			creatures[i]->takeDamageFrom(creatures[currentCreature]);
			std::cout << "	" << creatures[currentCreature]->plural << " attacked "<< creatures[i]->plural << ". " << creatures[i]->count << " survived the attack." << std::endl;

			if (creatures[i]->count <= 0) {
				if (creatures[i]->getFactionId() == attackerFaction) {
					attackerStacks--;
					std::cout << "	Attacker lost a stack, stacks remaining: " << attackerStacks << std::endl;
					if (attackerStacks <= 0) {
						endCombat(defenderFaction);
						return;
					}
				}
				else {
					defenderStacks--;
					std::cout << "	Defender lost a stack, stacks remaining: " << defenderStacks << std::endl;
					if (defenderStacks <= 0) {
						endCombat(attackerFaction);
						return;
					}
				}
			}

			break; // TODO better attack logic, retaliation, etc.
		}
	}

	nextCreature();
}

void CombatLogic::endCombat(int winnerFaction) {
	ViewManager::instance().isActive[CamId::COMBAT] = false;
	std::cout << "	Combat over, faction " << winnerFaction << " won." << std::endl;

	if (winnerFaction == attackerFaction) {
		if (attacker->isReal) {
			if (!FactionSetup::instance().isAI[attacker->getFactionId()]) {
				GameLogic::instance().getCurrentPlayer()->pf.findPaths(attackerObject->pos);
			}
			attacker->experience += attackerPotentialReward; // TODO add reward for hero kills and other non-experience rewards
		}
		if (defender->isReal) {
			defender->isDead = true;
		}
		else {
			GameLogic::instance().map->removeObject(defenderObject->pos); // TEMP TODO change this to map->objectLostFight(pos) etc.
		}
	}
	else {
		if (defender->isReal) {
			defender->experience += attackerPotentialReward; // TEMP TODO change this to map->objectWonFight(pos) etc.
		}
		if (attacker->isReal) {
			attacker->isDead = true;
		}
		else {
			GameLogic::instance().map->removeObject(attackerObject->pos); // this shouldn't ever happen
		}
	}

	combatActive = false;
}

void CombatLogic::simpleCombatAI() {
	Creature* currCreature = getActiveCreature();
	intp currPos = currCreature->combatPos;
	float bestDist = oo;
	int bestMove = 0;
	for (int i = 0; i < (int)creatures.size(); i++) {
		if (creatures[i]->getFactionId() != currCreature->getFactionId()  && creatures[i]->count > 0) {
			intp enemyPos = creatures[i]->combatPos;
			for (int j = 0; j < (int)validMoves.size(); j++) {
				intp nextPos = validMoves[j];

				// TODO ranged
				if (abs(nextPos.x - enemyPos.x) <= 1 + eps && abs(nextPos.y - enemyPos.y) <= 1 + eps) {
					move(validMoves[j], enemyPos - nextPos);
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

bool CombatLogic::isAiActive() {
	return FactionSetup::instance().isAI[getActiveFaction()];
}

/*
CombatLogic& CombatLogic::instance() {
	static CombatLogic COMBATLOGIC_INSTANCE;

	return COMBATLOGIC_INSTANCE;
}
*/