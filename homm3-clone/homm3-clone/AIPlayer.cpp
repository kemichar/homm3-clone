#include "AIPlayer.h"
#include <queue>
#include "MOMine.h" // TODO refactor
#include "MOCastle.h"
#include "FactionSetup.h"

AIPlayer::AIPlayer(int _factionId) : Player(_factionId) {
	// TODO set main goals?
}

void AIPlayer::startTurn() {
	printf("Adventure: AI turn starting...\n");

	GameLogic::instance().aiThreadStatus = 1;
}

// TODO periodically add goals?
void AIPlayer::calculateTurn() {
	GameLogic &gameLogic = GameLogic::instance();
	Map* map = gameLogic.map;

	// add the available castles (TODO and external dwellings)
	castles.clear();
	availableTroops.clear();
	for (intp worldBuildingPos : buildingsControlled) {
		MapObject* object = map->getObject(worldBuildingPos);
		if (object == nullptr) {
			continue;
		}

		if (object->objectType == MOType::CASTLE) {
			castles.push_back((MOCastle*)object);
			visitedBuildings.insert(object->pos); // TEMP the first AI turn will not have the castles "visited" otherwise
		}
	}
	calcCastleTroopAvailability();

	std::cout << "\nAI available castle recruits:" << std::endl;
	for (int i = 0; i < (int)castles.size(); i++) {
		printf("	%d (%d, %d) -> %d power\n", i, castles[i]->pos.x, castles[i]->pos.y, availableTroops[i]);
	}
	std::cout << std::endl;

	upgradeCastles();

	// assumes castles are stored beforehand
	decideNewHeroPurchases();

	std::queue<MOHero*> availableHeroes;
	for (MOHero* heroObject : heroObjects) {
		availableHeroes.push(heroObject);
		if (heroGoals.find(heroObject->hero->uniqueId) != heroGoals.end()) {
			heroGoals[heroObject->hero->uniqueId]->actionsThisTurn = 0;
		}
	}

	// TODO remove invalidated goals (item picked up by enemy etc.)
	while (!availableHeroes.empty()) {
		MOHero* heroObject = availableHeroes.front();
		Hero* hero = heroObject->hero;
		availableHeroes.pop();

		printf("	AI using hero %d.\n", hero->uniqueId);
		setHeroByUniqueId(hero->uniqueId);

		// CHECK modify the scan distance
		pf.findPaths(heroObject->pos /*, hero->movementPoints + HERO_BASE_MOVE_POINTS * 5*/); 

		if (heroGoals.find(hero->uniqueId) == heroGoals.end()) {
			assignGoal(heroObject);
			Goal* tempGoal = heroGoals[hero->uniqueId];
			printf("	AI created goal %s with target (%d, %d).\n",
				tempGoal->toStringSeq().c_str(), tempGoal->target.x, tempGoal->target.y);
		}
		Goal* currGoal = heroGoals[hero->uniqueId];

		Goal* attackGoal = currGoal->findWithType(GoalType::ATTACK_HERO);
		if (attackGoal != nullptr) {
			int targetId = ((GAttackHero*)attackGoal)->targetId;
			MOHero* targetHero = gameLogic.getHeroByUniqueId(targetId);
			if (targetHero != nullptr && !targetHero->hero->isDead && pf.distGhost[targetHero->pos.x][targetHero->pos.y] > -1) {
				attackGoal->target = targetHero->pos;
			}
			else {
				heroGoals[hero->uniqueId] = nullptr;
				delete currGoal;
				assignGoal(heroObject);
				currGoal = heroGoals[hero->uniqueId];
			}
		}

		if (currGoal->target == heroObject->pos) {
			Goal* currGoal = heroGoals[hero->uniqueId];
			// TEMP TODO change the way movement->interaction works, interact with everything on move so this isn't needed
			if (currGoal->type == GoalType::PICK_UP && map->getObject(heroObject->pos)->objectType != MOType::EMPTY) {
				gameLogic.interact(heroObject);
			}
			else if (currGoal->type == GoalType::VISIT) {
				visitedBuildings.insert(currGoal->target);
			}
			else if (currGoal->type == GoalType::ATTACK_HERO) {
				// we updated this goal in the if statement above so we're positive the enemy is actually there
				gameLogic.getHeroByUniqueId(((GAttackHero*)currGoal)->targetId)->interact();
			}
			else if (currGoal->type == GoalType::GATHER_ARMY) {
				MapObject* tempObject = pf.map->getObject(heroObject->pos);
				if (tempObject->objectType != MOType::CASTLE) {
					printf("	AI wants to gather army at an invalid location.\n");
				}
				else {
					printf("	AI moving army from castle to hero.\n");
					transferReservedArmy(tempObject->hero, (GGatherArmy*)currGoal, hero);
				}
			}

			if (currGoal->next != nullptr) {
				Goal* nextGoal = currGoal->next;
				currGoal->next = nullptr;
				delete currGoal;
				heroGoals[hero->uniqueId] = nextGoal;
				printf("	AI moved to goal %s with target (%d, %d).\n",
					nextGoal->toStringSeq().c_str(), nextGoal->target.x, nextGoal->target.y);
			}
			else {
				heroGoals[hero->uniqueId] = nullptr;
				delete currGoal;
				assignGoal(heroObject);
				Goal* tempGoal = heroGoals[hero->uniqueId];
				printf("	AI created goal %s with target (%d, %d).\n",
					tempGoal->toStringSeq().c_str(), tempGoal->target.x, tempGoal->target.y);
			}
		}

		// if the hero has a goal -> lets try to work on it
		if (heroGoals.find(hero->uniqueId) != heroGoals.end()){
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			currGoal = heroGoals[hero->uniqueId];

			int moveStatus = moveHero(heroObject, currGoal->target);

			// TODO remove goal if it fails too many times

			// check if the hero died after (possibly) interacting with the last tile
			if (heroObject->hero->isDead) {
				delete heroGoals[hero->uniqueId];
				heroGoals.erase(hero->uniqueId);
				continue;
			}
			
			// TEMP allowing 3 actions per turn, might help with heroes blocking eachother etc.
			if (hero->movementPoints > 0 && ++currGoal->actionsThisTurn < 3) {
				availableHeroes.push(heroObject);
			}
		}
	}

	// intentionally calling this a second time
	upgradeCastles();

	printf("Adventure: AI turn ending...\n");

	// the ordering of the next two lines matters!
	gameLogic.aiThreadStatus = 0;
	gameLogic.endTurn();
}

