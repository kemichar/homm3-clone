#include "Currency.h"

Currency::Currency() {
	memset(storage, 0, sizeof storage);
}

glm::vec3 Currency::resourceColor(Resource resource) {
	if (resource == GOLD) {
		return COLORS[YELLOW];
	}
	else if (resource == WOOD) {
		return COLORS[BROWN];
	}
	else if (resource == ORE) {
		return COLORS[GRAY];
	}
	else if (resource == CRYSTAL) {
		return COLORS[PURPLE];
	}

	return COLORS[RED];
}
