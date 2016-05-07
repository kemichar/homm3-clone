#pragma once

#include <map>
#include <string>
#include "Utility.h"
#include "Constants.h"
#include "Creature.h"
#include "MapBlock.h"
#include "objglm/objglm.h"

using namespace std;

class Resources {

public:
	static void loadCreatures();
	static void loadMapBlocks();
	static void loadModels();
	static vector<MapBlock*> getBlocksWith(string properties);

	static vector<MapBlock*> blockData;
	static map<string, Creature*> creatureData;
	static map<string, GLMmodel*> modelData;
};