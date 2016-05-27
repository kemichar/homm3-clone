#pragma once

#include <vector>
#include <set>
#include "Utility.h"
#include "Constants.h"
#include "Map.h"
#include "MapObject.h"

class Pathfinder {

public:
	/*struct Node {
		Node(intp _pos, intp _last, int _dist)
			: pos(_pos), last(_last), dist(_dist) {
		}

		intp pos;
		intp last;
		int dist;
	};*/

	Pathfinder();

	void makeVisibleAround(intp tile, int radius);

	void findPaths(intp origin, int maxDistance = oo);

	std::vector<intp> getPathTo(intp target, bool isGhost = false);

	bool isAccessible(intp location, bool isGhost = false);

	std::vector<intp> getReachableTiles(int maxDistance = oo);
	int countInvisibleAround(intp tile, int radius = 5);

	bool tileVisible[MAP_MAX_COLS][MAP_MAX_ROWS];
	int distSolid[MAP_MAX_COLS][MAP_MAX_ROWS];
	int distGhost[MAP_MAX_COLS][MAP_MAX_ROWS];
	intp backSolid[MAP_MAX_COLS][MAP_MAX_ROWS];
	intp backGhost[MAP_MAX_COLS][MAP_MAX_ROWS];

	std::set<intp> reachableItems;
	std::set<intp> reachableBuildings;

private:
	Map* map;

	bool isBuilding(MapObject* object);
	bool isItem(MapObject* object);
};