#pragma once

#include "MOItem.h"
#include "Currency.h"

class MOResource : public MOItem {

public:
	MOResource(Resource _type, intp _pos);
	MOResource(intp _pos);

	void draw(float size, bool mapDependency) override;
	void interact() override;

	vec3 tempColor;
	Currency wallet;
	Resource type;

};