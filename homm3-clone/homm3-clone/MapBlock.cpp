#include "MapBlock.h"

MapBlock::MapBlock(string _properties, vector<string> _data) : properties(_properties), data(_data) {
	height = _data.size();
	width = _data[0].size();
}

MapBlock::MapBlock(vector<string> _everything) : properties(_everything[0]) {
	height = _everything.size() - 1;
	width = _everything[1].size();
	for (int i = 1; i < _everything.size(); i++) {
		data.push_back(_everything[i]);
	}
}

bool MapBlock::match(string requirements) {
	if (properties.size() != requirements.size()) {
		// DEBUG
		printf("Block properties size and requirements size don't match.\n");
		return false;
	}
	for (int i = 0; i < properties.size(); i++) {
		if (properties[i] != requirements[i] && requirements[i] != 'x') {
			return false;
		}
	}
	return true;
}