void AIPlayer::decideNewHeroPurchases() {
	if (castles.empty()) {
		return;
	}

	std::vector<int> nearbyHeroes(castles.size(), 0);
	for (int i = 0; i < (int)castles.size(); i++) {
		for (int j = 0; j < (int)heroObjects.size(); j++) {
			if (manhattan(castles[i]->pos, heroObjects[j]->pos) < 15) {
				nearbyHeroes[i]++;
			}
		}
	}

	std::vector<int> sortedIndices(castles.size(), 0);
	for (int i = 0; i < (int)castles.size(); i++) {
		sortedIndices[i] = i;
	}
	std::sort(sortedIndices.begin(), sortedIndices.end(), [nearbyHeroes](const int a, const int b) -> bool {
		return nearbyHeroes[a] < nearbyHeroes[b];
	});

	castles[sortedIndices[0]]->buyHero();
}

void AIPlayer::calcCastleTroopAvailability() {
	availableTroops.resize(castles.size());

	for (int i = 0; i < (int)castles.size(); i++) {
		availableTroops[i] = calcCastleTroopAvailability(castles[i]);
	}
}

void AIPlayer::archiveHero(int index) {
	if (heroObjects.empty()) {
		return;
	}

	Goal* goal = heroGoals[heroObjects[index]->hero->uniqueId];
	heroGoals.erase(heroObjects[index]->hero->uniqueId);
	delete goal;

	heroHistory.push_back(heroObjects[index]);
	heroObjects.erase(heroObjects.begin() + index);
	if (heroObjects.empty()) {
		selectedHero = -1;
	}
}

