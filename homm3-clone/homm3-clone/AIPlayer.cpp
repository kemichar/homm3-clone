#include "AIPlayer.h"
#include <queue>
#include "MOMine.h" // TODO refactor

AIPlayer::AIPlayer(int _factionId) : Player(_factionId) {
	// TODO set main goals?
}

void AIPlayer::startTurn() {
	printf("Adventure: AI turn starting...\n");

	GameLogic::instance().aiThreadStatus = 1;
}

void AIPlayer::calculateTurn() {
	std::queue<MOHero*> availableHeroes;
	for (MOHero* heroObject : heroObjects) {
		availableHeroes.push(heroObject);
		if (heroGoals.find(heroObject->hero->uniqueId) != heroGoals.end()) {
			heroGoals[heroObject->hero->uniqueId].actionsThisTurn = 0;
		}
	}

	GameLogic &gameLogic = GameLogic::instance();
	Map* map = gameLogic.map;

	while (!availableHeroes.empty()) {
		MOHero* heroObject = availableHeroes.front();
		Hero* hero = heroObject->hero;
		availableHeroes.pop();

		printf("	AI using hero %d.\n", hero->uniqueId);

		pf.findPaths(heroObject->pos, hero->movementPoints + HERO_BASE_MOVE_POINTS * 3); // TEMP modify the scan distance
		
		if (heroGoals.find(hero->uniqueId) == heroGoals.end() || heroGoals[hero->uniqueId].target == heroObject->pos) {
			assignGoal(heroObject);
			Goal &tempGoal = heroGoals[hero->uniqueId];
			printf("	AI assigned hero %d goal %s with target (%d, %d).\n",
				hero->uniqueId, tempGoal.type.c_str(), tempGoal.target.x, tempGoal.target.y);
		}

		// if the hero has a goal, lets try to work towards it
		if (heroGoals.find(hero->uniqueId) != heroGoals.end()){
			this_thread::sleep_for(0.5s); // TEMP
			Goal &currGoal = heroGoals[hero->uniqueId];
			intp &currTarget = currGoal.target;

			int moveStatus = moveHero(heroObject, currTarget);

			// check if the hero died after (possibly) interacting with the last tile
			if (heroObject->hero->isDead) {
				continue;
			}
			
			// TEMP allowing 3 actions per turn, might help with heroes blocking eachother etc.
			if (hero->movementPoints > 0 && ++currGoal.actionsThisTurn < 3) {
				availableHeroes.push(heroObject);
			}
		}
	}


	printf("Adventure: AI turn ending...\n");
	gameLogic.aiThreadStatus = 0;
	gameLogic.endTurn();
}

// WARNING update the pathfinder paths before calling this
// CHECK TODO maybe return something? is failing even an option?
void AIPlayer::assignGoal(MOHero* heroObject) {
	Hero* hero = heroObject->hero;
	Map* map = GameLogic::instance().map;

	// TODO modify goal adding etc., currently very simplified
	// TODO add goal queue for goals to be completed in the future
	vector<pair<float, intp>> candidateGoals;
	for (intp endpoint : pf.reachableBuildings) {
		// check if this goal is already assigned to a different hero
		bool taken = false;
		for (auto assignment : heroGoals) {
			if (assignment.second.target == endpoint) {
				taken = true;
				break;
			}
		}
		if (taken) {
			break;
		}

		float priority = 0;
		MapObject* object = map->getObject(endpoint);
		// TODO add other buildings
		if (object->objectType == MINE && object->getFactionId() != getFactionId()) {
			priority = 1;
			if (map->isThreatened(endpoint) && isWinnable(hero, map->getFirstThreat(endpoint))) {
				priority += 0.5f;
				candidateGoals.push_back(make_pair(priority, endpoint));
			}
			else if (!map->isThreatened(endpoint)) {
				candidateGoals.push_back(make_pair(priority, endpoint));
			}
		}
	}
	for (intp endpoint : pf.reachableItems) {
		// check if this goal is already assigned to a different hero
		bool taken = false;
		for (auto assignment : heroGoals) {
			if (assignment.second.target == endpoint) {
				taken = true;
				break;
			}
		}
		if (taken) {
			break;
		}

		float priority = 0;
		MapObject* object = map->getObject(endpoint);
		// TODO add other items
		if (object->objectType == RESOURCE) {
			priority = 2;
			if (map->isThreatened(endpoint) && isWinnable(hero, map->getFirstThreat(endpoint))) {
				priority += 0.5f;
				candidateGoals.push_back(make_pair(priority, endpoint));
			}
			else if (!map->isThreatened(endpoint)) {
				candidateGoals.push_back(make_pair(priority, endpoint));
			}
		}
	}

	sort(candidateGoals.begin(), candidateGoals.end(),
		[this](const pair<float, intp> a, const pair<float, intp> b) -> bool {
		return a.first < b.first;
	});

	if (!candidateGoals.empty()) {
		heroGoals[hero->uniqueId] = Goal("MISC", candidateGoals[0].second);
	}
	else {
		heroGoals[hero->uniqueId] = getExploreGoal(heroObject);
	}
}

