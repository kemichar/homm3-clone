#ifndef _H_CONSTANTS_
#define _H_CONSTANTS_

#include <glm/glm.hpp>
#include <string>
// #include "Utility.h" TODO replace vec2 with intp

#define oo 1<<27
#define eps 0.000001f

// input

const std::string INPUT_BACKGROUND_TEXTURE = "textures/grass.png";
const std::string INPUT_COMBAT_BACKGROUND_TEXTURE = "textures/stone.png";
const std::string INPUT_CREATURE_DIRECTORY = "creatures/";
const std::string INPUT_CREATURE_LIST = "creature_list.txt";
const std::string INPUT_MAP_BLOCKS = "MapGenerator/blocks.txt";
const std::string INPUT_MODEL_DIRECTORY = "Models/";
const std::string INPUT_MODEL_LIST = "model_list.txt";
const std::string INPUT_CONFIG = "config.txt";

// application

const int INITIAL_WIDTH = 1500;
const int INITIAL_HEIGHT = 1000;
const int INITIAL_POS_X = 300;
const int INITIAL_POS_Y = 30;

// colors

enum Color{
	WHITE, BLUE, RED, VIOLET, PURPLE, GREEN, YELLOW, ORANGE, BROWN, GRAY
};
const int COLOR_COUNT = 10;
const glm::vec3 COLORS[COLOR_COUNT] = {
	glm::vec3(1, 1, 1), glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec3(0.42, 0, 0.58),
	glm::vec3(0.25, 0, 0.5), glm::vec3(0, 1, 0), glm::vec3(1, 1, 0), glm::vec3(1, 0.5, 0),
	glm::vec3(0.65, 0.16, 0.16), glm::vec3(0.55, 0.54, 0.54)
};

// particles

const int PART_MAX = 10000;
const float PART_SIZE = 0.2f;
const int PART_TTL = 250;
const int PART_EXPLOSION_COUNT = 20;

// combat

const int COMBAT_ROWS = 11;
const int COMBAT_COLS = 15;
const glm::vec2 COMBAT_POS[6][6] = {
	{ glm::vec2(1, 5) },
	{ glm::vec2(1, 3), glm::vec2(1, 7) },
	{ glm::vec2(1, 3), glm::vec2(1, 5), glm::vec2(1, 7) },
	{ glm::vec2(0, 2), glm::vec2(1, 4), glm::vec2(1, 6), glm::vec2(0, 8) },
	{ glm::vec2(0, 1), glm::vec2(1, 3), glm::vec2(1, 5), glm::vec2(1, 7), glm::vec2(0, 9) },
	{ glm::vec2(1, 1), glm::vec2(0, 3), glm::vec2(1, 4), glm::vec2(1, 6), glm::vec2(0, 7), glm::vec2(1, 9) }
};
const glm::vec2 COMBAT_ATTACK_DIR[9] = {
	glm::vec2(-1, -1), glm::vec2(-1, 0), glm::vec2(-1, 1),
	glm::vec2(0, -1), glm::vec2(0, 0), glm::vec2(0, 1),
	glm::vec2(1, -1), glm::vec2(1, 0), glm::vec2(1, 1)
};
// the first 4 are cardinal directions, the last 4 are diagonals
const glm::vec2 COMBAT_MOVE_DIR[8] = {
	glm::vec2(-1, 0), glm::vec2(0, 1), glm::vec2(1, 0), glm::vec2(0, -1),
	glm::vec2(-1, -1), glm::vec2(-1, 1), glm::vec2(1, 1), glm::vec2(1, -1)
};

// currency

enum Resource {
	GOLD, WOOD, ORE, CRYSTAL, _RESOURCE_END
};
const int RES_MIN_SPAWN[_RESOURCE_END] = { 500, 2, 2, 1 };
const int RES_MAX_SPAWN[_RESOURCE_END] = { 1500, 10, 10, 5 };
const int RES_MINE_GAIN[_RESOURCE_END] = { 1000, 2, 2, 1 };
const int RES_MINE_SPAWN_ORDER[6] = { WOOD, ORE, CRYSTAL, ORE, WOOD, GOLD };

// town buildings and external dwellings

