#include "MOWall.h"

MOWall::MOWall(intp _pos, bool invisible)
	: MapObject(_pos) {
	if (invisible) {
		objectType = PART;
	}
	else {
		objectType = WALL;
	}
}

bool MOWall::isBlocking() {
	return true;
}

void MOWall::draw(float size) {
	if (objectType == PART) {
		return;
	}

	if (model == nullptr) {
		glColor3f(0.2, 0.2, 0.2);
		glutSolidCube(size * 0.7);
	}
	else {
		MapObject::draw(size);
	}
}

void MOWall::interact() {
	// this is a wall, there is no interaction
}
