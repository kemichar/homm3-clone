#include "Creature.h"
#include "Resources.h"

Creature::Creature() {
}

Creature::Creature(string _name, int _count, int _factionId) :
	name(_name), count(_count), FactionMember(_factionId) {
	Creature* original = Resources::creatureData.at(_name);

	level = original->level;
	baseDamageMin = original->baseDamageMin;
	baseDamageMax = original->baseDamageMax;
	attack = original->attack;
	defense = original->defense;
	speed = original->speed;
	maxHealth = original->maxHealth;
	experience = original->experience;
	growth = original->growth;
	cost = original->cost;
	plural = original->plural;
}

Creature::Creature(string _name, int _count, FactionMember * member) :
	Creature(_name, _count, member->getFactionId()) {
}

Creature::Creature(string _name, string _infoString, int _count, int _factionId) :
	name(_name), infoString(_infoString), count(_count), FactionMember(_factionId) {
	stringstream ss(_infoString);
	string attribute;
	while (ss >> attribute) {
		if (attribute == "level") {
			int value;
			ss >> value;
			level = value;
		}
		else if (attribute == "baseDamageMin") {
			int value;
			ss >> value;
			baseDamageMin = value;
		}
		else if (attribute == "baseDamageMax") {
			int value;
			ss >> value;
			baseDamageMax = value;
		}
		else if (attribute == "attack") {
			int value;
			ss >> value;
			attack = value;
		}
		else if (attribute == "defense") {
			int value;
			ss >> value;
			defense = value;
		}
		else if (attribute == "speed") {
			int value;
			ss >> value;
			speed = value;
		}
		else if (attribute == "maxHealth") {
			int value;
			ss >> value;
			maxHealth = value;
		}
		else if (attribute == "experience") {
			int value;
			ss >> value;
			experience = value;
		}
		else if (attribute == "growth") {
			int value;
			ss >> value;
			growth = value;
		}
		else if (attribute == "cost") {
			int value;
			ss >> value;
			cost = value;
		}
		else if (attribute == "plural") {
			string value;
			ss >> value;
			plural = value;
		}
		else {
			string value;
			ss >> value;
		}
	}
}

Creature::Creature(Creature * _original, int _count, int _factionId) :
	name(_original->name), count(_count), FactionMember(_factionId) {

	level = _original->level;
	baseDamageMin = _original->baseDamageMin;
	baseDamageMax = _original->baseDamageMax;
	attack = _original->attack;
	defense = _original->defense;
	speed = _original->speed;
	maxHealth = _original->maxHealth;
	experience = _original->experience;
	growth = _original->growth;
	cost = _original->cost;
	plural = _original->plural;
}

void Creature::refresh() {
	health = maxHealth;
}

void Creature::takeDamageFrom(Creature * opponentStack) {
	string report = "";
	if (opponentStack->count == 1) {
		report += "1 " + opponentStack->name + " attacks, ";
	}
	else {
		report += to_string(opponentStack->count) + " " + opponentStack->plural + " attack, ";
	}

	float attackModifier = mmax(0, opponentStack->attack - defense) * 0.05f;
	float defenseModifier = mmax(0, defense - opponentStack->attack) * 0.025f;
	float baseDamage =
		rand() % (opponentStack->baseDamageMax - opponentStack->baseDamageMin + 1)
		+ opponentStack->baseDamageMin;
	int totalDamage = baseDamage * opponentStack->count * (1 + attackModifier) * (1 - defenseModifier);

	report += "dealing " + to_string(totalDamage) + " damage.";

	count -= totalDamage / maxHealth;
	health -= totalDamage % maxHealth;
	if (health <= 0) {
		count--;
		health += maxHealth;
	}
	if (count < 0) {
		count = 0;
	}

	cout << "Combat: " << report << endl;
}

void Creature::draw(float size) {
	glColor3f(COLORS[factionId].r, COLORS[factionId].g, COLORS[factionId].b);

	if (Resources::modelData.find(this->name) != Resources::modelData.end()) {
		glmDraw(Resources::modelData[this->name], GLM_SMOOTH | GLM_COLOR, 1);
	}
	else {
		glutWireCone(size / 3, size, 10, 10);
	}
}

float Creature::universalPower() {
	return (baseDamageMin + baseDamageMax) / 2.f * count;
}

vector<string> Creature::getDescription() {
	description.clear();

	if (count == 1) {
		description.push_back("  " + name);
	}
	else {
		description.push_back("  " + plural);
	}
	description.push_back("\n");
	description.push_back("Attack: " + to_string(attack));
	description.push_back("Defense: " + to_string(defense));
	description.push_back("Damage: " + to_string(baseDamageMin) + "-" + to_string(baseDamageMax));
	description.push_back("Speed: " + to_string(speed));
	description.push_back("Health: " + to_string(health) + '/' + to_string(maxHealth));

	return description;
}