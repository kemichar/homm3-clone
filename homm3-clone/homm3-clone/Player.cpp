#include "Player.h"

Player::Player(int _factionId) : FactionMember(_factionId) {
	selectedHero = -1;
	memset(minesControlled, 0, sizeof minesControlled);
}

void Player::startTurn() {
	ViewManager::showPrompt(string("Player ") + to_string(factionId) + "'s turn.", 2000);
}

void Player::addHeroObject(MOHero* heroObject) {
	heroObject->hero->refresh();
	heroObjects.push_back(heroObject);
	if (heroObjects.size() == 1) {
		selectedHero = 0;
	}
	pf.makeVisibleAround(heroObject->pos, 5);
}

void Player::addNewHero(string name) { // TODO name adding or sth.
	MOHero* heroObject = new MOHero(spawnPosition, factionId);
	addHeroObject(heroObject);
}

MOHero* Player::setNextHero() {
	if (heroObjects.empty()) {
		return nullptr;
	}

	selectedHero = (selectedHero + 1) % heroObjects.size();

	ViewManager::instance().mapCameraLookAt(getCurrentHero()->pos);

	return getCurrentHero();
}

MOHero* Player::setPreviousHero() {
	if (heroObjects.empty()) {
		return nullptr;
	}

	selectedHero = (selectedHero + heroObjects.size() - 1) % heroObjects.size();

	ViewManager::instance().mapCameraLookAt(getCurrentHero()->pos);

	return getCurrentHero();
}

MOHero* Player::setHeroByIndex(int index) {
	if (heroObjects.empty()) {
		return nullptr;
	}

	selectedHero = index % heroObjects.size();

	//ViewManager::instance().mapCameraLookAt(getCurrentHero()->pos);

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

MOHero* Player::getCurrentHero() {
	if (heroObjects.empty()) {
		return nullptr;
	}

	return heroObjects[selectedHero];
}

void Player::archiveHero(int index) {
	if (heroObjects.empty()) {
		return;
	}

	heroHistory.push_back(heroObjects[index]);
	heroObjects.erase(heroObjects.begin() + index);
	if (!heroObjects.empty()) {
		selectedHero %= heroObjects.size();
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
}

void Player::setSpawnPosition(intp _spawnPosition) {
	spawnPosition = _spawnPosition;
}