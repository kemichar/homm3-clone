#pragma once

#include <glm/glm.hpp>
#include <GL/glut.h>
#include <vector>

using namespace glm;
using namespace std;

class Camera {

public:

	enum Mode {NORMAL, ORTHO, ORTHO2D};

	Camera() {}
	Camera(float* _offset, vec3 _clearColor = vec3(1, 1, 1), vec3 _pos = vec3(0, 0, 1), vec3 _speed = vec3(0, 0, 0), float _fovFactor = 0.2f)
		: clearColor(_clearColor), pos(_pos), speed(_speed), fovFactor(_fovFactor) {

		for (int i = 0; i < 4; i++) {
			offset[i] = _offset[i];
		}
	}

	void windowResized(int _width, int _height) {
		windowWidth = _width;
		windowHeight = _height;

		aspectRatio = windowWidth * (offset[2] - offset[0]) / (windowHeight * (offset[3] - offset[1]));

		/*calculateProjection();
		calculateModelview();*/
		calculateViewport();
	}

	void activateView(Mode viewMode = NORMAL, bool clear = true) {
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

	vec3 viewToWorldPoint(int x, int y, Mode viewMode = NORMAL) {
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

	// TODO speed up (precalculate distances)
	vec2 worldToViewPoint(vec3 pos, Mode viewMode = NORMAL) {
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

	void addRotation(vec4 rotation) {
		rotations.push_back(rotation);
	}

	void rotate() {
		for (vec4 rotation : rotations) {
			glRotatef(rotation.x, rotation.y, rotation.z, rotation.w);
		}
	}

	GLdouble* getProjection(Mode viewMode = NORMAL){
		calculateProjection(viewMode);
		return projection;
	}

	GLdouble* getModelview(Mode viewMode = NORMAL) {
		calculateModelview(viewMode);
		return modelview;
	}

	GLint* getViewport() {
		return viewport;
	}

	void setFov(float _fovFactor) {
		fovFactor = _fovFactor;
	}

	// the camera position in 3rd space
	vec3 pos;
	vec3 speed;

private:
	void calculateProjection(Mode viewMode = NORMAL) {
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

	void calculateModelview(Mode viewMode = NORMAL) {
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

	void calculateViewport() {
		viewport[0] = windowWidth * offset[0];
		viewport[1] = windowHeight * offset[1];
		viewport[2] = windowWidth * (offset[2] - offset[0]) + 1;
		viewport[3] = windowHeight * (offset[3] - offset[1]) + 1;
	}

	int windowWidth;
	int windowHeight;
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