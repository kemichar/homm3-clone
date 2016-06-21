#include "MOItem.h"
#include "GameLogic.h"

MOItem::MOItem(intp _pos)
	: MapObject(_pos) {
	objectType = ITEM;
}

bool MOItem::isBlocking() {
	return false;
}

bool MOItem::isHolding() {
	return true;
}

void MOItem::interact() {
	GameLogic::instance().map->removeObject(pos);
}

void MOItem::draw(float size, bool mapDependency) {
	glColor3f(0.2, 0.2, 0.6);
	glutSolidSphere(size / 8, 20, 20);
}