#include "GameLogic.h"

GameLogic::GameLogic() {
	currentPlayer = 0;
	turnCount = 0;
}

void GameLogic::endTurn() {
	currentPlayer = (currentPlayer + 1) % players.size();
	players[currentPlayer]->refresh();
	turnCount++;
	printf(".. turn %d finished ..\n", turnCount);
}

Player * GameLogic::getCurrentPlayer() {
	return players[currentPlayer];
}

GameLogic& GameLogic::instance() {
	static GameLogic GAMELOGIC_INSTANCE;

	return GAMELOGIC_INSTANCE;
}
