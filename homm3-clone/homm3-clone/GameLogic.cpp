#include "GameLogic.h"
#include "CombatLogic.h"
#include "ViewManager.h"
#include "AIPlayer.h"
#include "FactionSetup.h"

GameLogic::GameLogic() {
	currentPlayer = 0;
	turnCount = 0;
	rowCount = 100;
	colCount = 100;

	aiThreadStatus = 0;
}

void GameLogic::aiThreadJob() {
	while(aiThreadStatus != -1){
		this_thread::sleep_for(1s);
		if (aiThreadStatus == 1) {
			printf("AI thread waking up.\n");
			((AIPlayer*)getCurrentPlayer())->calculateTurn();
			printf("AI thread back to sleep...\n");
		}
	}
}

void GameLogic::endTurn() {
	printf(".. turn %d finished ..\n", turnCount++);

	currentPlayer = (currentPlayer + 1) % players.size();
	players[currentPlayer]->refresh();
	
	MOHero* currentHeroObject = players[currentPlayer]->getCurrentHero();
	if (currentHeroObject != nullptr) {
	//	ViewManager::instance().mapCameraLookAt(currentHero->pos);
	}
	
	players[currentPlayer]->startTurn();
}

void GameLogic::quitGame() {
	aiThreadStatus = -1;
	aiThread.join();

	exit(0); // TODO modify ofc; temp
}

Player * GameLogic::getCurrentPlayer() {
	return players[currentPlayer];
}

Player * GameLogic::getPlayerByIndex(int index) {
	if (index < 0 || index >= (int)players.size()) {
		return nullptr;
	}

	return players[index];
}

Player * GameLogic::addPlayer(bool isBot) {
	Player* player;
	
	if (!isBot) {
		player = new Player(FactionSetup::instance().newFactionID());
	}
	else {
		player = new AIPlayer(FactionSetup::instance().newFactionID(true));
	}

	player->setSpawnPosition(map->factionStartingZone(player->getFactionId()));
	players.push_back(player);
	return player;
}

Player * GameLogic::getPlayerByFaction(int factionId) {
	for (Player* player : players) {
		if (player->getFactionId() == factionId) {
			return player;
		}
	}

	return nullptr;
}

void GameLogic::interact(MOHero* heroObject) {
	if (map->isThreatened(heroObject->pos)) {
		MapObject* threat = map->getFirstThreat(heroObject->pos);
		CombatLogic::instance().setupCombat(heroObject, threat);
	}
	else {
		map->getObject(heroObject->pos)->interact();
	}
}

bool GameLogic::tileHasHero(intp tileLocation) {
	for (Player* player : players) {
		for (MOHero* heroObject : player->heroObjects) {
			if (heroObject->pos == tileLocation) {
				return true;
			}
		}
	}

	return false;
}

GameLogic& GameLogic::instance() {
	static GameLogic GAMELOGIC_INSTANCE;

	return GAMELOGIC_INSTANCE;
}