float AIPlayer::calcCastleTroopAvailability(MOCastle* castle) {
	float availablePower = 0;
	int budget = wallet[GOLD] - putAside[GOLD];
	for (int i = DWELL_SEVEN; i >= DWELL_ONE; i--) {
		Buildings::Dwelling* building = (Buildings::Dwelling*)castle->getBuilding(static_cast<BuildingType>(i));
		if (building != nullptr) {
			Creature* creature = building->getCreature();
			availablePower += creaturePower(creature) * building->getMaxAvailable(budget);
			budget -= creature->cost * building->getMaxAvailable(budget);
		}
	}
	return availablePower;
}

void AIPlayer::transferReservedArmy(Hero* heroSource, GGatherArmy* goal, Hero* heroTarget) {
	heroTarget->stackArmyNeatly();

	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		Creature* creature = goal->creatures[i];
		if (creature == nullptr) {
			continue;
		}

		int spaceToMove = -1;
		for (int j = 0; j < HERO_UNIT_SLOTS; j++) {
			if (heroTarget->creatures[j] == nullptr || heroTarget->creatures[j]->name == creature->name) {
				spaceToMove = j;
				break;
			}
		}
		if (spaceToMove == -1) {
			continue;
		}

		int count = creature->count;
		for (int j = 0; j < HERO_UNIT_SLOTS && count > 0; j++) {
			if (heroSource->creatures[j] != nullptr && heroSource->creatures[j]->name == creature->name) {
				int toMove = mmin(heroSource->creatures[j]->count, count);
				count -= toMove;
				// TODO refactor? move deleting to Hero?
				heroSource->creatures[j]->count -= toMove;
				if (heroSource->creatures[j]->count == 0) {
					delete heroSource->creatures[j];
					heroSource->creatures[j] = nullptr;
				}
			}
		}

		if (count == creature->count) {
			continue;
		}

		if (heroTarget->creatures[spaceToMove] == nullptr) {
			heroTarget->creatures[spaceToMove] =
				new Creature(creature, creature->count - count, heroTarget->getFactionId());
		}
		else {
			heroTarget->creatures[spaceToMove]->count += creature->count - count;
		}
	}
}

void AIPlayer::upgradeCastles() {
	Map* map = GameLogic::instance().map;
	for (intp worldBuildingPos : buildingsControlled) {
		MapObject* object = map->getObject(worldBuildingPos);
		if (object == nullptr) {
			continue;
		}

		if (object->objectType == MOType::CASTLE) {
			std::cout << "AI Building at castle " <<  worldBuildingPos.x << ", " << worldBuildingPos.y << std::endl;
			for (int i = 0; i < _BUILDINGTYPE_END; i++) {
				BuildStatus result = ((MOCastle*)object)->build(static_cast<BuildingType>(i));
				if (result == BuildStatus::OK) {
					printf("	Completed building: %s\n", Buildings::toString(static_cast<BuildingType>(i)).c_str());
				}
			}
		}
	}
}