enum BuildingType {
	HALL, DWELL_ONE, DWELL_TWO, DWELL_THREE, DWELL_FOUR, DWELL_FIVE,
	DWELL_SIX, DWELL_SEVEN, _BUILDINGTYPE_END
};
// TEMP costs, TODO move to config files etc.
const int BUILD_COST[_BUILDINGTYPE_END][_RESOURCE_END] = {
	{1500, 0, 0, 0}, {500, 0, 5, 0}, {1000, 5, 5, 0}, {1000, 0, 10, 0},
	{2000, 0, 10, 0}, {3000, 5, 5, 5}, {5000, 20, 5, 0}, {20000, 10, 10, 10}
};
const int CASTLE_HERO_BUY_COST = 2500;
const int CASTLE_MAX_GOLD_GAIN_LEVEL = 3;
const int CASTLE_GOLD_GAIN[CASTLE_MAX_GOLD_GAIN_LEVEL + 1] = {
	500, 1000, 2000, 4000
};
const int CASTLE_DAILY_BUILD_LIMIT = 1;

// map

enum MOType {
	UNKNOWN, EMPTY, WALL, PATH, CREATURE, ITEM, MINE, TREE, PART, RESOURCE, CASTLE
};

const int MAP_MAX_ROWS = 250;
const int MAP_MAX_COLS = 250;
const glm::vec2 MAP_THREAT_DIR[9] = {
	glm::vec2(-1, -1), glm::vec2(-1, 0), glm::vec2(-1, 1),
	glm::vec2(0, -1), glm::vec2(0, 0), glm::vec2(0, 1),
	glm::vec2(1, -1), glm::vec2(1, 0), glm::vec2(1, 1)
};

// map generation parameters

const int MGEN_ZONE_LIMIT = 50;
const int MGEN_SUBZONE_LIMIT = 5;
const int MGEN_ORIGIN_CANDIDATES = 100;
const int MGEN_ZONE_CONNECTEDNESS = 1; // the maximum number of paths connecting each zone to each adjacent zone
const glm::vec2 MGEN_BOUNDARY_BRUSH[8] = {
	glm::vec2(-1, 0), glm::vec2(0, 1), glm::vec2(1, 0), glm::vec2(0, -1),
	glm::vec2(-1, -1), glm::vec2(-1, 1), glm::vec2(1, 1), glm::vec2(1, -1)
};

// factions

const int FACTIONS_MAX_COUNT = 10;

// heroes

const int HERO_BASE_MOVE_POINTS = 15;
//const int HERO_TILE_MOVE_COST = 10;
const int HERO_UNIT_SLOTS = 6; // don't change, volatile
const int HERO_MOVE_DIRS = 4;
const glm::vec2 HERO_MOVE_DIR[8] = {
	glm::vec2(-1, 0), glm::vec2(0, 1), glm::vec2(1, 0), glm::vec2(0, -1),
	glm::vec2(-1, -1), glm::vec2(-1, 1), glm::vec2(1, 1), glm::vec2(1, -1)
};
const int MAX_HERO_COUNT = 8;

// creatures

const int CREATURES_MAX_LVL = 7;
const int CREATURE_SPAWN_MIN[7] = {
	19, 14, 10, 8, 5, 3, 1
};
const int CREATURE_SPAWN_MAX[7] = {
	49, 35, 27, 19, 14, 10, 5
};

// adventure AI

const float AI_AGGRESSION_FACTOR = 1.5f; // [0, 1] aggressive, <1, ...> defensive
const float AI_COMBAT_OPTIMISM = 0.2f; // the portion of his army the AI expects to keep in a winnable fight

// tooltips
const std::string DEV_HOTKEYS_TOOLTIP =
	"F1 - toggle fast AI turns\n"
	"F2 - remake map\n"
	"F3 - toggle autofocus\n"
	"F4 - toggle coordinates\n"
	"F5 - toggle grid\n"
	"F6 - toggle background\n"
	"F7 - toggle fog\n"
	"F8 - scroll fog owner\n"
	"F9 - toggle distance display\n"
	"F10 - toggle camera bounds\n"
	"x - tilt the view\n"
	"+ - add movement points"
;

#endif