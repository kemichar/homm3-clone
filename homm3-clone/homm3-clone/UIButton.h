#pragma once

#include <stdlib.h>
#include <GL/glut.h>
#include "Utility.h"
#include "GameLogic.h"

class UIButton {

public:
	typedef void (GameLogic::*actionFunc)();

	UIButton(vec2 _dimension, std::string _text, actionFunc _action) :
		dimension(_dimension), text(_text), action(_action) {
	}

	vec2 getDimension(){
		return dimension;
	}

	virtual void activate();
	virtual void draw();

private:
	vec2 dimension; // width, height
	std::string text;
	actionFunc action;
};