#include "Pathfinder.h"
#include <queue>
#include "GameLogic.h"

Pathfinder::Pathfinder() {
	map = GameLogic::instance().map;
	memset(tileVisible, 0, sizeof tileVisible);
}

void Pathfinder::makeVisibleAround(intp tile, int radius) {
	for (int dist = 0; dist <= radius; dist++) {
		for (int x = 0; x <= dist; x++) {
			for (int y = 0; y <= dist - x; y++) {
				if (tile.x - x >= 0) {
					if (tile.y - y >= 0) {
						tileVisible[tile.x - x][tile.y - y] = true;
					}
					if (tile.y + y < map->rowCount) {
						tileVisible[tile.x - x][tile.y + y] = true;
					}
				}
				if (tile.x + x < map->colCount) {
					if (tile.y - y >= 0) {
						tileVisible[tile.x + x][tile.y - y] = true;
					}
					if (tile.y + y < map->rowCount) {
						tileVisible[tile.x + x][tile.y + y] = true;
					}
				}
			}
		}
	}
}

void Pathfinder::findPaths(intp origin, int maxDistance) {
	memset(distSolid, -1, sizeof distSolid);
	distSolid[origin.x][origin.y] = 0;

	// using bfs for now because we're dealing with unit costs
	std::queue<intp> bfsQueue;
	bfsQueue.push(origin);
	backSolid[origin.x][origin.y] = intp(-1, -1); // to be able to find the origin even if using a wrong hero
	while (!bfsQueue.empty()) {
		intp current = bfsQueue.front();
		bfsQueue.pop();

		if (current != origin && (map->getObject(current)->isHolding() || map->isThreatened(current))) {
			continue;
		}

		if (distSolid[current.x][current.y] == maxDistance) {
			continue;
		}

		for (int i = 0; i < HERO_MOVE_DIRS; i++) {
			intp next = current + HERO_MOVE_DIR[i];

			if (next.x < 0 || next.x >= map->colCount || next.y < 0 || next.y >= map->rowCount) {
				continue;
			}
			if (tileVisible[next.x][next.y] && isAccessible(next) && distSolid[next.x][next.y] < 0) {
				bfsQueue.push(next);
				distSolid[next.x][next.y] = distSolid[current.x][current.y] + 1;
				backSolid[next.x][next.y] = current;
			}
		}
	}

	// a bfs ignoring everything but walls and visibility to see what is reachable
	memset(distGhost, -1, sizeof distGhost);
	distGhost[origin.x][origin.y] = 0;
	while (!bfsQueue.empty()) {
		bfsQueue.pop();
	}
	bfsQueue.push(origin);
	backGhost[origin.x][origin.y] = intp(-1, -1); // to be able to find the origin even if using a wrong hero
	while (!bfsQueue.empty()) {
		intp current = bfsQueue.front();
		bfsQueue.pop();

		if (distGhost[current.x][current.y] == maxDistance) {
			continue;
		}

		for (int i = 0; i < HERO_MOVE_DIRS; i++) {
			intp next = current + HERO_MOVE_DIR[i];

			if (next.x < 0 || next.x >= map->colCount || next.y < 0 || next.y >= map->rowCount) {
				continue;
			}
			if (tileVisible[next.x][next.y] && isAccessible(next, true) && distGhost[next.x][next.y] < 0) {
				bfsQueue.push(next);
				distGhost[next.x][next.y] = distGhost[current.x][current.y] + 1;
				backGhost[next.x][next.y] = current;
				if (isBuilding(map->getObject(next))) {
					reachableBuildings.insert(next);
				}
				else if (isItem(map->getObject(next))) {
					reachableItems.insert(next);
				}
			}
		}
	}
}

vector<intp> Pathfinder::getPathTo(intp target, bool isGhost) {
	vector<intp> path;
	// if the target tile is not reachable
	if (distGhost[target.x][target.y] == -1 || (!isGhost && distSolid[target.x][target.y] == -1)) {
		return path; // empty
	}

	intp curr = target;
	while (curr != intp(-1, -1)) {
		path.push_back(curr);
		if (isGhost) {
			curr = backGhost[curr.x][curr.y];
		}
		else {
			curr = backSolid[curr.x][curr.y];
		}
	}

	reverse(path.begin(), path.end());
	return path;
}

bool Pathfinder::isAccessible(intp location, bool isGhost) {
	if (location.x < 0 || location.y < 0 || location.x >= map->colCount || location.y >= map->rowCount) {
		return false;
	}
	MapObject* tempObject = map->getObject(location);
	if (tempObject->isBlocking() && (!isGhost || (tempObject->objectType != ITEM && tempObject->objectType != CREATURE))) {
		return false;
	}
	if (GameLogic::instance().tileHasHero(location) && !isGhost) {
		return false;
	}

	return true;
}

std::vector<intp> Pathfinder::getReachableTiles(int maxDistance) {
	std::vector<intp> ret;
	for (int i = 0; i < map->colCount; i++) {
		for (int j = 0; j < map->rowCount; j++) {
			if (distSolid[i][j] >= 0 && distSolid[i][j] <= maxDistance) {
				ret.push_back(intp(i, j));
			}
		}
	}

	return ret;
}

int Pathfinder::countInvisibleAround(intp tile, int radius) {
	int ret = 0;
	for (int dist = 0; dist <= radius; dist++) {
		for (int x = 0; x <= dist; x++) {
			for (int y = 0; y <= dist - x; y++) {
				if (tile.x - x >= 0) {
					if (tile.y - y >= 0 && !tileVisible[tile.x - x][tile.y - y]) {
						ret++;
					}
					if (tile.y + y < map->rowCount && !tileVisible[tile.x - x][tile.y + y]) {
						ret++;
					}
				}
				if (tile.x + x < map->colCount) {
					if (tile.y - y >= 0 && !tileVisible[tile.x + x][tile.y - y]){
						ret++;
					}
					if (tile.y + y < map->rowCount && !tileVisible[tile.x + x][tile.y + y]) {
						ret++;
					}
				}
			}
		}
	}
	return ret;
}

bool Pathfinder::isBuilding(MapObject* object) {
	if (object->objectType == MINE) {
		return true;
	}

	return false;
}

bool Pathfinder::isItem(MapObject* object) {
	if (object->objectType == RESOURCE) {
		return true;
	}

	return false;
}