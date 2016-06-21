#include "FactionSetup.h"

int FactionSetup::areAllies(int factionA, int factionB) {
	if (factionA < 0 || factionA > FACTIONS_MAX_COUNT ||
		factionB < 0 || factionB > FACTIONS_MAX_COUNT) {
		return -1;
	}

	usedFactions[factionA] = usedFactions[factionB] = true;

	return allies[factionA][factionB];
}

int FactionSetup::areAllies(FactionMember memberA, FactionMember memberB) {
	return areAllies(memberA.getFactionId(), memberB.getFactionId());
}

/*
Receives two faction identifiers and sets their affiliation to the boolean
value provided. False represents enemies, true represents allies. Returns
the set affiliation, or -1 if invalid identifiers are provided.
*/

int FactionSetup::setAllies(int factionA, int factionB, bool allied) {
	if (factionA < 0 || factionA > FACTIONS_MAX_COUNT ||
		factionB < 0 || factionB > FACTIONS_MAX_COUNT) {
		return -1;
	}

	usedFactions[factionA] = usedFactions[factionB] = true;

	return allies[factionA][factionB] = allied;
}

int FactionSetup::newFactionID(bool isBot) {
	for (int i = 1; i < FACTIONS_MAX_COUNT; i++) {
		if (!usedFactions[i]) {
			usedFactions[i] = true;
			isAI[i] = isBot;
			return i;
		}
	}

	return -1;
}

glm::vec3 FactionSetup::getFactionColor(FactionMember * member) {
	return COLORS[member->getFactionId()];
}

glm::vec3 FactionSetup::getFactionColor(int factionId) {
	if (factionId >= FACTIONS_MAX_COUNT) {
		return COLORS[0];
	}

	return COLORS[factionId];
}

FactionSetup & FactionSetup::instance() {
	static FactionSetup GAMELOGIC_INSTANCE;

	return GAMELOGIC_INSTANCE;
}

FactionSetup::FactionSetup() {
	memset(usedFactions, 0, sizeof usedFactions);

	memset(allies, 0, sizeof allies);
	for (int i = 0; i < FACTIONS_MAX_COUNT; i++) {
		allies[i][i] = true;
	}

	isAI[0] = true;
}
