#include "Map.h"

// some headers included here to avoid circular dependency problems
#include <algorithm>
#include <queue>
#include <vector>
#include "MOItem.h"
#include "MOEmpty.h"
#include "MOWall.h"
#include "MOCastle.h"
#include "MOCreature.h"
#include "MOResource.h"
#include "MOPath.h"
#include "MOMine.h"
#include "Utility.h"
#include "BIT.h"
#include "Resources.h"

Map::Map() : dataTree(nullptr) {}

Map::Map(int _colCount, int _rowCount) : colCount(_colCount), rowCount(_rowCount), dataTree(nullptr) {
}

void Map::fillMap() {/*
	for (int i = 0; i < colCount; i++) {
		for (int j = 0; j < rowCount; j++) {
			int random = rand() % 100;
			if (random > 80) {
				mapObjects[i][j] = new MOWall(vec2(i, j));
			}
			else if (random > 70) {
				mapObjects[i][j] = new MOItem(vec2(i, j), this);
			}
			else if (random > 68) {
				mapObjects[i][j] = new MOBuilding(vec2(i, j));
			}
			else if (random > 66) {
				mapObjects[i][j] = new MOCreature(vec2(i, j), "Slow", 15);
				spreadThreat(vec2(i, j));
			}
			else if (random > 63) {
				// TODO maybe only allow dynamic positioning in MOCreature? depends on opponent?
				vector<int> counts;
				counts.push_back(5);
				counts.push_back(10);
				counts.push_back(5);
				mapObjects[i][j] = new MOCreature(vec2(i, j), "Slow", counts);
				spreadThreat(vec2(i, j));
			}
			else {
				mapObjects[i][j] = new MOEmpty(vec2(i, j));
			}
		}
	}

	mapObjects[0][0] = new MOEmpty(vec2(0, 0));*/
}

