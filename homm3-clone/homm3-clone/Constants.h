#ifndef CONSTANTS
#define CONSTANTS

#include <glm/glm.hpp>
#include <string>

using namespace glm;
using namespace std;

#define oo 1<<27
#define eps 0.000001f

/*
	All time related values are in milliseconds.
*/

// input

//#define INPUT_CHARACTER "bull.obj"  // NVM, triangles ftw
const string INPUT_PARTICLE_TEXTURE = "textures/cestica.bmp";
const string INPUT_ENEMY_TEXTURE = "textures/alien3.png";
const string INPUT_ENEMY_TEXTURE_2 = "textures/alien4.png";
const string INPUT_PLAYER_TEXTURE = "textures/ship.png";
const string INPUT_BACKGROUND_TEXTURE = "textures/space2.bmp";
const string INPUT_PROJECTILE_TEXTURE = "textures/projectile.png";
const string INPUT_CREATURE_DIRECTORY = "creatures/";
const string INPUT_CREATURE_LIST = "creature_list.txt";
const string INPUT_MAP_BLOCKS = "MapGenerator/blocks.txt";
const string INPUT_MODEL_DIRECTORY = "Models/";
const string INPUT_MODEL_LIST = "model_list.txt";

// application

const int INITIAL_WIDTH = 1500;
const int INITIAL_HEIGHT = 1000;
const int INITIAL_POS_X = 300;
const int INITIAL_POS_Y = 30;

// colors

enum Color{
	WHITE, BLUE, RED, VIOLET, PURPLE, GREEN, YELLOW, ORANGE
};
const int COLOR_COUNT = 8;
const vec3 COLORS[COLOR_COUNT] = { vec3(1, 1, 1), vec3(0, 0, 1), vec3(1, 0, 0), vec3(0.42, 0, 0.58), vec3(75, 0, 130), vec3(0, 1, 0), vec3(255, 255, 0), vec3(255, 127, 0) };

// particles

const int PART_MAX = 10000;
const float PART_SIZE = 0.2f;
const int PART_TTL = 250;
const int PART_EXPLOSION_COUNT = 20;

// combat

const int COMBAT_ROWS = 11;
const int COMBAT_COLS = 15;
const vec2 COMBAT_POS[6][6] = {
	{vec2(1, 5)},
	{vec2(1, 3), vec2(1, 7)},
	{vec2(1, 3), vec2(1, 5), vec2(1, 7)},
	{vec2(0, 2), vec2(1, 4), vec2(1, 6), vec2(0, 8)},
	{vec2(0, 1), vec2(1, 3), vec2(1, 5), vec2(1, 7), vec2(0, 9)},
	{vec2(1, 1), vec2(0, 3), vec2(1, 4), vec2(1, 6), vec2(0, 7), vec2(1, 9)}
};
const vec2 COMBAT_ATTACK_DIR[9] = {
	vec2(-1, -1), vec2(-1, 0), vec2(-1, 1),
	vec2(0, -1), vec2(0, 0), vec2(0, 1),
	vec2(1, -1), vec2(1, 0), vec2(1, 1)
};

// the first 4 are cardinal directions, the last 4 are diagonals
const vec2 COMBAT_MOVE_DIR[8] = {
	vec2(-1, 0), vec2(0, 1), vec2(1, 0), vec2(0, -1),
	vec2(-1, -1), vec2(-1, 1), vec2(1, 1), vec2(1, -1)
};

// map

const int MAP_MAX_ROWS = 250;
const int MAP_MAX_COLS = 250;
const vec2 MAP_THREAT_DIR[9] = {
	vec2(-1, -1), vec2(-1, 0), vec2(-1, 1),
	vec2(0, -1), vec2(0, 0), vec2(0, 1),
	vec2(1, -1), vec2(1, 0), vec2(1, 1)
};

// map generation parameters

const int MGEN_ZONE_LIMIT = 50;
const int MGEN_SUBZONE_LIMIT = 5;
const int MGEN_ORIGIN_CANDIDATES = 100;
const int MGEN_ZONE_CONNECTEDNESS = 1; // the maximum number of paths connecting each zone to each adjacent zone
const vec2 MGEN_BOUNDARY_BRUSH[8] = {
	vec2(-1, 0), vec2(0, 1), vec2(1, 0), vec2(0, -1),
	vec2(-1, -1), vec2(-1, 1), vec2(1, 1), vec2(1, -1)
};

// factions

const int FACTIONS_MAX_COUNT = 10;

// heroes

const int HERO_BASE_MOVE_POINTS = 100;
const int HERO_TILE_MOVE_COST = 10;
const int HERO_UNIT_SLOTS = 6; // don't change, volatile
const int HERO_MOVE_DIRS = 4;
const vec2 HERO_MOVE_DIR[8] = {
	vec2(-1, 0), vec2(0, 1), vec2(1, 0), vec2(0, -1),
	vec2(-1, -1), vec2(-1, 1), vec2(1, 1), vec2(1, -1)
};

#endif