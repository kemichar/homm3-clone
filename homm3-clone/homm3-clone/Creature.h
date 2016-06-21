#pragma once

#include <string>
#include <sstream>
#include <vector>
#include "Utility.h"
#include "FactionMember.h"

/*
	This class represents the creatures used in combat. For creature objects
	visible on the map see MOCreature.
*/
class Creature : public FactionMember {

public:
	Creature();
	Creature(std::string _name, int _count = 1, int _factionId = 0);
	Creature(std::string _name, int _count, FactionMember* member);
	Creature(std::string _name, std::string _infoString, int _count = 1, int _factionId = 0);
	Creature(Creature* _original, int _count = 1, int _factionId = 0);
	
	void refresh();
	void takeDamageFrom(Creature *opponentStack);
	void draw(float size);
	float universalPower();

	std::vector<std::string> getDescription();

	std::string name;
	int count;
	int level;
	int baseDamageMin;
	int baseDamageMax;
	int attack;
	int defense;
	int speed;
	int maxHealth;
	int health; // health of the first creature on the stack
	int experience;
	int growth;
	int cost;
	std::string plural;

	intp combatPos;

private:
	std::string infoString;
	std::vector<std::string> description;
};