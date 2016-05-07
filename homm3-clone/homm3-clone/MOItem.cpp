#include "MOItem.h"

MOItem::MOItem(intp _pos, Map * _map) : MapObject(_pos) {
	mapInstance = _map;
	objectCode = 2;
	objectName = "Item";
}

bool MOItem::isBlocking() {
	return false;
}

bool MOItem::isHolding() {
	return true;
}

void MOItem::interact() {
	// inherit and add behaviour
	mapInstance->removeObject(pos);
}

void MOItem::draw(float size) {
	glPushMatrix();

	glColor3f(0.2, 0.2, 0.6);
	glutSolidSphere(size / 8, 20, 20);
	//glutSolidSphere(size / 20, 20, 20);

	glPopMatrix();
}