// WARNING update the pathfinder paths before calling this
// CHECK TODO maybe return something? is failing even an option?
void AIPlayer::assignGoal(MOHero* heroObject) {
	Hero* hero = heroObject->hero;
	Map* map = GameLogic::instance().map;

	// TODO add goal queue for goals to be completed in the future? maybe not here though

	std::vector<std::pair<float, Goal*>> candidateGoals;

	// construct candidate goals for reachable interesting buildings
	for (intp endpoint : pf.reachableBuildings) {
		if (visitedBuildings.find(endpoint) != visitedBuildings.end()) {
			continue; // we've already visited this building (this week if revisitable)
		}

		if (targetReserved(endpoint)) {
			continue; // TODO modify this if some buildings become per-hero benefitial (e.g. mana refill)
		}

		std::pair<float, Goal*> candidate = constructGoal(hero, GoalType::VISIT, endpoint);
		if (candidate.second != nullptr) {
			candidateGoals.push_back(candidate);
		}
	}

	// construct candidate goals for reachable items
	for (intp endpoint : pf.reachableItems) {
		if (targetReserved(endpoint)) {
			continue;
		}

		std::pair<float, Goal*> candidate = constructGoal(hero, GoalType::PICK_UP, endpoint);
		if (candidate.second != nullptr) {
			candidateGoals.push_back(candidate);
		}
	}

	// construct candidate goals for reachable enemy heroes
	GameLogic &gameLogic = GameLogic::instance();
	FactionSetup &factionSetup = FactionSetup::instance();
	for (Player* player : GameLogic::instance().players) {
		if (player != nullptr && !factionSetup.areAllies(player->getFactionId(), this->getFactionId())) {
			for (MOHero* candidateHero : player->heroObjects) {
				if (candidateHero != nullptr && !candidateHero->hero->isDead && candidateHero->hero->isReal
					&& pf.distGhost[candidateHero->pos.x][candidateHero->pos.y] != -1) {
					
					bool heroMarkedForAttack = false;
					for (auto &assignment : heroGoals) {
						Goal* attackGoal = assignment.second->findWithType(GoalType::ATTACK_HERO);
						if (attackGoal != nullptr && ((GAttackHero*)attackGoal)->targetId == candidateHero->hero->uniqueId) {
							heroMarkedForAttack = true;
							break;
						}
					}
					if (heroMarkedForAttack) {
						continue;
					}

					std::pair<float, Goal*> candidate = constructGoal(hero, GoalType::ATTACK_HERO, candidateHero->pos);
					if (candidate.second != nullptr) {
						candidateGoals.push_back(candidate);
					}
				}
			}
		}
	}

	std::sort(candidateGoals.begin(), candidateGoals.end(),
		[](const std::pair<float, Goal*> a, const std::pair<float, Goal*> b) -> bool {
		return a.first < b.first;
	});

	if (!candidateGoals.empty()) {
		Goal* goal = candidateGoals[0].second;
		heroGoals[hero->uniqueId] = goal;
		while (goal != nullptr) {
			if (goal->type == GoalType::GATHER_ARMY) {
				purchaseTroops(
					(MOCastle*)pf.map->getObject(goal->target),
					((GGatherArmy*)goal)->powerRequired,
					(GGatherArmy*)goal);
			}
			goal = goal->next;
		}
	}
	else {
		heroGoals[hero->uniqueId] = getExploreGoal(heroObject);
		// TEMP
		if (heroGoals[hero->uniqueId] == nullptr) {
			heroGoals[hero->uniqueId] =
				new Goal(GoalType::WAIT, heroObject->pos,
					new Goal(GoalType::WAIT, heroObject->pos));
		}
	}

	// delete the unused goals
	for (int i = 1; i < (int)candidateGoals.size(); i++) {
		delete candidateGoals[i].second;
	}
}

std::pair<float, Goal*> AIPlayer::constructGoal(Hero* hero, GoalType type, intp target) {
	// TODO refactor a bit
	float missingTroops = missingPowerToReach(hero, target);
	if (missingTroops <= 0) {
		intp approachPoint = getApproachPoint(target);
		Goal* finalGoal = nullptr;
		if (type == GoalType::ATTACK_HERO) {
			finalGoal = new GAttackHero(type, target, GameLogic::instance().getHeroAt(target)->hero->uniqueId);
		}
		else {
			finalGoal = new Goal(type, target);
		}
		if (approachPoint == target) {
			return std::make_pair((float)pf.distSolid[target.x][target.y], finalGoal);
		}
		else {
			Goal* transitionGoal = new Goal(GoalType::MOVE, approachPoint, finalGoal);
			return std::make_pair(pf.distSolid[approachPoint.x][approachPoint.y] +
				pf.distGhost[target.x][target.y] - pf.distGhost[approachPoint.x][approachPoint.y], transitionGoal);
		}
	}
	else {
		MOCastle* castle = closestCreatureSource(missingTroops);
		if (castle == nullptr) {
			return std::make_pair(0, nullptr);
		}

		intp approachPoint = getApproachPoint(target);
		Goal* nextGoal;
		float nextDistance = 0;

		Goal* finalGoal = nullptr;
		if (type == GoalType::ATTACK_HERO) {
			finalGoal = new GAttackHero(type, target, GameLogic::instance().getHeroAt(target)->hero->uniqueId);
		}
		else {
			finalGoal = new Goal(type, target);
		}
		if (approachPoint == target) {
			nextGoal = finalGoal;
			nextDistance = pf.distSolid[target.x][target.y];
		}
		else {
			nextGoal = new Goal(GoalType::MOVE, approachPoint, finalGoal);
			nextDistance = pf.distSolid[approachPoint.x][approachPoint.y] +
				pf.distGhost[target.x][target.y] - pf.distGhost[approachPoint.x][approachPoint.y];
		}
		Goal* goal = new GGatherArmy(GoalType::GATHER_ARMY, castle->pos, missingTroops, nextGoal);
		return std::make_pair(2 * pf.distSolid[castle->pos.x][castle->pos.y] + nextDistance, goal);
	}
}

