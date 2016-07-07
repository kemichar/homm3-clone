#pragma once

#include <vector>
#include "Utility.h"

class UIClickableContainer {
public:
	UIClickableContainer();

	void processClick(float x, float y);
	void processHover(float x, float y);
	virtual void draw(intp pos, intp dim) = 0;
	virtual void areaClicked(int index);
	virtual void areaHovered(int index);

	std::vector<Rect> areas;

private:

};