#pragma once

#include <string>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>
#include "FactionMember.h"

using namespace std;
using namespace glm;

/*
	This class represents the creatures used in combat. For creature objects
	visible on the map see Creature.
*/
class Creature : public FactionMember {

public:
	Creature();
	Creature(string _name, int _count = 1, int _factionId = 0);
	Creature(string _name, int _count, FactionMember* member);
	Creature(string _name, string _infoString, int _count = 1, int _factionId = 0);
	
	void refresh();
	void takeDamageFrom(Creature *opponentStack);
	void draw(float size);

	vector<string> getDescription();

	string name;
	int count;
	int attack;
	int defense;
	int speed;
	int attackRange;
	int moveRange;
	int maxHealth;
	int health; // health of the first creature on the stack

	vec2 combatPos;

private:
	string infoString;
	vector<string> description;
};