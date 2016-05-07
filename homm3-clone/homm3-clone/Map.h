#pragma once

#include <ctime>
#include "Constants.h"
#include "MapObject.h"
#include "Utility.h"
#include "BIT.h"

class Map {

public:
	enum Blueprint {
		UNKNOWN, EMPTY, WALL, PATH, CREATURE, ITEM, MINE, TREE, PART
	};

	Map();
	Map(int _colCount, int _rowCount);

	void fillMap();
	void testFillMap(int zones = 5);
	void clearMap();

	void removeObject(intp location);
	MapObject* getObject(intp location);
	MapObject* getFirstThreat(intp location);
	bool isThreatened(intp location);

	vector<intp> threats[MAP_MAX_COLS][MAP_MAX_ROWS];

	// TODO move to private, just debugging here
	BIT* dataTree;

	// stores the map generation data and is used to generate map objects
	int blueprint[MAP_MAX_COLS][MAP_MAX_ROWS];
private:
	void spreadThreat(intp pos);
	void removeThreat(intp pos);

	MapObject* mapObjects[MAP_MAX_COLS][MAP_MAX_ROWS];
	int colCount;
	int rowCount;

	// map generation data
	vector<intp> zoneOrigins;
	vector<intp> boundaries[MGEN_ZONE_LIMIT][MGEN_ZONE_LIMIT];
	vector<intp> subzoneOrigins[MGEN_ZONE_LIMIT];
	vector<intp> subBoundaries[MGEN_ZONE_LIMIT][MGEN_SUBZONE_LIMIT][MGEN_SUBZONE_LIMIT];
	// tile zone identifiers; if zoneId >= zones denotes a boundary tile
	int zoneId[MAP_MAX_COLS][MAP_MAX_ROWS];
	int zoneCount[MGEN_ZONE_LIMIT];
	int subzoneId[MAP_MAX_COLS][MAP_MAX_ROWS];
	// temporary
	intp tempData[MAP_MAX_COLS][MAP_MAX_ROWS];
	int tempFlags[MAP_MAX_COLS][MAP_MAX_ROWS];
};