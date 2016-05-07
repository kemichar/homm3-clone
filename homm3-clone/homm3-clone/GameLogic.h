#pragma once

#include "Player.h"
#include "Map.h"

class GameLogic {

public:
	void endTurn();
	void quitGame() {
		exit(0); // TODO modify ofc; temp
	}

	Player* getCurrentPlayer();

	vector<Player*> players;
	int currentPlayer;
	int turnCount;

	Map* map;

	static GameLogic& instance();
	GameLogic(GameLogic const&) = delete;
	void operator = (GameLogic const &) = delete;
private:
	GameLogic();

};