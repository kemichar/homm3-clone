#pragma once

#include "UIClickableContainer.h"
#include "Hero.h"
#include "Utility.h"

class UICreatureContainerDouble : public UIClickableContainer {
public:
	UICreatureContainerDouble(Hero* _firstHero, Hero* _secondHero);

	virtual void draw(intp pos, intp dim) override;
	virtual void areaClicked(int index) override;

	std::pair<Hero*, int> selectedSlot;
	Hero* firstHero;
	Hero* secondHero;

private:

};