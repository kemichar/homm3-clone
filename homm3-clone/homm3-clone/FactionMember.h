#pragma once

class FactionMember {

public:
	FactionMember(int _factionId = 0);

	int getFactionId();

protected:
	int factionId;
};