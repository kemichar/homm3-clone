#pragma once

#include <glm/glm.hpp>
#include "Constants.h"
#include "FactionMember.h"

class FactionSetup {

public:
	int areAllies(int factionA, int factionB);

	int areAllies(FactionMember memberA, FactionMember memberB);

	/*
		Receives two faction identifiers and sets their affiliation to the boolean
		value provided. False represents enemies, true represents allies. Returns
		the set affiliation, or -1 if invalid identifiers are provided.
	*/
	int setAllies(int factionA, int factionB, bool allied);

	int newFactionID(bool isBot = false);

	glm::vec3 getFactionColor(FactionMember* member);

	glm::vec3 getFactionColor(int factionId);

	static FactionSetup& instance();
	FactionSetup(FactionSetup const&) = delete;
	void operator = (FactionSetup const &) = delete;

	bool isAI[FACTIONS_MAX_COUNT];

private:
	FactionSetup();;

	bool allies[FACTIONS_MAX_COUNT][FACTIONS_MAX_COUNT];
	bool usedFactions[FACTIONS_MAX_COUNT];
};