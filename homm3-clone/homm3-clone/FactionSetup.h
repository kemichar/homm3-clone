#pragma once

#include "Constants.h"
#include "FactionMember.h"

class FactionSetup {

public:
	FactionSetup() {
		memset(usedFactions, 0, sizeof usedFactions);

		memset(allies, 0, sizeof allies);
		for (int i = 0; i < FACTIONS_MAX_COUNT; i++)
			allies[i][i] = true;
	}

	int areAllies(int factionA, int factionB) {
		if (factionA < 0 || factionA > FACTIONS_MAX_COUNT ||
			factionB < 0 || factionB > FACTIONS_MAX_COUNT) {
			return -1;
		}

		usedFactions[factionA] = usedFactions[factionB] = true;

		return allies[factionA][factionB];
	}

	int areAllies(FactionMember memberA, FactionMember memberB) {
		return areAllies(memberA.getFactionId(), memberB.getFactionId());
	}

	/*
		Receives two faction identifiers and sets their affiliation to the boolean
		value provided. False represents enemies, true represents allies. Returns
		the set affiliation, or -1 if invalid identifiers are provided.
	*/
	int setAllies(int factionA, int factionB, bool allied) {
		if (factionA < 0 || factionA > FACTIONS_MAX_COUNT ||
			factionB < 0 || factionB > FACTIONS_MAX_COUNT) {
			return -1;
		}

		usedFactions[factionA] = usedFactions[factionB] = true;

		return allies[factionA][factionB] = allied;
	}

	int newFactionID() {
		for (int i = 0; i < FACTIONS_MAX_COUNT; i++) {
			if (!usedFactions[i]) {
				usedFactions[i] = true;
				return i;
			}
		}

		return -1;
	}

private:
	bool allies[FACTIONS_MAX_COUNT][FACTIONS_MAX_COUNT];
	bool usedFactions[FACTIONS_MAX_COUNT];
};