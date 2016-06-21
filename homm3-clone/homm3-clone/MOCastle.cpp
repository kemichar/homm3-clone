#include "MOCastle.h"
#include "Resources.h"
#include "objglm\objglm.h"
#include "GameLogic.h"
#include "Player.h"
#include "Resources.h"
#include "ViewManager.h"
#include "FactionSetup.h"

MOCastle::MOCastle(Map * map, intp _pos, int _factionId)
	: MOBuilding(map, _pos, _factionId){
	hero = new Hero(factionId, false);

	isControllable = true;
	objectType = MOType::CASTLE;
	setModel(Resources::modelData["castle"]);

	goldGainLevel = 0;

	refresh();
}

void MOCastle::interact() {
	int playerFaction = GameLogic::instance().getCurrentPlayer()->getFactionId();

	if (playerFaction != factionId) {
		setControl(playerFaction);
	}

	if (!FactionSetup::instance().isAI[playerFaction]) {
		ViewManager::instance().showBuilding(this->pos);
	}
}

void MOCastle::buyHero() {
	Player* currPlayer = GameLogic::instance().getCurrentPlayer();
	if (getFactionId() != currPlayer->getFactionId()) {
		return;
	}

	if (currPlayer->heroCount() >= MAX_HERO_COUNT) {
		return;
	}

	if (currPlayer->wallet[GOLD] < CASTLE_HERO_BUY_COST) {
		return;
	}

	currPlayer->wallet[GOLD] -= CASTLE_HERO_BUY_COST;
	currPlayer->addHeroObject(new MOHero(this->pos, currPlayer->getFactionId()));
}

void MOCastle::setControl(int _factionId) {
	if (factionId > 0) {
		Player* player = GameLogic::instance().getPlayerByFaction(factionId);
		if (player != nullptr) {
			player->disownBuilding(pos);
		}
	}

	factionId = _factionId;
	Player* player = GameLogic::instance().getPlayerByFaction(_factionId);
	if (player != nullptr) {
		printf("Castle %d %d control passed to faction %d.\n", pos.x, pos.y, _factionId); // DEBUG
		player->buildingsControlled.push_back(pos);
	}
}

bool MOCastle::canBuild(BuildingType type) {
	if (type < 0 || type >= _BUILDINGTYPE_END || allowedDailyBuilds <= 0) {
		return false;
	}

	if (factionId != GameLogic::instance().getCurrentPlayer()->getFactionId()) {
		return false;
	}

	if (hasBuilding(type)) {
		return false;
	}

	if (!canAfford(type)) {
		return false;
	}

	// TEMP check the building requirements
	if (type == BuildingType::HALL) {
		// intentionally empty prerequisite check
		return false;
	}
	else if (type == BuildingType::DWELL_ONE) {
	}
	else if (type == BuildingType::DWELL_TWO) {
		if (!hasBuilding(DWELL_ONE)) {
			return false;
		}
	}
	else if (type == BuildingType::DWELL_THREE) {
		if (!hasBuilding(DWELL_TWO)) {
			return false;
		}
	}
	else if (type == BuildingType::DWELL_FOUR) {
		if (!hasBuilding(DWELL_THREE)) {
			return false;
		}
	}
	else if (type == BuildingType::DWELL_FIVE) {
		if (!hasBuilding(DWELL_FOUR)) {
			return false;
		}
	}
	else if (type == BuildingType::DWELL_SIX) {
		if (!hasBuilding(DWELL_FIVE)) {
			return false;
		}
	}
	else if (type == BuildingType::DWELL_SEVEN) {
		if (!hasBuilding(DWELL_SIX)) {
			return false;
		}
	}

	return true;
}

