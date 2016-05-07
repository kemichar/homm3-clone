#pragma once

#include <string>
#include <GL/glut.h>
#include "Utility.h"
#include "Creature.h"
#include "Constants.h"
#include "CombatResult.h"
#include "objglm/objglm.h"

using namespace glm;

class MapObject {

public:
	MapObject(intp _pos, floatp _modelOffset = floatp(0, 0));

	~MapObject();

	int creatureCount();
	void setModel(GLMmodel* _model);

	virtual bool isBlocking();
	virtual bool isHolding();
	virtual bool hasCreatures();
	virtual void interact();
	virtual void draw(float size);
	virtual void onCombatEnd(CombatResult result);

	intp pos;
	Creature* creatures[HERO_UNIT_SLOTS];

protected:
	int objectCode;
	std::string objectName;
	GLMmodel* model;
	floatp modelOffset;

private:

};