bool AIPlayer::targetReserved(intp target) {
	// check if this goal target is already assigned to a different hero
	for (auto &assignment : heroGoals) {
		Goal* goal = assignment.second;
		if (goal == nullptr) {
			continue;
		}
		while (goal->next != nullptr) {
			goal = goal->next;
		}
		if (goal->target == target) {
			return true;
		}
	}
	return false;
}

MOCastle* AIPlayer::closestCreatureSource(float requiredTroops) {
	MOCastle* closestProvider = nullptr;

	int bestDist = oo;
	for (int i = 0; i < (int)castles.size(); i++) {
		// if the castle can provide the troops and is reachable
		int dist = pf.distSolid[castles[i]->pos.x][castles[i]->pos.y];
		if (availableTroops[i] >= requiredTroops && dist >= 0 && dist < bestDist) {
			bestDist = dist;
			closestProvider = castles[i];
		}
	}

	return closestProvider;
}

/*
	Calculates how much more troop power we need to reach the location while defeating
	everything that's blocking us (guards).
	Note: If we can avoid fighting something we usually will, for example:
	......
	......
	S.x.xE
	(starting at S, we want to reach E, x are creatures)
	We will avoid the left creature by going 2 rows above, but the second one
	is directly guarding E so we have to fight it. Therefore, we only take the
	second creature when calculating the result.
*/
float AIPlayer::missingPowerToReach(Hero* hero, intp location) {
	intp closestUnguarded = getApproachPoint(location);
	std::vector<intp> ghostPath = pf.getPathTo(location, true);

	// we can safely assume the approach point is on the ghost path (see getApproachPoint)
	for (int i = 0; i < (int)ghostPath.size(); i++) {
		if (ghostPath[i] == closestUnguarded) {
			ghostPath.erase(ghostPath.begin(), ghostPath.begin() + i);
			break;
		}
	}

	std::set<intp> threatsCleared; // assuming we can't have two threats originating from the same position
	float totalMissingPower = 0;
	float currentPower = totalPower(hero);
	for (int i = 0; i < (int)ghostPath.size(); i++) {
		std::vector<intp> safeCopy = pf.map->threats[ghostPath[i].x][ghostPath[i].y];
		for (intp threatOrigin : safeCopy) {
			if (threatsCleared.find(threatOrigin) != threatsCleared.end()) {
				continue; // we've already cleared this threat
			}
			threatsCleared.insert(threatOrigin);

			// WARNING if map object logic changes (e.g. multiple with same position) might get the wrong one
			Hero* enemy = pf.map->getObject(threatOrigin)->hero;
			float missingPower = missingPowerToWin(currentPower, enemy);
			if (missingPower > 0) {
				totalMissingPower += missingPower;
				currentPower += missingPower;
			}
			currentPower *= AI_COMBAT_OPTIMISM;
		}
	}

	// in case we want to attack a hero at the destination point
	MOHero* heroObject = GameLogic::instance().getHeroAt(location);
	if (heroObject != nullptr) {
		float missingPower = missingPowerToWin(currentPower, heroObject->hero);
		if (missingPower > 0) {
			totalMissingPower += missingPower;
		}
	}

	return totalMissingPower;
}

