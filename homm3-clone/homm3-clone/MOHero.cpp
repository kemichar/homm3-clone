#include "MOHero.h"
#include "GameLogic.h"

MOHero::MOHero(intp _pos, int _factionId) 
	: MapObject(_pos, _factionId){
	hero = new Hero(_factionId, true);
}


void MOHero::draw(float size) {
	vec3 factionColor = COLORS[factionId];
	glColor3f(factionColor.r, factionColor.g, factionColor.b);
	glutSolidCone(0.1, 0.2, 5, 5);
}

void MOHero::interact() {
}

void MOHero::moveTo(intp target, int cost) {
	pos = target;
	hero->movementPoints -= cost;
	// CHECK is this a bad idea
	GameLogic::instance().interact(this);
}