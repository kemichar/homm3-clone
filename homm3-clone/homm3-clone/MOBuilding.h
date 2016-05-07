#pragma once

#include "MapObject.h"
#include "ViewManager.h"

class MOBuilding : public MapObject {

public:
	MOBuilding(intp _pos, floatp _modelOffset = floatp(0, 0)) : MapObject(_pos, _modelOffset) {
		objectCode = 3;
		objectName = "Building";
	}

	bool isBlocking() {
		return false;
	}

	virtual bool isHolding() {
		return true;
	}

	virtual void interact() {
		// TEMP proof of concept, inherit and change
		ViewManager::showPrompt("Testing purple donut activation...");
	}

	virtual void draw(float size) {
		if (model == nullptr) {
			glPushMatrix();
			if (modelOffset != floatp(0, 0)) {
				glTranslatef(modelOffset.x * size, modelOffset.y * size, 0);
				glColor3f(0.7, 0.7, 0.7);
				glutSolidTorus(size / 2, size / 2, 10, 10);
			}
			else {
				glColor3f(0.8, 0.4, 0.8);
				glutSolidTorus(size / 4.8, size / 5, 10, 10);
			}
			glPopMatrix();
		}
		else {
			MapObject::draw(size);
		}
	}

private:

};