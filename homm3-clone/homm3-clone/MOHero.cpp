#include "MOHero.h"
#include "GameLogic.h"
#include "CombatLogic.h"

MOHero::MOHero(intp _pos, int _factionId) 
	: MapObject(_pos, _factionId){
	hero = new Hero(_factionId, true);
}


void MOHero::draw(float size, bool mapDependency) {
	vec3 factionColor = COLORS[factionId];
	glColor3f(factionColor.r, factionColor.g, factionColor.b);
	glutSolidCone(0.1, 0.2, 5, 5);
}

void MOHero::interact() {
	MOHero* otherHeroObject = GameLogic::instance().getCurrentPlayer()->getCurrentHero();
	if (otherHeroObject->getFactionId() != factionId) {
		CombatLogic::instance().setupCombat(otherHeroObject, this);
	}
	else {
		// TODO add friendly hero interaction (army rearrangement etc.)
	}
}

void MOHero::moveTo(intp target, int cost) {
	pos = target;
	hero->movementPoints -= cost;
	// CHECK is this a bad idea
	GameLogic::instance().interact(this);
}