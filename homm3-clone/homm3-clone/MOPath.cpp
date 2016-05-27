#include "MOPath.h"

MOPath::MOPath(Map * map, intp _pos)
	: MOEmpty(_pos) {
	tempShape = 0;
	if (pos.x > 0 && map->blueprint[pos.x - 1][pos.y] == PATH) {
		tempShape |= 1;
	}
	if (pos.x < map->colCount - 1 && map->blueprint[pos.x + 1][pos.y] == PATH) {
		tempShape |= 4;
	}
	if (pos.y > 0 && map->blueprint[pos.x][pos.y - 1] == PATH) {
		tempShape |= 2;
	}
	if (pos.y < map->rowCount - 1 && map->blueprint[pos.x][pos.y + 1] == PATH) {
		tempShape |= 8;
	}
}

// TEMP scribbling some paths

void MOPath::draw(float size) {
	glDisable(GL_LIGHTING);
	glColor3f(0.8, 0.8, 0.8);

	if (tempShape & 1) {
		glRectd(-size / 2, -size / 5, 0, size / 5);
	}
	if (tempShape & 2) {
		glRectd(-size / 5, -size / 2, size / 5, 0);
	}
	if (tempShape & 4) {
		glRectd(0, -size / 5, size / 2, size / 5);
	}
	if (tempShape & 8) {
		glRectd(-size / 5, 0, size / 5, size / 2);
	}

	glEnable(GL_LIGHTING);
}
