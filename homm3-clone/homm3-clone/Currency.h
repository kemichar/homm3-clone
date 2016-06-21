#pragma once

#include <glm/glm.hpp>
#include "Constants.h"

class Currency {

public:

	Currency();

	int& operator[](int res) {
		return storage[res];
	}

	glm::vec3 resourceColor(Resource resource);

	int storage[_RESOURCE_END];
};