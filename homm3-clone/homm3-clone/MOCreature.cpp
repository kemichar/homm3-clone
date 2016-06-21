#include "MOCreature.h"
#include "GameLogic.h"

MOCreature::MOCreature(intp _pos, std::string _creatureName, std::vector<int> _counts)
	: MapObject(_pos) {
	hero = new Hero(0, false);
	for (int i = 0; i < (int)_counts.size() && i < HERO_UNIT_SLOTS; i++) {
		hero->creatures[i] = new Creature(_creatureName, _counts[i]);
	}

	objectType = CREATURE;
}

MOCreature::MOCreature(intp _pos, std::string _creatureName, int _count)
	: MapObject(_pos) {
	hero = new Hero(0, false);
	hero->creatures[0] = new Creature(_creatureName, _count);

	objectType = CREATURE;
}

MOCreature::MOCreature(intp _pos, Creature * _creatureOriginal, int _count)
	: MapObject(_pos) {
	hero = new Hero(0, false);
	hero->creatures[0] = new Creature(_creatureOriginal, _count);

	objectType = CREATURE;
}

bool MOCreature::isBlocking() {
	return true;
}

bool MOCreature::isHolding() {
	return true;
}

void MOCreature::interact() {
	// no way to interact because this object is blocking
	// TODO might change? interact from a distance?
}

void MOCreature::draw(float size, bool mapDependency) {
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		if (hero->creatures[i] != nullptr && hero->creatures[i]->count > 0) {
			hero->creatures[i]->draw(size);
			return;
		}
	}

	printf("WARNING: Creatures not assigned to %d %d MOCreature object.\n", pos.x, pos.y);
	glColor3f(1, 0, 0);
	glutSolidCube(size);
}