// TEMP hardcoded TODO make config files for buildings, prerequisites and other properties
BuildStatus MOCastle::build(BuildingType type) {
	if (type < 0 || type >= _BUILDINGTYPE_END || allowedDailyBuilds <= 0) {
		return BuildStatus::OTHER_ERROR;
	}

	if (factionId != GameLogic::instance().getCurrentPlayer()->getFactionId()) {
		return BuildStatus::NOT_ACTIVE_PLAYER;
	}

	if (hasBuilding(type)) {
		return BuildStatus::DUPLICATE;
	}
	
	if (!canAfford(type)) {
		return BuildStatus::MISSING_RES;
	}

	Buildings::Building* building;
	// TEMP check the building requirements
	if (type == BuildingType::HALL) {
		// intentionally empty prerequisite check
		return BuildStatus::OTHER_ERROR; // TEMP
	}
	else if (type == BuildingType::DWELL_ONE) {
		// intentionally empty prerequisite check
		building = new Buildings::Dwelling(Resources::creatureData["Pikeman"]);
	}
	else if (type == BuildingType::DWELL_TWO) {
		if (!hasBuilding(DWELL_ONE)) {
			return BuildStatus::MISSING_REQ;
		}
		building = new Buildings::Dwelling(Resources::creatureData["Archer"]);
	}
	else if (type == BuildingType::DWELL_THREE) {
		if (!hasBuilding(DWELL_TWO)) {
			return BuildStatus::MISSING_REQ;
		}
		building = new Buildings::Dwelling(Resources::creatureData["Griffin"]);
	}
	else if (type == BuildingType::DWELL_FOUR) {
		if (!hasBuilding(DWELL_THREE)) {
			return BuildStatus::MISSING_REQ;
		}
		building = new Buildings::Dwelling(Resources::creatureData["Swordsman"]);
	}
	else if (type == BuildingType::DWELL_FIVE) {
		if (!hasBuilding(DWELL_FOUR)) {
			return BuildStatus::MISSING_REQ;
		}
		building = new Buildings::Dwelling(Resources::creatureData["Monk"]);
	}
	else if (type == BuildingType::DWELL_SIX) {
		if (!hasBuilding(DWELL_FIVE)) {
			return BuildStatus::MISSING_REQ;
		}
		building = new Buildings::Dwelling(Resources::creatureData["Cavalier"]);
	}
	else if (type == BuildingType::DWELL_SEVEN) {
		if (!hasBuilding(DWELL_SIX)) {
			return BuildStatus::MISSING_REQ;
		}
		building = new Buildings::Dwelling(Resources::creatureData["Angel"]);
	}

	if (building == nullptr) {
		printf("Castle buildings: failed to create.\n");
		return BuildStatus::OTHER_ERROR;
	}

	deduceCost(type);
	buildings.push_back(building);

	allowedDailyBuilds--;
	return BuildStatus::OK;
}

bool MOCastle::hasBuilding(BuildingType type) {
	for (Buildings::Building* owned : buildings) {
		if (owned->type == type) {
			return true;
		}
	}
	return false;
}

Buildings::Building* MOCastle::getBuilding(BuildingType type) {
	for (Buildings::Building* owned : buildings) {
		if (owned->type == type) {
			return owned;
		}
	}
	return nullptr;
}

void MOCastle::deduceCost(BuildingType type) {
	Player* currentPlayer = GameLogic::instance().getCurrentPlayer(); // CHECK get by faction?
	for (int i = 0; i < _RESOURCE_END; i++) {
		currentPlayer->wallet[i] -= BUILD_COST[type][i];
	}
}

bool MOCastle::canAfford(BuildingType type) {
	Player* currentPlayer = GameLogic::instance().getCurrentPlayer(); // CHECK get by faction?

	for (int i = 0; i < _RESOURCE_END; i++) {
		if (BUILD_COST[type][i] > currentPlayer->wallet[i]) {
			return false;
		}
	}

	return true;
}

void MOCastle::refresh() {
	allowedDailyBuilds = CASTLE_DAILY_BUILD_LIMIT;
	Player* owningPlayer = GameLogic::instance().getPlayerByFaction(getFactionId());
	if (owningPlayer != nullptr) {
		owningPlayer->wallet[GOLD] += CASTLE_GOLD_GAIN[goldGainLevel];
	}
}

void MOCastle::weeklyRefresh() {
	for (Buildings::Building* building : buildings) {
		if (building->type >= BuildingType::DWELL_ONE && building->type <= BuildingType::DWELL_SEVEN) {
			((Buildings::Dwelling*)building)->weeklyRefresh();
		}
	}
}
