#pragma once

#include <vector>
#include "Utility.h"

class BIT {

public:
	BIT(int _width, int _height);

	void update(intp point, int value);

	int query(int x, int y);

	int query(intp point);

	int query(intp bottomLeft, intp topRight);

private:
	std::vector<std::vector<intp>> tree;

};