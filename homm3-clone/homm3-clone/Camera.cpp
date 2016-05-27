#include "Camera.h"

Camera::Camera() {
	unit = 1;
	unitRows = 1;
	unitCols = 1;
}

Camera::Camera(float * _offset, vec3 _clearColor, vec3 _pos, vec3 _speed,
	float _fovFactor, float _unit, int _unitRows, int _unitCols)
	: clearColor(_clearColor), pos(_pos), speed(_speed), fovFactor(_fovFactor),
	unit(_unit), unitRows(_unitRows), unitCols(_unitCols) {

	for (int i = 0; i < 4; i++) {
		offset[i] = _offset[i];
	}
}

void Camera::setGrid(intp dimensions) {
	unitCols = dimensions.x;
	unitRows = dimensions.y;
	unit = mmin(viewport[2] / (float)unitCols, viewport[3] / (float)unitRows);
}

void Camera::windowResized(int _width, int _height) {
	windowWidth = _width;
	windowHeight = _height;

	aspectRatio = windowWidth * (offset[2] - offset[0]) / (windowHeight * (offset[3] - offset[1]));

	/*calculateProjection();
	calculateModelview();*/
	calculateViewport();

	unit = mmin(viewport[2] / (float)unitCols, viewport[3] / (float)unitRows);
}

void Camera::activateView(Mode viewMode, bool clear) {
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (viewMode == NORMAL) {
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glFrustum(-aspectRatio * fovFactor, aspectRatio * fovFactor, -fovFactor, fovFactor, .2, 50.);
	}
	else if (viewMode == ORTHO) {
		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);
		glOrtho(0, viewport[2], 0, viewport[3], .2, 100);
	}
	else if (viewMode == ORTHO2D) {
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		gluOrtho2D(0, viewport[2], 0, viewport[3]);
	}


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (viewMode == NORMAL || viewMode == ORTHO) {
		glTranslatef(-pos.x, -pos.y, -pos.z);
		rotate();
	}
	else if (viewMode == ORTHO2D) {
		glTranslatef(-pos.x, -pos.y, 0);
	}

	if (clear) {
		glClearColor(clearColor.x, clearColor.y, clearColor.z, 0);
		glEnable(GL_SCISSOR_TEST);
		glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
	}

	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

vec3 Camera::viewToWorldPoint(int x, int y, Mode viewMode) {
	if (viewMode == ORTHO2D) {
		return vec3(x, y, 0);
	}
	else {
		x += viewport[0];
		y += viewport[1];

		double closeX, closeY, closeZ;
		gluUnProject(x, y, -1, getModelview(viewMode), getProjection(viewMode), getViewport(), &closeX, &closeY, &closeZ);
		double farX, farY, farZ;
		gluUnProject(x, y, 1, getModelview(viewMode), getProjection(viewMode), getViewport(), &farX, &farY, &farZ);

		return lineXYPlaneIntersection(vec3(closeX, closeY, closeZ), vec3(farX, farY, farZ));
	}
}

vec2 Camera::worldToViewPoint(vec3 pos, Mode viewMode) {
	if (viewMode == ORTHO2D) {
		return vec2(pos.x, pos.y);
	}
	else {
		double screenX = 0, screenY = 0, screenZ = 0;
		gluProject(pos.x, pos.y, pos.z, getModelview(viewMode), getProjection(viewMode), getViewport(),
			&screenX, &screenY, &screenZ);
		return vec2(screenX - viewport[0], screenY - viewport[1]);
	}
}

void Camera::addRotation(vec4 rotation) {
	rotations.push_back(rotation);
}

void Camera::rotate() {
	for (vec4 rotation : rotations) {
		glRotatef(rotation.x, rotation.y, rotation.z, rotation.w);
	}
}

GLdouble * Camera::getProjection(Mode viewMode) {
	calculateProjection(viewMode);
	return projection;
}

GLdouble * Camera::getModelview(Mode viewMode) {
	calculateModelview(viewMode);
	return modelview;
}

GLint * Camera::getViewport() {
	return viewport;
}

void Camera::setFov(float _fovFactor) {
	fovFactor = _fovFactor;
}

void Camera::calculateProjection(Mode viewMode) {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glLoadIdentity();
	if (viewMode == NORMAL) {
		glFrustum(-aspectRatio * fovFactor, aspectRatio * fovFactor, -fovFactor, fovFactor, .2, 100.);
	}
	else if (viewMode == ORTHO) {
		glOrtho(0, viewport[2], 0, viewport[3], .2, 100);
	}
	else if (viewMode == ORTHO2D) {
		gluOrtho2D(0, viewport[2], 0, viewport[3]);
	}
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void Camera::calculateModelview(Mode viewMode) {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();
	if (viewMode == ORTHO2D) {
		glTranslatef(-pos.x, -pos.y, 0);
	}
	else {
		glTranslatef(-pos.x, -pos.y, -pos.z);
		rotate();
	}
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

	glPopMatrix();
}

void Camera::calculateViewport() {
	viewport[0] = windowWidth * offset[0];
	viewport[1] = windowHeight * offset[1];
	viewport[2] = windowWidth * (offset[2] - offset[0]) + 1;
	viewport[3] = windowHeight * (offset[3] - offset[1]) + 1;
}