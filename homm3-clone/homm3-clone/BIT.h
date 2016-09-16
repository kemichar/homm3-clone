#pragma once

#include <vector>
#include "Utility.h"

/*
	A 2D Binary Indexed Tree (Fenwick Tree) structure.
	Used for efficient value storage and sum-querying
	of value intevals. Time complexity of (single cell)
	update and (any) query is O(logN * logM), where
	N and M are the dimensions of the matrix.
	For more details check out the TopCoder tutorial on BITs.
*/
class BIT {

public:
	BIT(int _width, int _height);

	/*
		Sets the value at cell (<paramref name="point.x"/>,
		<paramref name="point.y"/>) to <paramref name="value"/>.
	*/
	void update(intp point, int value);

	/*
		Returns the value at cell (<paramref name="x"/>,
		<paramref name="y"/>).
	*/
	int query(int x, int y);

	/*
		Returns the value at cell (<paramref name="point.x"/>,
		<paramref name="point.y"/>).
	*/
	int query(intp point);

	/*
		Returns the sum of values in the rectangle between
		<paramref name="bottomLeft"/> and
		<paramref name="topRight"/>, inclusive.
	*/
	int query(intp bottomLeft, intp topRight);

private:
	/*
		The tree's data storage. The value at some
		location tree[x][y] does not correspond to the
		value at location (x, y) in the matrix of values
		this structure covers.
	*/
	std::vector<std::vector<intp>> tree;

};