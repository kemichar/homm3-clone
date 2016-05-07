#pragma once
#ifndef _H_UTILITY_
#define _H_UTILITY_

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <GL/glut.h>
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <streambuf>
#include <sstream>
#include <vector>
#include <iostream>
#include <map>

using namespace glm;
using namespace std;

// A pair of ints.
struct intp {
	intp() : x(0), y(0) {}
	intp(int _x, int _y) : x(_x), y(_y) {}
	intp(vec2 _data) : x((int)_data.x), y((int)_data.y) {}

	intp operator+(const intp &a) const {
		return intp(a.x + x, a.y + y);
	}

	bool operator==(const intp &a) const {
		return (x == a.x && y == a.y);
	}

	bool operator!=(const intp &a) const {
		return !(x == a.x && y == a.y);
	}
	
	int x;
	int y;
};

// A pair of floats.
struct floatp {
	floatp() : x(0), y(0) {}
	floatp(float _x, float _y) : x(_x), y(_y) {}
	floatp(vec2 _data) : x((float)_data.x), y((float)_data.y) {}

	floatp operator+(const floatp &a) const {
		return floatp(a.x + x, a.y + y);
	}

	bool operator==(const floatp &a) const {
		return (x == a.x && y == a.y);
	}

	bool operator!=(const floatp &a) const {
		return !(x == a.x && y == a.y);
	}

	float x;
	float y;
};

inline vec2 randomUnit(){
	vec2 ret = vec2(rand() - RAND_MAX / 2, rand() - RAND_MAX / 2);
	return normalize(ret);
}

inline float angleBetween(vec3 x, vec3 y){
	return dot(normalize(x), normalize(y)) / 57.2957795131f;
}

inline vec3 axisBetween(vec3 x, vec3 y){
	return cross(normalize(x), normalize(y));
}

inline float manhattan(vec2 x, vec2 y) {
	return abs(x.x - y.x) + abs(x.y - y.y);
}

inline float manhattan(intp x, intp y) {
	return abs(x.x - y.x) + abs(x.y - y.y);
}

inline float euclid(vec2 x, vec2 y) {
	return sqrt((x.x - y.x) * (x.x - y.x) + (x.y - y.y) * (x.y - y.y));
}

inline float euclid(intp x, intp y) {
	return sqrt((x.x - y.x) * (x.x - y.x) + (x.y - y.y) * (x.y - y.y));
}

inline float mmin(float x, float y) {
	return x < y ? x : y;
}

inline float mmax(float x, float y) {
	return x > y ? x : y;
}

inline double mmax(double x, double y) {
	return x > y ? x : y;
}

inline double mmin(double x, double y) {
	return x < y ? x : y;
}

inline int mmax(int x, int y) {
	return x > y ? x : y;
}

inline int mmin(int x, int y) {
	return x < y ? x : y;
}

inline vec3 clamp(vec3 l, vec3 r, vec3 x){
	x.x = mmin(r.x, mmax(l.x, x.x));
	x.y = mmin(r.y, mmax(l.y, x.y));
	x.z = mmin(r.z, mmax(l.z, x.z));
	return x;
}

inline bool rectContains(vec2 rectPoint, vec2 rectDim, vec2 point) {
	return point.x >= rectPoint.x && point.x < rectPoint.x + rectDim.x
		&& point.y >= rectPoint.y && point.y <= rectPoint.y + rectDim.y;
}

inline void drawText(void* font, string text) {
	for (int i = 0; i < (int)text.size(); i++) {
		glutBitmapCharacter(font, text[i]);
	}
}

inline double clamp(double l, double r, double x) {
	return mmin(r, mmax(l, x));
}

inline vec2 lerp(vec2 fromPoint, vec2 toPoint, float progress){
	return fromPoint + (toPoint - fromPoint) * progress;
}

inline vec2 coordsFromMap(vec2 mapCoords, double tileDimension) {
	return vec2(mapCoords.x * tileDimension + tileDimension / 2, mapCoords.y * tileDimension + tileDimension / 2);
}

/*
Intersection with the z = 0 plane.
*/
inline vec3 lineXYPlaneIntersection(vec3 a, vec3 b) {
	vec3 normal(0, 0, 1);
	vec3 ba = b - a;
	float nDotA = dot(normal, a);
	float nDotBA = dot(normal, ba);

	return a + (-nDotA / nDotBA) * ba;
}

inline std::map<string, string> readListedFiles(string directory, string listFileName) {
	if (directory[directory.size() - 1] != '/') {
		directory += '/';
	}

	listFileName = directory + listFileName;

	std::map<string, string> data;

	FILE *fileList = fopen(listFileName.c_str(), "r");
	if (fileList != NULL) {
		char fileName[50] = "";
		while (fscanf(fileList, "%s", fileName) != EOF) {
			string nameString(fileName);
			FILE *file = fopen((directory + nameString).c_str(), "r");
			if (file != NULL) {
				ifstream t(file);
				stringstream buffer;
				buffer << t.rdbuf();
				data[nameString.substr(0, nameString.find_last_of("."))] = buffer.str(); // be careful when there's no '.'
				fclose(file);
			}
		}
		fclose(fileList);
	}

	return data;
}

inline vector<string> readFile(string fileName) {
	vector<string> data;

	FILE *file = fopen(fileName.c_str(), "r");
	if (file != NULL) {
		ifstream t(file);
		stringstream buffer;
		buffer << t.rdbuf();
		string line;
		while (getline(buffer, line)) {
			data.push_back(line);
		}
		fclose(file);
	}
	else {
		// DEBUG
		cout << "Utility.h error: Couldn't open file " << fileName << endl;
	}

	return data;
}

inline vector<string> splitString(string line) {
	vector<string> ret;

	stringstream buffer(line);
	string word;
	while (buffer.good()) {
		buffer >> word;
		ret.push_back(word);
	}

	return ret;
}

#endif