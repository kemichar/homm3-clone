#include "FactionMember.h"

FactionMember::FactionMember(int _factionId) : factionId(_factionId) {
}

int FactionMember::getFactionId() {
	return factionId;
}
