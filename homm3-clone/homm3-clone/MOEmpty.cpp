#include "MOEmpty.h"

MOEmpty::MOEmpty(intp _pos)
	: MapObject(_pos) {
	objectType = EMPTY;
}

void MOEmpty::interact() {
	// intentionally empty
}

bool MOEmpty::isBlocking() {
	return false;
}

bool MOEmpty::isHolding() {
	return false;
}