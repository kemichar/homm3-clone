#pragma once

#include <glm/glm.hpp>
#include "GameLogic.h"

class UIButton {

public:
	typedef void (GameLogic::*actionFunc)();

	UIButton(glm::vec2 _dimension, std::string _text, actionFunc _action);

	glm::vec2 getDimension();

	virtual void activate();
	virtual void draw();

private:
	glm::vec2 dimension; // width, height
	std::string text;
	actionFunc action;
};