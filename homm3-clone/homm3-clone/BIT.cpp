#include "BIT.h"

BIT::BIT(int _width, int _height) {
	_width++;
	_height++;

	tree.resize(_width);
	for (int i = 0; i < (int)tree.size(); i++) {
		tree[i].resize(_height, intp(0, 0));
	}
}

void BIT::update(intp point, int value) {
	point.x++;
	point.y++;

	int add = value - tree[point.x][point.y].y;
	tree[point.x][point.y].y = value;
	for (int i = point.x; i <= (int)tree.size(); i += i & -i)
		for (int j = point.y; j <= (int)tree[i].size(); j += j & -j)
			tree[i][j].x += add;
}

int BIT::query(int x, int y) {
	x++;
	y++;

	int res = 0;
	for (int i = x; i; i -= i & -i)
		for (int j = y; j; j -= j & -j)
			res += tree[i][j].x;
	return res;
}

int BIT::query(intp point) {
	return query(point.x, point.y);
}

int BIT::query(intp bottomLeft, intp topRight) {
	return query(topRight.x, topRight.y) - query(topRight.x, bottomLeft.y - 1) -
		query(bottomLeft.x - 1, topRight.y) + query(bottomLeft.x - 1, bottomLeft.y - 1);
}
