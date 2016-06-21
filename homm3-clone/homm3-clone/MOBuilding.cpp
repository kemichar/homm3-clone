#include "MOBuilding.h"

MOBuilding::MOBuilding(Map * map, intp _pos, int _factionId)
	: MapObject(_pos, _factionId) {
	modelOffset = floatp(0, 0);
	for (int offs = 1; pos.x - offs >= 0 && map->blueprint[pos.x - offs][pos.y] == PART; offs++) {
		modelOffset.x -= 0.5f;
	}
	for (int offs = 1; pos.y + offs < map->rowCount && map->blueprint[pos.x][pos.y + offs] == PART; offs++) {
		modelOffset.y += 0.5f;
	}
	for (int offs = 1; pos.x + offs < map->colCount && map->blueprint[pos.x + offs][pos.y] == PART; offs++) {
		modelOffset.x += 0.5f;
	}
	for (int offs = 1; pos.y - offs >= 0 && map->blueprint[pos.x][pos.y - offs] == PART; offs++) {
		modelOffset.y -= 0.5f;
	}
}

bool MOBuilding::isBlocking() {
	return false;
}

bool MOBuilding::isHolding() {
	return true;
}

void MOBuilding::draw(float size, bool mapDependency) {
	if (model == nullptr) {
		glPushMatrix();
		if (modelOffset != floatp(0, 0)) {
			glTranslatef(modelOffset.x * size, modelOffset.y * size, 0);
			glColor3f(0.8, 0.4, 0.8);
			glutSolidTorus(size / 2, size / 2, 10, 10);
		}
		else {
			glColor3f(0.8, 0.4, 0.8);
			glutSolidTorus(size / 4.8, size / 5, 10, 10);
		}
		glPopMatrix();
	}
	else {
		if (mapDependency) {
			glTranslatef(modelOffset.x * size, modelOffset.y * size, size / 2);
		}

		// TEMP COLORING
		if (isControllable) {
			glColor3f(COLORS[factionId].r, COLORS[factionId].g, COLORS[factionId].b);
		}
		glmDraw(model, GLM_COLOR | GLM_SMOOTH, (int)isControllable);
	}
}