// finds the closest point that reveals the most fogged (invisible) tiles
Goal* AIPlayer::getExploreGoal(MOHero* heroObject) {
	std::vector<intp> allReachable = pf.getReachableTiles();
	std::vector<std::pair<int, intp>> candidateTiles;

	candidateTiles.reserve(allReachable.size());
	for (intp tile : allReachable) {
		if (!pf.map->isThreatened(tile) && !GameLogic::instance().tileHasHero(tile) && pf.countInvisibleAround(tile, 5) > 0) {
			candidateTiles.push_back(std::make_pair(pf.distSolid[tile.x][tile.y], tile));
		}
	}
	sort(candidateTiles.begin(), candidateTiles.end(),
		[](const std::pair<int, intp> &a, const std::pair<int, intp> &b) -> bool {
		return a.first < b.first;
	});

	intp bestTile(-1, -1);
	// we'll look for good explorable tiles nearby and then broaden the search
	// TODO modify this, it's a bit wonky
	int range = mmin(heroObject->hero->movementPoints, HERO_BASE_MOVE_POINTS / 3);
	candidateTiles.push_back(std::make_pair(oo, intp(-1, -1))); // stopping condition for the wonky loop
	for (int i = 0; i < (int)candidateTiles.size(); i++) {
		if (candidateTiles[i].first > range) {
			sort(candidateTiles.begin(), candidateTiles.begin() + i,
				[this](const std::pair<int, intp> &a, const std::pair<int, intp> &b) -> bool {
				return a.first + 0.3 * getClosestCastleAirDist(a.second) - pf.countInvisibleAround(a.second, 5) <
					b.first + 0.3 * getClosestCastleAirDist(b.second) - pf.countInvisibleAround(b.second, 5);
			});

			if (i > 0) {
				bestTile = candidateTiles[0].second;
				break;
			}

			range += HERO_BASE_MOVE_POINTS;
		}
	}

	if (bestTile == intp(-1, -1)) {
		return nullptr;
	}
	return new Goal(GoalType::MOVE, bestTile);
}

// WARNING presumes the castle ownership is updated for this turn
int AIPlayer::getClosestCastleAirDist(intp location) {
	int bestDist = oo;
	for (MOCastle* castle : castles) {
		bestDist = mmin(bestDist, (int)manhattan(castle->pos, location));
	}
	return bestDist;
}

intp AIPlayer::getApproachPoint(intp location) {
	std::vector<intp> path = pf.getPathTo(location, true);

	if (path.empty()) {
		return intp(-1, -1); // not reachable
	}

	// CHECK are items even skipped? probably doesn't matter
	// find the furthest point on the ghost path we can reach even with creatures / items / etc. in the way
	int furthest = 0;
	for (int i = 1; i < (int)path.size(); i++) {
		intp temp = path[i];
		if (pf.distSolid[temp.x][temp.y] >= 0 && !pf.map->isThreatened(temp) && !GameLogic::instance().tileHasHero(temp)) {
			furthest = i;
		}
	}

	return path[furthest];
}

int AIPlayer::moveHero(MOHero* heroObject, intp target) {
	std::vector<intp> ghostPath = pf.getPathTo(target, true);

	if (ghostPath.empty()) {
		return 0; // not reachable
	}
	if (ghostPath[0] != heroObject->pos) {
		printf("ERROR: AIPlayer using the wrong hero's calculations for pathfinding.\n");
		return -1; // we're using the wrong hero's calculations
	}

	// find a clear path to the furthest point
	intp approachPoint = getApproachPoint(target);
	std::vector<intp> path = pf.getPathTo(approachPoint);
	// move the hero tile-by-tile
	int i = 1;
	for (; i < (int)path.size() && heroObject->hero->movementPoints > 0; i++) {
		heroObject->moveTo(path[i], 1);
		pf.makeVisibleAround(path[i], 5); // TODO make the scouting radius a hero property (not only here!)
		std::this_thread::sleep_for(std::chrono::milliseconds(200)); // TEMP AI movement animation
	}

	if (heroObject->hero->movementPoints > 0) {
		// we can safely assume the approach point is on the ghost path (see getApproachPoint)
		for (int i = 0; i < (int)ghostPath.size(); i++) {
			if (ghostPath[i] == approachPoint) {
				ghostPath.erase(ghostPath.begin(), ghostPath.begin() + i);
				break;
			}
		}
		for (int i = 0; i < (int)ghostPath.size() && heroObject->hero->movementPoints > 0; i++) {
			heroObject->moveTo(ghostPath[i], 1);
			pf.makeVisibleAround(ghostPath[i], 5); // TODO make the scouting radius a hero property (not only here!)
			std::this_thread::sleep_for(std::chrono::milliseconds(200)); // TEMP AI movement animation
		}
	}

	if (heroObject->pos == target) {
		return 1; // we managed to reach the target (either directly or we took some turns)
	}
	return 2; // partial movement towards the target
}

