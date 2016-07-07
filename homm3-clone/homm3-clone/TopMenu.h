#pragma once

#include "UIClickableContainer.h"
#include "Utility.h"

class TopMenu : public UIClickableContainer {
public:
	TopMenu();

	virtual void draw(intp pos, intp dim) override;
	virtual void areaClicked(int index) override;
	virtual void areaHovered(int index) override;

private:
	intp lastDrawPos;
	intp lastDrawDim;

};