#include "MOMine.h"

MOMine::MOMine(int _type, Map * map, intp _pos, int _factionId)
	: MOBuilding(map, _pos, _factionId), type(_type) {
	objectType = MINE;
	isControllable = true;
}

void MOMine::interact() {
	Player* player = GameLogic::instance().getCurrentPlayer();
	if (factionId == player->getFactionId()) {
		return;
	}

	// if a player used to control this mine before it changed factions update him
	if (factionId > 0) {
		Player* oldPlayer = GameLogic::instance().getPlayerByFaction(factionId);
		oldPlayer->disownBuilding(pos);
		oldPlayer->minesControlled[type]--;
	}

	player->minesControlled[type]++;
	// TEMP
	factionId = player->getFactionId();
	player->buildingsControlled.push_back(pos);
}

void MOMine::draw(float size, bool mapDependency) {
	if (model == nullptr) {
		glPushMatrix();
		if (mapDependency) {
			glTranslatef(modelOffset.x * size, modelOffset.y * size, 0);
		}
		glColor3f(COLORS[factionId].r, COLORS[factionId].g, COLORS[factionId].b);
		glutSolidTorus(size / 2, size / 2, 10, 10);
		glPopMatrix();
	}
	else {
		MOBuilding::draw(size, mapDependency);
	}
}
