#include "UICreatureContainerDouble.h"

UICreatureContainerDouble::UICreatureContainerDouble(Hero * _firstHero, Hero * _secondHero)
	: selectedSlot(std::make_pair(nullptr, -1)), firstHero(_firstHero), secondHero(_secondHero){

	const floatp slotDim(1.f / HERO_UNIT_SLOTS, 0.5f);
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		areas.push_back(Rect(floatp(i * slotDim.x, slotDim.y), floatp(slotDim.x, slotDim.y)));
	}
	for (int i = 0; i < HERO_UNIT_SLOTS; i++) {
		areas.push_back(Rect(floatp(i * slotDim.x, 0), floatp(slotDim.x, slotDim.y)));
	}
}

void UICreatureContainerDouble::draw(intp pos, intp dim) {
	for (int i = 0; i < (int)areas.size(); i++) {
		glColor3f(0, 0, 0);
		glRectf(pos.x + areas[i].pos.x * dim.x, pos.y + areas[i].pos.y * dim.y,
			pos.x + (areas[i].pos.x + areas[i].dim.x) * dim.x, pos.y + (areas[i].pos.y + areas[i].dim.y) * dim.y);
		glColor3f(0.7f, 0.7f, 0.7f);
		glRectf(pos.x + areas[i].pos.x * dim.x + 1, pos.y + areas[i].pos.y * dim.y + 1,
			pos.x + (areas[i].pos.x + areas[i].dim.x) * dim.x - 2, pos.y + (areas[i].pos.y + areas[i].dim.y) * dim.y - 2);

		if (i < HERO_UNIT_SLOTS) {
			Creature* creature = firstHero->creatures[i];
			if (creature != nullptr) {
				glColor3f(0, 0, 0);
				glRasterPos2d(pos.x + areas[i].pos.x * dim.x + 5, pos.y + (areas[i].pos.y + areas[i].dim.y / 3 * 2)* dim.y);
				drawText(GLUT_BITMAP_HELVETICA_12, std::to_string(creature->count));
				glRasterPos2d(pos.x + areas[i].pos.x * dim.x + 5, pos.y + (areas[i].pos.y + areas[i].dim.y / 3)* dim.y);
				drawText(GLUT_BITMAP_HELVETICA_12, creature->count > 1 ? creature->plural : creature->name);
			}
		}
		else if (secondHero != nullptr){
			Creature* creature = secondHero->creatures[i - HERO_UNIT_SLOTS];
			if (creature != nullptr) {
				glColor3f(0, 0, 0);
				glRasterPos2d(pos.x + areas[i].pos.x * dim.x + 5, pos.y + (areas[i].pos.y + areas[i].dim.y / 3 * 2)* dim.y);
				drawText(GLUT_BITMAP_HELVETICA_12, std::to_string(creature->count));
				glRasterPos2d(pos.x + areas[i].pos.x * dim.x + 5, pos.y + (areas[i].pos.y + areas[i].dim.y / 3)* dim.y);
				drawText(GLUT_BITMAP_HELVETICA_12, creature->count > 1 ? creature->plural : creature->name);
			}
		}
	}
}

void UICreatureContainerDouble::areaClicked(int index) {
	std::pair<Hero*, int> clickedSlot(nullptr, -1);
	if (index < HERO_UNIT_SLOTS) {
		clickedSlot = std::make_pair(firstHero, index);
	}
	else if (index < HERO_UNIT_SLOTS * 2) {
		clickedSlot = std::make_pair(secondHero, index - HERO_UNIT_SLOTS);
	}

	if (clickedSlot.first == nullptr) {
		return;
	}
	if (clickedSlot == selectedSlot) {
		selectedSlot = std::make_pair(nullptr, -1);
		return;
	}

	if (selectedSlot.first == nullptr) {
		if (clickedSlot.first->creatures[clickedSlot.second] != nullptr) {
			selectedSlot = clickedSlot;
		}
	}
	else {
		Creature* firstStack = selectedSlot.first->creatures[selectedSlot.second];
		Creature* secondStack = clickedSlot.first->creatures[clickedSlot.second];
		if (secondStack != nullptr && firstStack->name == secondStack->name) {
			secondStack->count += firstStack->count;
			selectedSlot.first->creatures[selectedSlot.second] = nullptr;
			delete firstStack;
			selectedSlot = std::make_pair(nullptr, -1);
		}
		else {
			selectedSlot.first->creatures[selectedSlot.second] = secondStack;
			clickedSlot.first->creatures[clickedSlot.second] = firstStack;
			selectedSlot = std::make_pair(nullptr, -1);
		}
	}
}
