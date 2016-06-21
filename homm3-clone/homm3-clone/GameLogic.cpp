#include "GameLogic.h"
#include "CombatLogic.h"
#include "ViewManager.h"
#include "AIPlayer.h"
#include "FactionSetup.h"

GameLogic::GameLogic() {
	currentPlayer = 0;
	turnCount = 0;
	dayCount = 0;

	rowCount = 100;
	colCount = 100;

	aiThreadStatus = 0;
	gameOver = false;
}

void GameLogic::aiThreadJob() {
	while(!gameOver){
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		if (aiThreadStatus == 1) {
			printf("AI thread waking up.\n");
			((AIPlayer*)getCurrentPlayer())->calculateTurn();
			printf("AI thread back to sleep...\n");
		}
	}
}

void GameLogic::startGame() {
	if (players.empty()) {
		printf("ERROR GameLogic.cpp .. can't start the game - no players ..\n");
		return;
	}

	printf(".. game starting ..\n");

	players[currentPlayer]->refresh();

	MOHero* currentHeroObject = players[currentPlayer]->getCurrentHero();
	if (currentHeroObject != nullptr) {
		ViewManager::instance().mapCameraLookAt(currentHeroObject->pos);
	}

	players[currentPlayer]->startTurn();
}

void GameLogic::endTurn() {
	if (aiThreadStatus != 0 && std::this_thread::get_id() != aiThread.get_id()) {
		printf("WARNING GameLogic.cpp .. player tried ending the AI's turn ..\n");
		return;
	}

	// archive dead heroes
	for (Player* player : players) {
		for (int i = 0; i < (int)player->heroObjects.size(); ) {
			if (player->heroObjects[i]->hero->isDead) {
				player->archiveHero(i);
			}
			else {
				i++;
			}
		}
	}

	// update building ownership
	for (Player* player : players) {
		for (int i = 0; i < (int)player->buildingsControlled.size(); ) {
			if (map->getObject(player->buildingsControlled[i]) == nullptr) {
				player->buildingsControlled.erase(player->buildingsControlled.begin() + i);
			}
			else {
				i++;
			}
		}
	}

	turnCount++;
	printf(".. turn %d finished ..\n", turnCount);

	currentPlayer = (currentPlayer + 1) % players.size();
	if (currentPlayer == 0) {
		dayCount++;
		printf(".. day %d finished ..\n", dayCount);
		// weekly refresh is on Mondays
		if (dayCount % 7 == 0) {
			weeklyRefresh();
		}
	}
	players[currentPlayer]->refresh();
	
	MOHero* currentHeroObject = players[currentPlayer]->getCurrentHero();
	if (currentHeroObject != nullptr) {
		ViewManager::instance().mapCameraLookAt(currentHeroObject->pos);
	}
	
	players[currentPlayer]->startTurn();
}

void GameLogic::weeklyRefresh() {
	// castle (dwelling) refresh
	for (int i = 0; i < colCount; i++) {
		for (int j = 0; j < rowCount; j++) {
			MapObject* object = map->getObject(intp(i, j));
			if (object->objectType == MOType::CASTLE) {
				// TODO probably move building (castle) weekly refresh to MOBuilding or sth.
				((MOCastle*)object)->weeklyRefresh();
			}
		}
	}

	// TODO add other buildings
}

void GameLogic::quitGame() {
	gameOver = true;
	aiThread.join();

	exit(0); // TEMP
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

	// IMPORTANT the next 2 lines have to be in this order because the second one uses getPlayerByFaction
	players.push_back(player);
	player->setSpawnPosition(map->factionStartingZone(player->getFactionId()));
	// TEMP TODO move this to constants or something
	player->wallet[ORE] = 5;
	player->wallet[WOOD] = 5;
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

bool GameLogic::isAiActive() {
	return FactionSetup::instance().isAI[getCurrentPlayer()->getFactionId()];
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
			if (heroObject != nullptr && heroObject->pos == tileLocation && !heroObject->hero->isDead) {
				return true;
			}
		}
	}

	return false;
}

MOHero* GameLogic::getHeroAt(intp tileLocation) {
	for (Player* player : players) {
		for (MOHero* heroObject : player->heroObjects) {
			if (heroObject != nullptr && heroObject->pos == tileLocation && !heroObject->hero->isDead) {
				return heroObject;
			}
		}
	}

	return nullptr;
}

MOHero * GameLogic::getHeroByUniqueId(int heroId) {
	for (Player* player : players) {
		MOHero* heroObject = player->getHeroByUniqueId(heroId);
		if (heroObject != nullptr) {
			return heroObject;
		}
	}
	return nullptr;
}

GameLogic& GameLogic::instance() {
	static GameLogic GAMELOGIC_INSTANCE;

	return GAMELOGIC_INSTANCE;
}
