#include "MapObject.h"

MapObject::MapObject(intp _pos, floatp _modelOffset)
	: pos(_pos), modelOffset(_modelOffset){
	model = nullptr;
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		creatures[i] = nullptr;
	}
}

MapObject::~MapObject() {
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		delete creatures[i];
	}
}

int MapObject::creatureCount() {
	if (!hasCreatures()) {
		return 0;
	}

	int ret = 0;
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		if (creatures[i] != nullptr && creatures[i]->count > 0) {
			ret++;
		}
	}
	return ret;
}

void MapObject::setModel(GLMmodel * _model) {
	model = _model;
}

bool MapObject::isBlocking() {
	return false;
}

bool MapObject::isHolding() {
	return false;
}

bool MapObject::hasCreatures() {
	return false;
}

void MapObject::interact() {
	// intentionally empty
}

void MapObject::draw(float size) {
	if (model != nullptr) {
		glTranslatef(modelOffset.x * size / 2, modelOffset.y * size / 2, 0);
		glRotatef(90, 1, 0, 0);
		glTranslatef(0, size / 2, 0);
		drawModel(model);
	}
}

void MapObject::onCombatEnd(CombatResult result) {
	// inherit and modify
}
