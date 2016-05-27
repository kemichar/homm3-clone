#pragma once

#include <glm/glm.hpp>
#include <GL/glut.h>
#include <vector>
#include "Utility.h"

using namespace glm;
using namespace std;

class Camera {

public:

	enum Mode {NORMAL, ORTHO, ORTHO2D};

	Camera();
	Camera(float* _offset, vec3 _clearColor = vec3(1, 1, 1), vec3 _pos = vec3(0, 0, 1), vec3 _speed = vec3(0, 0, 0),
		float _fovFactor = 0.2f, float _unit = 1, int _unitRows = 1, int _unitCols = 1);

	void setGrid(intp dimensions);
	void windowResized(int _width, int _height);
	void activateView(Mode viewMode = NORMAL, bool clear = true);
	vec3 viewToWorldPoint(int x, int y, Mode viewMode = NORMAL);

	// TODO speed up (precalculate distances)
	vec2 worldToViewPoint(vec3 pos, Mode viewMode = NORMAL);

	void addRotation(vec4 rotation);

	void rotate();

	GLdouble* getProjection(Mode viewMode = NORMAL);

	GLdouble* getModelview(Mode viewMode = NORMAL);

	GLint* getViewport();

	void setFov(float _fovFactor);

	// the camera position in 3rd space
	vec3 pos;
	vec3 speed;
	float unit;
	int unitRows;
	int unitCols;

	int windowWidth;
	int windowHeight;

private:
	void calculateProjection(Mode viewMode = NORMAL);

	void calculateModelview(Mode viewMode = NORMAL);

	void calculateViewport();

	// used to calculate the camera's subwindow offset when multiplied by w/h
	// first two - bottom left subwindow point; last two - top right
	float offset[4];
	float aspectRatio;
	vec3 clearColor;
	GLdouble projection[16];
	GLdouble modelview[16];
	GLint viewport[4];
	vector<vec4> rotations;
	float fovFactor;

};