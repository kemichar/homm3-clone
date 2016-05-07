#pragma once

#include "MapObject.h"

class MOEmpty : public MapObject {

public:
	MOEmpty(intp _pos) : MapObject(_pos){
		objectCode = 0;
		objectName = "EmptyTile";
	}
};