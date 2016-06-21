#include "MapObject.h"

MapObject::MapObject(intp _pos, int _factionId)
	: pos(_pos),  FactionMember(_factionId){
	model = nullptr;
	objectType = EMPTY;
	hero = nullptr;
	isControllable = false;
}

MapObject::~MapObject() {
	if (hero != nullptr) {
		delete hero;
	}
}

void MapObject::setControl(int _factionId) {
	factionId = _factionId;
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

void MapObject::draw(float size, bool mapDependency) {
	if (model != nullptr) {
		glmDraw(model, GLM_COLOR | GLM_SMOOTH, 0);
	}
}