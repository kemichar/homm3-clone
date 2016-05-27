#pragma once

#include <ctime>
#include "Constants.h"
#include "MapObject.h"
#include "Utility.h"
#include "BIT.h"

class Map {

public:
	Map();
	Map(int _colCount, int _rowCount);

	void fillMap();
	void testFillMap(int zones = 5);
	void clearMap();

	intp factionStartingZone(int factionId);

	void removeObject(intp location);
	MapObject* getObject(intp location);
	MapObject* getFirstThreat(intp location);
	bool isThreatened(intp location);

	vector<intp> threats[MAP_MAX_COLS][MAP_MAX_ROWS];

	// TODO move to private, just debugging here
	BIT* dataTree;

	// stores the map generation data and is used to generate map objects
	int blueprint[MAP_MAX_COLS][MAP_MAX_ROWS];

	int colCount;
	int rowCount;

private:
	void spreadThreat(intp pos);
	void removeThreat(intp pos);

	MapObject* mapObjects[MAP_MAX_COLS][MAP_MAX_ROWS];

	int zoneStartingFaction[MGEN_ZONE_LIMIT];

	// map generation data
	vector<intp> zoneOrigins;
	vector<float> zoneRadius;
	vector<intp> boundaries[MGEN_ZONE_LIMIT][MGEN_ZONE_LIMIT];
	vector<int> adjZones[MGEN_ZONE_LIMIT];

	// tile zone identifiers; if zoneId >= zones denotes a boundary tile
	int zoneId[MAP_MAX_COLS][MAP_MAX_ROWS];
	int zoneCount[MGEN_ZONE_LIMIT];
	int zoneMineCount[MGEN_ZONE_LIMIT];

	// temporary
	intp tempData[MAP_MAX_COLS][MAP_MAX_ROWS];
	int tempFlags[MAP_MAX_COLS][MAP_MAX_ROWS];
};