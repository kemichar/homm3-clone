#pragma once

#include "MOBuilding.h"
#include "Map.h"
#include "Player.h"
#include "Buildings.h"

enum BuildStatus {
	OK, MISSING_RES, MISSING_REQ, DUPLICATE, NOT_ACTIVE_PLAYER, OTHER_ERROR
};

class MOCastle : public MOBuilding {

public:

	MOCastle(Map* map, intp _pos, int _factionId = 0);

	virtual void interact() override;
	
	virtual void setControl(int _factionId) override;

	bool canBuild(BuildingType type);

	void buyHero();

	BuildStatus build(BuildingType type);

	bool hasBuilding(BuildingType type);

	Buildings::Building* getBuilding(BuildingType type);

	void deduceCost(BuildingType type);

	void refresh();

	void weeklyRefresh();

	std::vector<Buildings::Building*> buildings;

	int goldGainLevel;

private:
	bool canAfford(BuildingType type);

	int allowedDailyBuilds;

};