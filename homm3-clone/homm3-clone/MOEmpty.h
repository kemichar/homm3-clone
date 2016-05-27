#pragma once

#include "MapObject.h"

class MOEmpty : public MapObject {

public:
	MOEmpty(intp _pos);

	void interact() override;

	bool isBlocking() override;

	bool isHolding() override;
};