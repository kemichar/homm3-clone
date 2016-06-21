#pragma once

#include <vector>
#include "MOHero.h"
#include "Currency.h"
#include "Pathfinder.h"

/*
	A storage class for owned heroes, resources and mines of a single player of the game.
	Handles hero selection, periodic resource gains and other turn-related updates.
*/
class Player : public FactionMember{

public:
	
	Player(int _factionId);

	virtual void startTurn();

//	virtual void onCombatEnd(MOHero* hero, CombatResult result);

	// adds the hero object to this player's owned heroes
	void addHeroObject(MOHero* hero);
	// creates and adds a new hero to this player's owned heroes
	void addNewHero(std::string name = ""); // TODO name adding or sth.
	int heroCount();
	// selects the next hero in the list; used for scrollable display lists of heroes
	MOHero* setNextHero();
	// selects the previous hero in the list; used for scrollable display lists of heroes
	MOHero* setPreviousHero();
	// selects the hero at the provided index; if index is out of range takes it modulo hero count
	MOHero* setHeroByIndex(int index);
	MOHero * setHeroByUniqueId(int uniqueId);
	MOHero * getHeroByUniqueId(int uniqueId);
	// returns the currently selected hero
	MOHero* getCurrentHero();
	// removes the currently selected hero from the list of active heroes
	virtual void archiveHero(int index);
	int getCurrentHeroIndex();
	// used at turn start to refresh the owned heroes' properties (such as movement points)
	void refresh();
	// sets the default map spawn location of the player's heroes
	void setSpawnPosition(intp _spawnPosition);
	// removes the location from the list of owned buildings
	void disownBuilding(intp location);
	
	void selectCastle(intp location);

	intp getSelectedCastle();

	void selectNextCastle();

	void selectPreviousCastle();

	// the list of owned hero objects
	std::vector<MOHero*> heroObjects;
	/*
		The player's owned currencies; for details see Currency; currencies are updated every turn
		(depending on the number of controlled mines) through refresh().
	*/
	Currency wallet;
	// the counts of controlled mine objects, one for each resource type
	int minesControlled[_RESOURCE_END];
	std::vector<intp> buildingsControlled;

	Pathfinder pf;

protected:
	// the history of dead / deleted heroes
	std::vector<MOHero*> heroHistory;
	// the currently active hero (only relevant if the player is active)
	int selectedHero;
	// the default map spawn location of the player's heroes
	intp spawnPosition;
	// the last activated castle (useful only for non-AI players)
	intp selectedCastle;
};