void Map::testFillMap(int zones) {
	if (dataTree != nullptr) {
		delete dataTree;
	}
	dataTree = new BIT(colCount, rowCount);

	for (int i = 0; i < zones; i++) {
		adjZones[i].clear();
	}

	memset(zoneId, -1, sizeof zoneId);
	memset(blueprint, UNKNOWN, sizeof blueprint);
	memset(zoneStartingFaction, 0, sizeof zoneStartingFaction);
	zoneOrigins.clear();
	zoneRadius.clear();
	
	// the outer map borders
	/* unnecessary
	for (int i = 0; i < colCount; i++) {
		blueprint[i][0] = WALL;
		blueprint[i][rowCount - 1] = WALL;
	}
	for (int i = 0; i < rowCount; i++) {
		blueprint[0][i] = WALL;
		blueprint[colCount - 1][i] = WALL;
	}
	*/

	// spread out the zone origins
	for (int i = 0; i < zones; i++) {
		float bestDist = 0;
		intp bestOrigin(0, 0);
		for (int j = 0; j < MGEN_ORIGIN_CANDIDATES; j++) {
			// TEMP castles (at origins) will be 3 wide and 2 high + 1 free row under to be accessible
			intp candidate(rand() % (colCount - 2) + 1, rand() % (rowCount - 2) + 1); 
			float currDist = oo;
			for (int k = 0; k < i; k++) {
				currDist = mmin(currDist, euclid(zoneOrigins[k], candidate));
			}
			if (currDist > bestDist) {
				bestDist = currDist;
				bestOrigin = candidate;
			}
		}
		zoneOrigins.push_back(bestOrigin);
		// TEMP castle layout
		blueprint[bestOrigin.x][bestOrigin.y] = CASTLE;
		blueprint[bestOrigin.x - 1][bestOrigin.y] = PART;
		blueprint[bestOrigin.x - 1][bestOrigin.y + 1] = PART;
		blueprint[bestOrigin.x][bestOrigin.y + 1] = PART;
		blueprint[bestOrigin.x + 1][bestOrigin.y + 1] = PART;
		blueprint[bestOrigin.x + 1][bestOrigin.y] = PART;
	}

	// find the Voronoi zone boundaries
	for (int i = 0; i < MGEN_ZONE_LIMIT; i++) {
		for (int j = 0; j < MGEN_ZONE_LIMIT; j++) {
			boundaries[i][j].clear();
		}
	}
	vector<int> tempZones;
	for (int i = 0; i < colCount; i++) {
		for (int j = 0; j < rowCount; j++) {
			vec2 current(i, j);

			tempZones.clear();
			for (int k = 0; k < zones; k++) {
				tempZones.push_back(k);
			}
			sort(tempZones.begin(), tempZones.end(),
				[&](const int & a, const int & b) -> bool {
				return euclid(zoneOrigins[a], current) < euclid(zoneOrigins[b], current);
			});

			if (euclid(zoneOrigins[tempZones[1]], current) - euclid(zoneOrigins[tempZones[0]], current) <= 1) {
				boundaries[mmin(tempZones[0], tempZones[1])][mmax(tempZones[0], tempZones[1])].push_back(current);
			}
		}
	}

	// divide the zones with the found Voronoi boundaries
	for (int i = 0; i < zones; i++) {
		for (int j = i + 1; j < zones; j++) {
			if (!boundaries[i][j].empty()) {
				adjZones[i].push_back(j);
				adjZones[j].push_back(i);
				for (int k = 0; k < (int)boundaries[i][j].size(); k++) {
					intp boundaryTile = boundaries[i][j][k];

					// apply the boundary tile
					blueprint[boundaryTile.x][boundaryTile.y] = WALL;
					dataTree->update(boundaryTile, 1);

					// "brush"-spread the boundary
					// TODO don't use this for now (mby Blueprint::WOULD_LOOK_PRETTY_AS_A_WALL)
					for (int l = 0; l < 4; l++) {
						intp adjTile = boundaryTile + MGEN_BOUNDARY_BRUSH[l];
						if (adjTile.x >= 0 && adjTile.y >= 0 && adjTile.x < colCount && adjTile.y < rowCount) {
							blueprint[adjTile.x][adjTile.y] = WALL;
							dataTree->update(adjTile, 1);
						}
					}
				}
			}
		}
	}

	// store the zone areas
	memset(zoneCount, 0, sizeof zoneCount);
	for (int i = 0; i < zones; i++) {
		zoneRadius.push_back(0);

		queue<intp> bfsq;
		bfsq.push(zoneOrigins[i]);
		zoneId[bfsq.front().x][bfsq.front().y] = i;
		zoneCount[i]++;
		while (!bfsq.empty()) {
			intp currTile = bfsq.front();
			bfsq.pop();
			for (int j = 0; j < HERO_MOVE_DIRS; j++) {
				intp adjTile = currTile + intp(HERO_MOVE_DIR[j]);
				if (adjTile.x >= 0 && adjTile.x < colCount && adjTile.y >= 0 && adjTile.y < rowCount &&
					!~zoneId[adjTile.x][adjTile.y] && blueprint[adjTile.x][adjTile.y] != WALL) {
					zoneId[adjTile.x][adjTile.y] = i;
					zoneCount[i]++;
					bfsq.push(adjTile);

					zoneRadius[i] = mmax(manhattan(adjTile, zoneOrigins[i]), zoneRadius[i]);
				}
			}
		}
	}

	// break some boundaries to connect the zones
	memset(tempFlags, 0, sizeof tempFlags);
	for (int i = 0; i < zones; i++) {
		for (int j = i + 1; j < zones; j++) {
			if (!boundaries[i][j].empty()) {
				for (int k = 0; k < MGEN_ZONE_CONNECTEDNESS; k++) {
					int breakIndex = rand() % boundaries[i][j].size();
					intp breakStart = boundaries[i][j][breakIndex];
					int flag = -(i * 1000 + j); // flag id coding, not used anywhere else

					// breaking points for the first / second zone
					intp adjFirst(-1, -1);
					intp adjSecond(-1, -1);
					queue<intp> breaking;
					breaking.push(breakStart);

					// DEBUG
					//cout << "Zone " << i << "|" << j << " wall broken at (" << breaking.front().x << ", " << breaking.front().y << ")" << endl;

					blueprint[breakStart.x][breakStart.y] = PATH;
					// POSSIBLE dataTree update
					tempFlags[breakStart.x][breakStart.y] = flag;
					while (!breaking.empty() && (adjFirst.x < 0 || adjSecond.x < 0)) {
						intp currTile = breaking.front();
						breaking.pop();
						for (int k = 0; k < HERO_MOVE_DIRS && (adjFirst.x < 0 || adjSecond.x < 0); k++) {
							intp adjTile = currTile + HERO_MOVE_DIR[k];
							if (adjTile.x >= 0 && adjTile.x < colCount && adjTile.y >= 0 && adjTile.y < rowCount && tempFlags[adjTile.x][adjTile.y] != flag) {
								if (blueprint[adjTile.x][adjTile.y] != UNKNOWN) {
									breaking.push(adjTile);
									tempFlags[adjTile.x][adjTile.y] = flag;
									tempData[adjTile.x][adjTile.y] = currTile;
								}
								else if (zoneId[adjTile.x][adjTile.y] == i && adjFirst.x < 0) {
									adjFirst = currTile;
								}
								else if (zoneId[adjTile.x][adjTile.y] == j && adjSecond.x < 0) {
									adjSecond = currTile;
								}
							}
						}
					}

					// this shouldn't ever ever ever happen
					if (adjFirst.x < 0 || adjSecond.x < 0) {
						cout << "Failed to break the boundary between " << i << " " << j << endl;
						continue;
					}

					intp breakPos = adjFirst;
					while (breakPos != breakStart) {
						blueprint[breakPos.x][breakPos.y] = PATH;
						// POSSIBLE dataTree update
						breakPos = tempData[breakPos.x][breakPos.y];
					}
					breakPos = adjSecond;
					while (breakPos != breakStart) {
						blueprint[breakPos.x][breakPos.y] = PATH;
						// POSSIBLE dataTree update
						breakPos = tempData[breakPos.x][breakPos.y];
					}
				}
			}
		}
	}


	// create the main paths between the zone origins (will always stay unblocked)
	// TODO make them turn and stuff, currently just the shortest path
	memset(tempFlags, -1, sizeof tempFlags);
	for (int i = 0; i < zones; i++) {
		queue<intp> bfsq;
		bfsq.push(zoneOrigins[i]);
		tempFlags[zoneOrigins[i].x][zoneOrigins[i].y] = i;
		while (!bfsq.empty()) {
			intp currTile = bfsq.front();
			bfsq.pop();
			for (int j = 0; j < HERO_MOVE_DIRS; j++) {
				intp adjTile = currTile + HERO_MOVE_DIR[j];
				if (adjTile.x >= 0 && adjTile.x < colCount && adjTile.y >= 0 && adjTile.y < rowCount &&
					(blueprint[adjTile.x][adjTile.y] != WALL && blueprint[adjTile.x][adjTile.y] != PART)
					&& tempFlags[adjTile.x][adjTile.y] != i) {
					tempFlags[adjTile.x][adjTile.y] = i;
					tempData[adjTile.x][adjTile.y] = currTile;
					bfsq.push(adjTile);
				}
			}
		}

		for (int j = i + 1; j < zones; j++) {
			if (!boundaries[i][j].empty()) {
				if (tempFlags[zoneOrigins[j].x][zoneOrigins[j].y] == i) {
					intp currTile = tempData[zoneOrigins[j].x][zoneOrigins[j].y];
					while (currTile != zoneOrigins[i]) {
						blueprint[currTile.x][currTile.y] = PATH;
						dataTree->update(currTile, 1);
						currTile = tempData[currTile.x][currTile.y];
					}
				}
				else {
					// DEBUG shouldn't ever happen
					cout << "Missing a path from zone " << i << " to " << j << endl;
				}
			}
		}
	}

	// BETA fill the zones with a balanced number of objects
	map<char, int> blockToBlueprint;
	blockToBlueprint['.'] = UNKNOWN;
	blockToBlueprint[','] = PART;
	blockToBlueprint['x'] = WALL;
	blockToBlueprint['I'] = ITEM;
	blockToBlueprint['C'] = CREATURE;
	blockToBlueprint['M'] = MINE;
	vector<string> blockGenOrder;
	vector<int> blockGenCount;
	blockGenOrder.push_back("001");
	blockGenCount.push_back(6);
	blockGenOrder.push_back("110");
	blockGenCount.push_back(25);
	blockGenOrder.push_back("000");
	blockGenCount.push_back(30);

	int testTries = 50;
	vector<intp> zoneLocations;
	for (int z = 0; z < zones; z++) {
		zoneLocations.clear();
		for (int i = 0; i < colCount; i++) {
			for (int j = 0; j < rowCount; j++) {
				if (zoneId[i][j] == z) {
					zoneLocations.push_back(intp(i, j));
				}
			}
		}
		for (int blockType = 0; blockType < (int)blockGenOrder.size(); blockType++) {
			int count = blockGenCount[blockType];
			int tries = testTries;
			vector<MapBlock*> goodBlocks = Resources::getBlocksWith(blockGenOrder[blockType]);
			while (count && tries--) {
				intp randomPos = zoneLocations[rand() % zoneLocations.size()];
				MapBlock* randomBlock = goodBlocks[rand() % goodBlocks.size()];
				// TODO use BIT for bigger blocks
				bool fits = (randomPos.x + randomBlock->width - 1 < colCount && randomPos.y - randomBlock->height + 1 >= 0);
				for (int i = randomPos.x; fits && i < randomPos.x + randomBlock->width; i++) {
					for (int j = randomPos.y; j > randomPos.y - randomBlock->height; j--) {
						if (blueprint[i][j] != UNKNOWN) {
							fits = false;
							break;
						}
					}
				}
				if (fits) {
					count--;

					for (int i = randomPos.x; i < randomPos.x + randomBlock->width; i++) {
						for (int j = randomPos.y; j > randomPos.y - randomBlock->height; j--) {
							blueprint[i][j] = blockToBlueprint[randomBlock->data[randomPos.y - j][i - randomPos.x]];
						}
					}
				}
			}
		}
	}

	// TESTING specify the obstacle types before creating objects
	int treeGroup = 8;
	memset(tempFlags, false, sizeof tempFlags);
	for (int i = 0; i < colCount; i++) {
		for (int j = 0; j < rowCount; j++) {
			if (blueprint[i][j] == WALL && !tempFlags[i][j]) {
				tempFlags[i][j] = true;

				int count = 1;
				int curr = 0;
				vector<intp> bfsq;
				bfsq.push_back(intp(i, j));
				while (curr < (int)bfsq.size()) {
					for (int k = 0; k < 8; k++) {
						intp nextTile = bfsq[curr] + HERO_MOVE_DIR[k];
						if (nextTile.x >= 0 && nextTile.x < colCount && nextTile.y >= 0 && nextTile.y < rowCount &&
							blueprint[nextTile.x][nextTile.y] == WALL && !tempFlags[nextTile.x][nextTile.y]) {
							tempFlags[nextTile.x][nextTile.y] = true;
							bfsq.push_back(nextTile);
							count++;
						}
					}
					curr++;
				}

				if (count <= treeGroup) {
					for (intp tree : bfsq) {
						blueprint[tree.x][tree.y] = TREE;
					}
				}
			}
		}
	}

	// create the actual objects following the blueprint
	memset(tempFlags, 0, sizeof tempFlags);
	memset(zoneMineCount, 0, sizeof zoneMineCount);
	for (int i = 0; i < colCount; i++) {
		for (int j = 0; j < rowCount; j++) {
			if (blueprint[i][j] == WALL) {
				mapObjects[i][j] = new MOWall(vec2(i, j));
			}
			else if (blueprint[i][j] == PART){
				mapObjects[i][j] = new MOWall(vec2(i, j), true);
			}
			else if (blueprint[i][j] == TREE) {
				mapObjects[i][j] = new MOWall(vec2(i, j));
				mapObjects[i][j]->setModel(Resources::modelData["tree"]);
			}
			else if (blueprint[i][j] == ITEM) {
				// TEMP every item is a resource for now
				blueprint[i][j] = RESOURCE;
				mapObjects[i][j] = new MOResource(vec2(i, j)); // will create a random resource pile
			}
			else if (blueprint[i][j] == CASTLE) {
				mapObjects[i][j] = new MOCastle(this, vec2(i, j));
			}
			else if (blueprint[i][j] == PATH) {
				mapObjects[i][j] = new MOPath(this, vec2(i, j));
			}
			else if (blueprint[i][j] == CREATURE) {
				if (zoneId[i][j] < 0 || zoneId[i][j] >= zones) {
					cout << "TODO map.cpp: creatures spawning on zone boundaries" << endl;
				}
				else {
					int level = clamp(1, CREATURES_MAX_LVL, manhattan(intp(i, j), zoneOrigins[zoneId[i][j]]) / zoneRadius[zoneId[i][j]] * CREATURES_MAX_LVL + 1);
					// TODO maybe store the creatures earlier
					vector<Creature*> candidates = Resources::getCreaturesOfLevel(level);
					Creature* tempCreature = candidates[rand() % candidates.size()];
					mapObjects[i][j] =
						new MOCreature(
							vec2(i, j),
							tempCreature,
							rand() % (CREATURE_SPAWN_MAX[level - 1] - CREATURE_SPAWN_MIN[level - 1] + 1) + CREATURE_SPAWN_MIN[level - 1]
						);
					cout << "Map generation: Spawning a " << tempCreature->name << " stack at (" << i << ", " << j << ")" << endl;
					spreadThreat(vec2(i, j));
				}
			}
			else if (blueprint[i][j] == MINE) {
				mapObjects[i][j] = new MOMine(RES_MINE_SPAWN_ORDER[zoneMineCount[zoneId[i][j]]++ % _RESOURCE_END], this, vec2(i, j));
				// TODO add a real model ... mapObjects[i][j]->setModel(Resources::modelData["mine"]);
			}
			else {
				mapObjects[i][j] = new MOEmpty(vec2(i, j));
			}
		}
	}

	// DEBUG
	for (int i = 0; i < zones; i++) {
		cout << "Zone " << i << " origin is at (" << zoneOrigins[i].x << ", " << zoneOrigins[i].y << ")" << endl;
		cout << "     contains " << zoneCount[i] << " tiles " << endl;
	}
}