void AIPlayer::purchaseTroops(MOCastle* castle, float powerRequired, GGatherArmy* purchaseInfo) {
	float purchasedPower = 0;
	for (int i = DWELL_SEVEN; i >= DWELL_ONE && purchasedPower < powerRequired; i--) {
		Buildings::Dwelling* building = (Buildings::Dwelling*)castle->getBuilding(static_cast<BuildingType>(i));
		if (building != nullptr) {
			Creature* creature = building->getCreature();
			int buyAmount = mmin(
				building->getMaxAvailable(wallet[GOLD]),
				(int)(powerRequired / building->getCreature()->universalPower()));
			purchasedPower += buyAmount * building->getCreature()->universalPower();
			building->purchase(buyAmount, this, castle->hero);
			// WARNING WARNING WARNING
			if (buyAmount > 0) {
				purchaseInfo->creatures[i - DWELL_ONE] = new Creature(building->getCreature(), buyAmount, factionId);
			}
		}
	}
	
	if (purchasedPower > 0) {
		calcCastleTroopAvailability();
	}

	printf("	AI purchased creatures in castle, total power %f.\n", purchasedPower);
}

float AIPlayer::missingPowerToWin(float power, Hero* enemy) {
	return totalPower(enemy) * AI_AGGRESSION_FACTOR - power;
}

float AIPlayer::missingPowerToWin(Hero* hero, Hero* enemy) {
	return totalPower(enemy) * AI_AGGRESSION_FACTOR - totalPower(hero);
}

float AIPlayer::totalPower(Hero* hero) {
	float ret = 0;
	for (Creature* creature : hero->creatures) {
		ret += creaturePower(creature);
	}
	return ret;
}

float AIPlayer::creaturePower(Creature* creature) {
	if (creature == nullptr) {
		return 0;
	}

	// TODO improve calculation or use config
	return (creature->baseDamageMin + creature->baseDamageMax) / 2.f * creature->count;
}

Goal::Goal(GoalType _type, intp _target, Goal * _next)
	: type(_type), target(_target), next(_next), actionsThisTurn(0) {
}

Goal::~Goal() {
	delete next;
	next = nullptr;
}

GGatherArmy::~GGatherArmy() {
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		delete creatures[i];
	}
}

GAttackHero::~GAttackHero() {
}

Goal * Goal::findWithType(GoalType type) {
	Goal* current = this;
	while (current != nullptr) {
		if (current->type == type) {
			return current;
		}
		current = current->next;
	}
	return nullptr;
}

std::string Goal::toString() {
	switch (type) {
		case MOVE:
			return "Move";
		case GHOSTMOVE:
			return "GhostMove";
		case PICK_UP:
			return "PickUp";
		case VISIT:
			return "Visit";
		case ATTACK_HERO:
			return "AttackHero";
		case GATHER_ARMY:
			return "GatherArmy";
		case WAIT:
			return "Wait";
		default:
			return "UNKNOWN";
	}
}

std::string Goal::toStringSeq() {
	std::string ret = "";

	Goal* goal = this;
	while (goal != nullptr) {
		ret += goal->toString() + "->";
		goal = goal->next;
	}

	return ret + "NULL";
}

GGatherArmy::GGatherArmy(GoalType _type, intp _target, float _powerRequired, Goal* _next)
	: Goal(_type, _target, _next), powerRequired(_powerRequired) {
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		creatures[i] = nullptr;
	}
}

GAttackHero::GAttackHero(GoalType _type, intp _target, int _targetId, Goal * _next)
	: Goal(_type, _target, _next), targetId(_targetId) {
}
