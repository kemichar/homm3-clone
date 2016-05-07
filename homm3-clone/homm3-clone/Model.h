#pragma once

#include <glm/glm.hpp>
#include <GL/glut.h>
#include <vector>
#include <string>

using namespace glm;
using namespace std;

class Model {

public:

	Model() {}
	~Model() {}

	bool loadFromFile(string inputFile);

	void addPoint(vec3 point);
	/*
		Adds a triangle face to this model. The face is represented by 3 point indices
		in the points vector. If the face is invalid (indices out of range) returns false,
		true otherwise.
	*/
	bool addFace(vec3 face);
	void setRotation(float _rotationAngle, vec3 _rotationAxis);
	void setRotationEnabled(bool _rotationEnabled);

	void draw();

private:
	vector<vec3> points;
	vector<vec3> faces;
	bool rotationEnabled;
	float rotationAngle;
	vec3 rotationAxis;
};