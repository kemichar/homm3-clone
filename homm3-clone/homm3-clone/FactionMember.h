#pragma once

class FactionMember {
public:
	FactionMember(int _factionId = 0) : factionId(_factionId){
	}

	int getFactionId() {
		return factionId;
	}

protected:
	int factionId;
};