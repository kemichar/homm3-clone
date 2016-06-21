#include "UIButton.h"
#include <stdlib.h>
#include <GL/glut.h>
#include "GameLogic.h"
#include "Utility.h"

UIButton::UIButton(vec2 _dimension, std::string _text, actionFunc _action) :
	dimension(_dimension), text(_text), action(_action) {
}

vec2 UIButton::getDimension() {
	return dimension;
}

void UIButton::activate() {
	((GameLogic::instance().*action))();
}

void UIButton::draw() {
	glRectf(0, 0, dimension.x, dimension.y);

	glColor3f(0, 0, 1);
	glPushMatrix();
	glRasterPos2d(dimension.x / 5, dimension.y / 5);
	glDisable(GL_DEPTH_TEST);
	drawText(GLUT_BITMAP_HELVETICA_12, text);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
}