void Map::clearMap() {
	for (int i = 0; i < colCount; i++) {
		for (int j = 0; j < rowCount; j++) {
			delete mapObjects[i][j];
			mapObjects[i][j] = nullptr;
		}
	}
	for (int i = 0; i < colCount; i++) {
		for (int j = 0; j < rowCount; j++) {
			threats[i][j].clear();
		}
	}
}

intp Map::factionStartingZone(int factionId) {
	for (int i = 0; i < (int)zoneOrigins.size(); i++) {
		if (zoneStartingFaction[i] == factionId){
			return zoneOrigins[i];
		}
	}

	// TODO potentially add better starting zone assignment
	int candidateZone = -1;
	for (int i = 0; i < (int)zoneOrigins.size(); i++) {
		if (!zoneStartingFaction[i]){
			if (!~candidateZone ||
				(adjZones[i].size() < adjZones[candidateZone].size() && adjZones[i].size() > 1)) {
				candidateZone = i;
			}
		}
	}

	if (!~candidateZone) {
		return intp(0, 0);
	}

	zoneStartingFaction[candidateZone] = factionId;
	mapObjects[zoneOrigins[candidateZone].x][zoneOrigins[candidateZone].y]->setControl(factionId);
	return zoneOrigins[candidateZone];
}

void Map::removeObject(intp location) {
	delete mapObjects[location.x][location.y];
	mapObjects[location.x][location.y] = new MOEmpty(location);

	removeThreat(location);
}

