#include "MOMine.h"

MOMine::MOMine(int _type, Map * map, intp _pos, int _factionId)
	: MOBuilding(map, _pos, _factionId), type(_type) {
	objectType = MINE;
}

void MOMine::interact() {
	// if a player used to control this mine before it changed factions update him
	if (factionId > 0) {
		Player* oldPlayer = GameLogic::instance().getPlayerByFaction(factionId);
		oldPlayer->minesControlled[type]--;
	}

	Player* player = GameLogic::instance().getCurrentPlayer();
	factionId = player->getFactionId();
	player->minesControlled[type]++;
}

void MOMine::draw(float size) {
	if (model == nullptr) {
		glPushMatrix();
		glTranslatef(modelOffset.x * size, modelOffset.y * size, 0);
		glColor3f(COLORS[factionId].r, COLORS[factionId].g, COLORS[factionId].b);
		glutSolidTorus(size / 2, size / 2, 10, 10);
		glPopMatrix();
	}
	else {
		MOBuilding::draw(size);
	}
}
