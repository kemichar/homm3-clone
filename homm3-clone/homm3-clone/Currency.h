#pragma once

#include <glm/glm.hpp>
#include "Constants.h"

class Currency {

public:

	Currency() {
		memset(storage, 0, sizeof storage);
	}

	int& operator[](int res) {
		return storage[res];
	}

	vec3 resourceColor(Resource resource) {
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

	int storage[_RESOURCE_END];
};