#pragma once

class CombatResult {
public:
	CombatResult(int _attackerFaction, int _defenderFaction, int _winnerFaction, int _experience)
		: attackerFaction(_attackerFaction), defenderFaction(_defenderFaction), winnerFaction(_winnerFaction), experience(_experience) {
	}

	int attackerFaction;
	int defenderFaction;
	int winnerFaction;
	int experience;
};