#pragma once

#include "MapObject.h"

class MOHero : public MapObject {

public:
	MOHero(intp _pos, int _factionId);

	virtual void draw(float size) override;

	virtual void interact() override;

	void moveTo(intp target, int cost);
};