#include "Player.h"
#include "GameLogic.h"
#include "MOCastle.h"
#include "ViewManager.h"

Player::Player(int _factionId) : FactionMember(_factionId) {
	selectedHero = -1;
	selectedCastle = intp(-1, -1);
	memset(minesControlled, 0, sizeof minesControlled);
}

void Player::startTurn() {
	//ViewManager::showPrompt(string("Player ") + to_string(factionId) + "'s turn.", 2000);
}

void Player::addHeroObject(MOHero* heroObject) {
	heroObject->hero->refresh();
	heroObjects.push_back(heroObject);
	pf.makeVisibleAround(heroObject->pos, 5);
	if (heroObjects.size() == 1) {
		selectedHero = 0;
		pf.findPaths(getCurrentHero()->pos);
	}
}

void Player::addNewHero(std::string name) { // TODO name adding or sth.
	MOHero* heroObject = new MOHero(spawnPosition, factionId);
	addHeroObject(heroObject);
}

int Player::heroCount() {
	int ret = 0;
	for (MOHero* heroObject : heroObjects) {
		if (heroObject != nullptr && heroObject->hero != nullptr
			&& !heroObject->hero->isDead && heroObject->hero->isReal) {
			ret++;
		}
	}
	return ret;
}

MOHero* Player::setNextHero() {
	if (heroObjects.empty()) {
		return nullptr;
	}

	selectedHero = (selectedHero + 1) % heroObjects.size();

	ViewManager::instance().mapCameraLookAt(getCurrentHero()->pos);
	pf.findPaths(getCurrentHero()->pos);

	return getCurrentHero();
}

MOHero* Player::setPreviousHero() {
	if (heroObjects.empty()) {
		return nullptr;
	}

	selectedHero = (selectedHero + heroObjects.size() - 1) % heroObjects.size();

	ViewManager::instance().mapCameraLookAt(getCurrentHero()->pos);
	pf.findPaths(getCurrentHero()->pos);

	return getCurrentHero();
}

MOHero* Player::setHeroByIndex(int index) {
	if (heroObjects.empty()) {
		return nullptr;
	}

	selectedHero = index % heroObjects.size();

	ViewManager::instance().mapCameraLookAt(getCurrentHero()->pos);
	pf.findPaths(getCurrentHero()->pos);

	return getCurrentHero();
}

MOHero* Player::setHeroByUniqueId(int uniqueId) {
	if (heroObjects.empty()) {
		return nullptr;
	}

	for (int i = 0; i < (int)heroObjects.size(); i++) {
		if (heroObjects[i]->hero->uniqueId == uniqueId) {
			return setHeroByIndex(i);
		}
	}

	return nullptr;
}

MOHero* Player::getHeroByUniqueId(int uniqueId) {
	if (heroObjects.empty()) {
		return nullptr;
	}

	for (int i = 0; i < (int)heroObjects.size(); i++) {
		if (heroObjects[i]->hero->uniqueId == uniqueId) {
			return heroObjects[i];
		}
	}

	return nullptr;
}


MOHero* Player::getCurrentHero() {
	try {
		return heroObjects[selectedHero];
	}
	catch (...) {
		return nullptr;
	}
}

void Player::archiveHero(int index) {
	if (heroObjects.empty()) {
		return;
	}

	heroHistory.push_back(heroObjects[index]);
	heroObjects.erase(heroObjects.begin() + index);
	if (!heroObjects.empty()) {
		selectedHero %= heroObjects.size();
		pf.findPaths(getCurrentHero()->pos);
	}
	else {
		selectedHero = -1;
	}
}

int Player::getCurrentHeroIndex() {
	return selectedHero;
}

void Player::refresh() {
	for (MOHero* heroObject : heroObjects) {
		heroObject->hero->refresh();
	}
	for (int resource = 0; resource < _RESOURCE_END; resource++) {
		wallet[resource] += RES_MINE_GAIN[resource] * minesControlled[resource];
	}
	Map* map = GameLogic::instance().map;
	for (intp location : buildingsControlled) {
		MapObject* building = map->getObject(location);
		if (building->objectType == MOType::CASTLE) {
			// TEMP the building refreshing is going to be moved to MOBuilding?
			((MOCastle*)building)->refresh();
		}
	}
}

void Player::setSpawnPosition(intp _spawnPosition) {
	spawnPosition = _spawnPosition;
}

void Player::disownBuilding(intp location) {
	for (int i = 0; i < (int)buildingsControlled.size(); i++) {
		if (buildingsControlled[i] == location) {
			buildingsControlled[i] = intp(-1, -1); // will be cleaned up by the idle function
			if (location == selectedCastle) {
				selectedCastle = intp(-1, -1);
				selectNextCastle();
			}
		}
	}
}

void Player::selectCastle(intp location) {
	GameLogic &gameLogic = GameLogic::instance();

	for (int i = 0; i < (int)buildingsControlled.size(); i++) {
		if (buildingsControlled[i] == location) {
			MapObject* object = gameLogic.map->getObject(location);
			if (object != nullptr && object->objectType == MOType::CASTLE) {
				selectedCastle = location;
			}
			return;
		}
	}
}

intp Player::getSelectedCastle() {
	// TEMP TODO move this to automatically happen on castle add or sth.
	if (selectedCastle == intp(-1, -1)) {
		selectNextCastle();
	}

	return selectedCastle;
}

void Player::selectNextCastle() {
	Map* map = GameLogic::instance().map;

	if (selectedCastle == intp(-1, -1)) {
		for (int i = 0; i < (int)buildingsControlled.size(); i++) {
			MapObject* object = map->getObject(buildingsControlled[i]);
			if (object != nullptr && object->objectType == MOType::CASTLE) {
				selectedCastle = buildingsControlled[i];
				return;
			}
		}
	}
	else {
		int currSelectedIndex = 0;
		for (int i = 0; i < (int)buildingsControlled.size(); i++) {
			if (buildingsControlled[i] == selectedCastle) {
				currSelectedIndex = i;
				break;
			}
		}
		for (int i = (currSelectedIndex + 1) % (int)buildingsControlled.size(); i != currSelectedIndex; i = (i + 1) % (int)buildingsControlled.size()) {
			MapObject* object = map->getObject(buildingsControlled[i]);
			if (object != nullptr && object->objectType == MOType::CASTLE) {
				selectedCastle = buildingsControlled[i];
				return;
			}
		}
	}
}

void Player::selectPreviousCastle() {
	Map* map = GameLogic::instance().map;

	if (selectedCastle == intp(-1, -1)) {
		for (int i = 0; i < (int)buildingsControlled.size(); i++) {
			MapObject* object = map->getObject(buildingsControlled[i]);
			if (object != nullptr && object->objectType == MOType::CASTLE) {
				selectedCastle = buildingsControlled[i];
				return;
			}
		}
	}
	else {
		int currSelectedIndex = 0;
		for (int i = 0; i < (int)buildingsControlled.size(); i++) {
			if (buildingsControlled[i] == selectedCastle) {
				currSelectedIndex = i;
				break;
			}
		}
		for (int i = (currSelectedIndex - 1 + (int)buildingsControlled.size()) % (int)buildingsControlled.size();
			i != currSelectedIndex; i = (i - 1 + (int)buildingsControlled.size()) % (int)buildingsControlled.size()) {
			MapObject* object = map->getObject(buildingsControlled[i]);
			if (object != nullptr && object->objectType == MOType::CASTLE) {
				selectedCastle = buildingsControlled[i];
				return;
			}
		}
	}
}
