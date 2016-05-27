#include "MapObject.h"

MapObject::MapObject(intp _pos, int _factionId)
	: pos(_pos),  FactionMember(_factionId){
	model = nullptr;
	objectType = EMPTY;
	hero = nullptr;
}

MapObject::~MapObject() {
	if (hero != nullptr) {
		delete hero;
	}
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

void MapObject::draw(float size) {
	if (model != nullptr) {
		drawModel(model);
	}
}