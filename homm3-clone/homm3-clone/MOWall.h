#pragma once

#include "MapObject.h"

class MOWall : public MapObject {

public:
	MOWall(intp _pos, bool _invisible = false) : MapObject(_pos), invisible(_invisible) {
		objectCode = 1;
		objectName = "Wall" + string((_invisible ? " (invisible)" : ""));
	}

	bool isBlocking() {
		return true;
	}

	virtual void draw(float size) {
		if (invisible) {
			return;
		}

		if (model == nullptr) {
			glPushMatrix();
			glColor3f(0.2, 0.2, 0.2);
			glutSolidCube(size * 0.7);
			glPopMatrix();
		}
		else {
			MapObject::draw(size);
		}
	}

private:
	bool invisible;

};