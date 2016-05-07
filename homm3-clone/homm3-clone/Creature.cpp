#include "Creature.h"
#include "Resources.h"

Creature::Creature() {
}

Creature::Creature(string _name, int _count, int _factionId) :
	name(_name), count(_count), FactionMember(_factionId) {
	Creature* original = Resources::creatureData.at(_name);

	attack = original->attack;
	defense = original->defense;
	speed = original->speed;
	moveRange = original->moveRange;
	maxHealth = original->maxHealth;
	// TODO copy everything else
}

Creature::Creature(string _name, int _count, FactionMember * member) :
	Creature(_name, _count, member->getFactionId()) {
}

Creature::Creature(string _name, string _infoString, int _count, int _factionId) :
	name(_name), infoString(_infoString), count(_count), FactionMember(_factionId) {
	stringstream ss(_infoString);
	string attribute;
	while (ss >> attribute) {
		int value;
		ss >> value;
		if (attribute == "attack") {
			attack = value;
		}
		else if (attribute == "defense") {
			defense = value;
		}
		else if (attribute == "speed") {
			speed = value;
		}
		else if (attribute == "attackRange") {
			attackRange = value;
		}
		else if (attribute == "moveRange") {
			moveRange = value;
		}
		else if (attribute == "maxHealth") {
			maxHealth = value;
		}
	}
}

void Creature::refresh() {
	health = maxHealth;
}

void Creature::takeDamageFrom(Creature * opponentStack) {
	// TODO modify damage calculation?
	int totalDamage = (opponentStack->attack - defense) * opponentStack->count;

	count -= totalDamage / maxHealth;
	health -= totalDamage % maxHealth;
	if (health <= 0) {
		count--;
		health += maxHealth;
	}
	if (count < 0) {
		count = 0;
	}
}

void Creature::draw(float size) {
	glPushMatrix();
	glutWireCone(size / 3, size, 10, 10);
	glPopMatrix();
}

vector<string> Creature::getDescription() {
	description.clear();

	description.push_back("   " + name);
	description.push_back("\n");
	description.push_back("Attack: " + to_string(attack));
	description.push_back("Defense: " + to_string(defense));
	description.push_back("Speed: " + to_string(speed));
	description.push_back("Health: " + to_string(health) + '/' + to_string(maxHealth));

	return description;
}

inline bool operator< (const Creature& lhs, const Creature& rhs) {
	return lhs.speed < rhs.speed;
}

inline bool operator> (const Creature& lhs, const Creature& rhs) {
	return  operator< (rhs, lhs);
}