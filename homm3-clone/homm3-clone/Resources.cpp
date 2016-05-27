#include "Resources.h"

vector<MapBlock*> Resources::blockData;
map<string, Creature*> Resources::creatureData;
map<string, GLMmodel*> Resources::modelData;

void Resources::loadCreatures() {
	map<string, string> creatureFiles = readListedFiles(INPUT_CREATURE_DIRECTORY, INPUT_CREATURE_LIST);
	for (map<string, string>::iterator it = creatureFiles.begin(); it != creatureFiles.end(); it++) {
		creatureData[it->first] = new Creature(it->first, it->second);
	}
}

void Resources::loadMapBlocks() {
	vector<string> blocks = readFile(INPUT_MAP_BLOCKS);
	vector<string> current;
	for (string line : blocks) {
		if (line.empty()) {
			if (!current.empty()) {
				blockData.push_back(new MapBlock(current));
				current.clear();
			}
		}
		else if (line[0] != '#') {
			current.push_back(line);
		}
	}
}

void Resources::loadModels() {
	vector<string> modelList = readFile(INPUT_MODEL_DIRECTORY + INPUT_MODEL_LIST);
	vector<string> names;
	for (string line : modelList) {
		names = splitString(line);
		if (names.size() != 2) {
			continue;
		}
		GLMmodel* model = glmReadOBJ((INPUT_MODEL_DIRECTORY + names[1]).c_str());
		if (model == nullptr) {
			continue;
		}
		glmUnitize(model);
		glmVertexNormals(model, 90.0, false);
		glmScale(model, 0.3f);
		modelData[names[0]] = model;
		initModel(model);
	}
}

vector<Creature*> Resources::getCreaturesOfLevel(int level) {
	vector<Creature*> ret;

	for (auto &creature : creatureData) {
		if (creature.second->level == level) {
			ret.push_back(creature.second);
		}
	}

	return ret;
}

vector<MapBlock*> Resources::getBlocksWith(string properties) {
	vector<MapBlock*> ret;

	for (MapBlock* block : blockData) {
		if (block->match(properties)) {
			ret.push_back(block);
		}
	}

	return ret;
}
