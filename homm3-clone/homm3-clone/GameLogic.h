#pragma once

#include <thread>
#include "Utility.h"
#include "Player.h"
#include "Map.h"
#include "MOHero.h"

class GameLogic {

public:
	void endTurn();
	void quitGame();

	Player* getCurrentPlayer();
	Player* getPlayerByIndex(int index);
	Player* addPlayer(bool isBot);
	Player* getPlayerByFaction(int factionId);
	void interact(MOHero* heroObject);
	bool tileHasHero(intp tileLocation);

	std::vector<Player*> players;
	int currentPlayer;
	int turnCount;

	Map* map;
	int rowCount;
	int colCount;
	float mapUnit;

	void aiThreadJob();
	int aiThreadStatus; // -1 kill, 0 inactive, 1 active
	std::thread aiThread;

	static GameLogic& instance();
	GameLogic(GameLogic const&) = delete;
	void operator = (GameLogic const &) = delete;
private:
	GameLogic();

};