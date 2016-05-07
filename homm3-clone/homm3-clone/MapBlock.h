#pragma once

#include <string>
#include <vector>

using namespace std;

class MapBlock {

public:
	MapBlock(string _properties, vector<string> _data);
	MapBlock(vector<string> _everything);
	bool match(string requirements);

	vector<string> data;
	string properties;
	int width;
	int height;
};