// finds the closest point that reveals the most fogged (invisible) tiles
AIPlayer::Goal AIPlayer::getExploreGoal(MOHero* heroObject) {
	vector<intp> allReachable = pf.getReachableTiles(heroObject->hero->movementPoints);
	vector<intp> candidateTiles;

	candidateTiles.reserve(allReachable.size());
	for (intp tile : allReachable) {
		if (pf.countInvisibleAround(tile, 5) > 0) {
			candidateTiles.push_back(tile);
		}
	}
	sort(candidateTiles.begin(), candidateTiles.end(),
		[this](const intp &a, const intp &b) -> bool {
		return pf.distSolid[a.x][a.y] < pf.distSolid[b.x][b.y];
	});

	intp bestTile(-1, -1);
	int invisCount = 0;
	int range = heroObject->hero->movementPoints;
	for (int i = 0; i < candidateTiles.size(); i++) {
		intp &tile = candidateTiles[i];
		if (pf.distSolid[tile.x][tile.y] > range) {
			if (invisCount > 0) {
				break;
			}
			range += HERO_BASE_MOVE_POINTS;
		}
		int count = pf.countInvisibleAround(tile, 5);
		if (count > invisCount) {
			invisCount = count;
			bestTile = tile;
		}
	}

	return AIPlayer::Goal("Explore", bestTile);
}

int AIPlayer::moveHero(MOHero* heroObject, intp target) {
	vector<intp> path = pf.getPathTo(target, true);

	if (path.empty()) {
		return 0; // not reachable
	}
	if (path[0] != heroObject->pos) {
		printf("ERROR: AIPlayer using the wrong hero's calculations for pathfinding.\n");
		return -1; // we're using the wrong hero's calculations
	}

	// find the furthest point on the ghost path we can reach even with creatures / items / etc. in the way
	int furthest = 0;
	for (int i = 1; i < (int)path.size(); i++) {
		intp temp = path[i];
		if (pf.distSolid[temp.x][temp.y] >= 0) {
			furthest = i;
		}
	}

	if (!furthest) {
		return 0; // we can't move closer without fighting
	}

	// find a clear path to the furthest point
	path = pf.getPathTo(path[furthest]);
	// move the hero tile-by-tile
	int i = 1;
	for (; i < (int)path.size() && i <= heroObject->hero->movementPoints; i++) {
		heroObject->moveTo(path[i], 1);
		pf.makeVisibleAround(path[i], 5); // TODO make the scouting radius a hero property
		this_thread::sleep_for(0.2s); // TEMP AI movement animation
	}

	if (i == path.size() && path[path.size() - 1] == target) {
		return 1; // we managed to reach the target (either directly or we took some turns)
	}
	return 2; // partial movement towards the target
}

bool AIPlayer::isWinnable(Hero* hero, MapObject* enemyObject) {
	Hero* enemy = enemyObject->hero;

	// TODO improve calculation
	float allyPower = 0;
	float enemyPower = 0;
	for (Creature* creature : hero->creatures) {
		allyPower += creaturePower(creature);
	}
	for (Creature* creature : enemy->creatures) {
		enemyPower += creaturePower(creature);
	}

	return allyPower > enemyPower * AI_AGGRESSION_FACTOR;
}

float AIPlayer::creaturePower(Creature* creature) {
	if (creature == nullptr) {
		return 0;
	}

	// TODO improve calculation or use config
	return (creature->baseDamageMin + creature->baseDamageMax) / 2.f * creature->count;
}