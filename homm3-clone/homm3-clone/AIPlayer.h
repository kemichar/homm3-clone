#pragma once

#include <map>
#include <set>
#include "Player.h"
#include "Pathfinder.h"

class AIPlayer : public Player {

public:

	struct Goal {
		Goal() {
			target = intp(-1, -1);
			type = "";
		}
		Goal(string _type, intp _target, int _actionsThisTurn = 0) :
			type(_type), target(_target), actionsThisTurn(_actionsThisTurn){
		}

		string type;
		intp target;
		int actionsThisTurn;
	};

	AIPlayer(int _factionId);

	void startTurn();

	void calculateTurn();

	void assignGoal(MOHero * heroObject);

	int moveHero(MOHero* heroObject, intp target);

	bool isWinnable(Hero * hero, MapObject * enemy);

	float creaturePower(Creature * creature);

	std::map<int, Goal> heroGoals;

private:
	AIPlayer::Goal getExploreGoal(MOHero * heroObject);

};
