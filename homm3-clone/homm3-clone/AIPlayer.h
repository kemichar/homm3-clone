#pragma once

#include <map>
#include <set>
#include "Player.h"
#include "Pathfinder.h"
#include "MOCastle.h"

/*
	MOVE - we have a location we want to move to
	GHOSTMOVE - similar to MOVE, but we're using the Pathfinder ghost path 
	and consequentially attacking everything on the way
	PICK_UP - move to the targetted item and pick it up
	VISIT - move to the targeted building and interact with it
	FIGHT - fight with all threats found at the target location
	GATHER_ARMY - pick up the reserved troops from the target location (e.g. castle)
*/
enum GoalType {
	WAIT, MOVE, GHOSTMOVE, PICK_UP, VISIT, ATTACK_HERO, GATHER_ARMY
};

class Goal {
	public:
		Goal(GoalType _type, intp _target, Goal* _next = nullptr);
		~Goal();

		Goal* findWithType(GoalType type);
		std::string toString();
		std::string toStringSeq();

		GoalType type;
		intp target;
		Goal* next;
		int actionsThisTurn;
};

class GGatherArmy : public Goal {
	public:
		GGatherArmy(GoalType _type, intp _target, float _powerRequired, Goal* _next = nullptr);
		~GGatherArmy();
		float powerRequired;
		Creature* creatures[HERO_UNIT_SLOTS];
};

class GAttackHero : public Goal {
	public:
		GAttackHero(GoalType _type, intp _target, int _targetId, Goal* _next = nullptr);
		~GAttackHero();
		int targetId;
};

class AIPlayer : public Player {

public:

	AIPlayer(int _factionId);

	void startTurn();

	void calculateTurn();

	void calcCastleTroopAvailability();

	virtual void archiveHero(int index) override;

	float calcCastleTroopAvailability(MOCastle * castle);

	void upgradeCastles();

	void assignGoal(MOHero * heroObject);

	int moveHero(MOHero* heroObject, intp target);

	std::map<int, Goal*> heroGoals;
	std::set<intp> visitedBuildings; // TODO add revisitable buildings; TODO update on disown

private:
	/*
		Decides whether to purchase (and does so if viable) new heroes based on the current gold
		budget and the positions of owned heroes.
	*/
	void decideNewHeroPurchases();
	void purchaseTroops(MOCastle * castle, float powerRequired, GGatherArmy * armyGoal);
	void transferReservedArmy(Hero * heroSource, GGatherArmy * goal, Hero * heroTarget);
	bool targetReserved(intp target);
	std::pair<float, Goal*> constructGoal(Hero* hero, GoalType type, intp target);
	MOCastle * closestCreatureSource(float requiredTroops);
	float missingPowerToReach(Hero * hero, intp location);
	float missingPowerToWin(Hero * hero, Hero * enemy);
	float missingPowerToWin(float power, Hero * enemy);
	Goal* getExploreGoal(MOHero * heroObject);
	int getClosestCastleAirDist(intp location);
	intp getApproachPoint(intp location);
	float totalPower(Hero * hero);
	float creaturePower(Creature * creature);

	std::vector<MOCastle*> castles;
	std::vector<int> availableTroops;
	Currency putAside;
};
