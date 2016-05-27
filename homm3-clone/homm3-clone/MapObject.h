#pragma once

#include <string>
#include <GL/glut.h>
#include "Utility.h"
#include "Constants.h"
#include "objglm/objglm.h"
#include "FactionMember.h"
#include "Hero.h"

using namespace glm;

class MapObject : public FactionMember {

public:
	MapObject(intp _pos, int _factionId = 0);
	~MapObject();

	void setModel(GLMmodel* _model);

	virtual bool isBlocking();
	virtual bool isHolding();
	virtual void interact() = 0;
	virtual void draw(float size);

	intp pos;
	Hero* hero;
	MOType objectType;

protected:
	GLMmodel* model;

};