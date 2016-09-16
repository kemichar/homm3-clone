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

/*
	The goal structure containing the description of a goal or a sequence
	of goals. Subsequent goals are stored as a linked list over <code>Goal* next</code>.
	The main properties of a goal are <code>type</code> and <code>target</code>,
	denoting the type of action to perform at the target location, respectively.
*/
class Goal {
	public:
		Goal(GoalType _type, intp _target, Goal* _next = nullptr);
		~Goal();

		/*
			Attempts to find a goal with GoalType equal to <paramref name="type"/>
			in this sequence. <returns>Returns the subgoal with the corresponding
			type if found, nullptr otherwise.</returns>
		*/
		Goal* findWithType(GoalType type);
		/*
			<returns>Returns a string representation of the current subgoal.</returns>
		*/
		std::string toString();
		/*
			<returns>Returns a string representation of this entire goal sequence.</returns>
		*/
		std::string toStringSeq();

		GoalType type;
		intp target;
		Goal* next;
		/*
			Denotes the number of actions (movement or other) used this turn on this subgoal.
			Serves to prevent repetition of failed actions too many times, e.g. movement was
			blocked, try again next turn.
		*/
		int actionsThisTurn;
};

/*
	Serves as a description of a creature army strong enough to complete the target action on some
	subsequent goal.
*/
class GGatherArmy : public Goal {
	public:
		GGatherArmy(GoalType _type, intp _target, float _powerRequired, Goal* _next = nullptr);
		~GGatherArmy();
		float powerRequired;
		Creature* creatures[HERO_UNIT_SLOTS];
};

/*
	Represents a goal with a mobile target, i.e. hero (since there are currently no other mobile targets).
	The game is turn-based, therefore the target location stays valid throughout the AI turn unless the
	target hero dies.
*/
class GAttackHero : public Goal {
	public:
		GAttackHero(GoalType _type, intp _target, int _targetId, Goal* _next = nullptr);
		~GAttackHero();
		int targetId;
};

class AIPlayer : public Player {

public:

	AIPlayer(int _factionId);

	/*
		Wakes up the AI thread to calculate the current AI turn.
	*/
	void startTurn();

	/*
		Calculates the visibility and reachability information using a <code>Pathfinder</code>
		instance. For each hero - either tries to work towards an existing hero goal or assigns
		a new goal.
		Additionaly, tries to upgrade controlled castles and buy new heroes under some preset rules.
	*/
	void calculateTurn();

	/*
		Calculates the amount of purchasable troops in <paramref name=castle/>. Depends on the
		weekly troop capacities for the castle and the current budget of the player.
	*/
	float calcCastleTroopAvailability(MOCastle * castle);

	/*
		Calculates the amount of purchasable troops for each controlled castle.
	*/
	void calcCastleTroopAvailability();

	virtual void archiveHero(int index) override;

	/*
		Attempt to build upgrades in all controlled castles.
	*/
	void upgradeCastles();
	/*
		Creates a list of potential goal sequences, sorts them by priority (distance-related)
		and assigns the highest priority goal sequence.
	*/
	void assignGoal(MOHero * heroObject);

	/*
		Attempts to move the <paramref name="heroObject"/> to the provided <paramref name="target"/>.
		If full movement is not possible, will move partially towards the target.
	*/
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
