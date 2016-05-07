#include "Model.h"

bool Model::loadFromFile(string inputFile) {
	FILE *input = fopen(inputFile.c_str(), "r");
	if (input == NULL) {
		printf("MISSING model file %s!\n", inputFile.c_str());
		return false;
	}
	char ch;
	while (fscanf(input, "%c", &ch) != EOF) {
		if (ch == 'v') {
			double x, y, z;
			fscanf(input, "%lf %lf %lf", &x, &y, &z);
			addPoint(vec3(x, y, z));
		}
		else if (ch == 'f') {
			int x, y, z;
			fscanf(input, "%d %d %d", &x, &y, &z);
			addFace(vec3(--x, --y, --z));
		}
	}
	fclose(input);
	printf("... loaded %s\n", inputFile.c_str());
	return true;
}

void Model::addPoint(vec3 point) {
	points.push_back(point);
}

bool Model::addFace(vec3 face) {
	if (face.x >= points.size() || face.y >= points.size() || face.z >= points.size()) {
		return false;
	}

	faces.push_back(face);
	return true;
}

void Model::setRotation(float _rotationAngle, vec3 _rotationAxis) {
	rotationEnabled = true;
	rotationAngle = _rotationAngle;
	rotationAxis = _rotationAxis;
}

void Model::setRotationEnabled(bool _rotationEnabled) {
	rotationEnabled = _rotationEnabled;
}

void Model::draw() {
	if (rotationEnabled) {
		glPushMatrix();
		glRotatef(rotationAngle, rotationAxis.x, rotationAxis.y, rotationAxis.z);
	}

	glBegin(GL_LINES);
	for (int i = 0; i < (int)faces.size(); i++) {
		glVertex3f(points[faces[i].x].x, points[faces[i].x].y, points[faces[i].x].z);
		glVertex3f(points[faces[i].y].x, points[faces[i].y].y, points[faces[i].y].z);

		glVertex3f(points[faces[i].y].x, points[faces[i].y].y, points[faces[i].y].z);
		glVertex3f(points[faces[i].z].x, points[faces[i].z].y, points[faces[i].z].z);

		glVertex3f(points[faces[i].z].x, points[faces[i].z].y, points[faces[i].z].z);
		glVertex3f(points[faces[i].x].x, points[faces[i].x].y, points[faces[i].x].z);
	}
	glEnd();

	if (rotationEnabled) {
		glPopMatrix();
	}
}
