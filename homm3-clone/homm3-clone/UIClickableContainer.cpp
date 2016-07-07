#include "UIClickableContainer.h"

UIClickableContainer::UIClickableContainer() {
	areas.clear();
}

void UIClickableContainer::processClick(float x, float y) {
	for (int i = 0; i < (int)areas.size(); i++) {
		if (areas[i].contains(floatp(x, y))) {
			areaClicked(i);
			return;
		}
	}
}

void UIClickableContainer::processHover(float x, float y) {
	for (int i = 0; i < (int)areas.size(); i++) {
		if (areas[i].contains(floatp(x, y))) {
			areaHovered(i);
			return;
		}
	}
}

void UIClickableContainer::areaClicked(int index) {
}

void UIClickableContainer::areaHovered(int index) {
}