MapObject * Map::getObject(intp location) {
	if (location.x < 0 || location.x >= colCount || location.y < 0 || location.y >= rowCount) {
		return nullptr;
	}

	return mapObjects[location.x][location.y];
}

MapObject * Map::getFirstThreat(intp location) {
	if (threats[location.x][location.y].size() == 0) {
		return nullptr;
	}

	return getObject(threats[location.x][location.y][0]);
}

bool Map::isThreatened(intp location) {
	return !threats[location.x][location.y].empty();
}

void Map::spreadThreat(intp pos) {
	// TODO do something about the constant?
	for (int i = 0; i < 9; i++) {
		intp newPos = pos + MAP_THREAT_DIR[i];
		if (newPos.x < 0 || newPos.x >= colCount || newPos.y < 0 || newPos.y >= rowCount) {
			continue;
		}
		threats[(int)newPos.x][(int)newPos.y].push_back(pos);
	}
}

void Map::removeThreat(intp pos) {
	// TODO do something about the constant?
	for (int i = 0; i < 9; i++) {
		intp newPos = pos + MAP_THREAT_DIR[i];
		if (newPos.x < 0 || newPos.x >= colCount || newPos.y < 0 || newPos.y >= rowCount) {
			continue;
		}
		
		// TODO modify
		auto it = find(threats[(int)newPos.x][(int)newPos.y].begin(),
			threats[(int)newPos.x][(int)newPos.y].end(), pos);
		if (it != threats[(int)newPos.x][(int)newPos.y].end()) {
			threats[(int)newPos.x][(int)newPos.y].erase(it);
		}
	}
}
