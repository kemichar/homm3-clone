#include "MOResource.h"
#include "GameLogic.h"

MOResource::MOResource(Resource _type, intp _pos)
	: MOItem(_pos), type(_type) {
	tempColor = wallet.resourceColor(_type);
	wallet[_type] = rand() % (RES_MAX_SPAWN[_type] - RES_MIN_SPAWN[_type] + 1) + RES_MIN_SPAWN[_type];
	objectType = RESOURCE;
}

MOResource::MOResource(intp _pos)
	: MOResource((Resource)(rand() % _RESOURCE_END), _pos) {
}

void MOResource::interact() {
	GameLogic::instance().getCurrentPlayer()->wallet[type] += wallet[type];
	GameLogic::instance().map->removeObject(pos);
}

void MOResource::draw(float size) {
	glColor3f(tempColor.r, tempColor.g, tempColor.b);
	glutSolidCube(size / 6);
}
