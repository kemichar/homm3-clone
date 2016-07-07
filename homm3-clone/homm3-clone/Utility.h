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

// A pair of ints.
struct intp {
	intp() : x(0), y(0) {}
	intp(int _x, int _y) : x(_x), y(_y) {}
	intp(glm::vec2 _data) : x((int)_data.x), y((int)_data.y) {}

	intp operator+(const intp &a) const {
		return intp(a.x + x, a.y + y);
	}

	intp operator-(const intp &a) const {
		return intp(x - a.x, y - a.y);
	}

	bool operator==(const intp &a) const {
		return (x == a.x && y == a.y);
	}

	bool operator!=(const intp &a) const {
		return !(x == a.x && y == a.y);
	}

	bool operator<(const intp &a) const {
		return (x < a.x) || (x == a.x && y < a.y);
	}
	
	int x;
	int y;
};

// A pair of floats.
struct floatp {
	floatp() : x(0), y(0) {}
	floatp(float _x, float _y) : x(_x), y(_y) {}
	floatp(glm::vec2 _data) : x((float)_data.x), y((float)_data.y) {}

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

struct Rect {
	Rect() : pos(floatp(0, 0)), dim(floatp(0, 0)) {}
	Rect(floatp _pos, floatp _dim) : pos(_pos), dim(_dim) {}

	bool contains(floatp point) {
		return point.x >= pos.x && point.x < pos.x + dim.x
			&& point.y >= pos.y && point.y <= pos.y + dim.y;
	}

	bool operator==(const Rect &a) const {
		return (pos == a.pos && dim == a.dim);
	}

	bool operator!=(const Rect &a) const {
		return !(pos == a.pos && dim == a.dim);
	}

	floatp pos;
	floatp dim;
};

inline glm::vec2 randomUnit(){
	glm::vec2 ret = glm::vec2(rand() - RAND_MAX / 2, rand() - RAND_MAX / 2);
	return normalize(ret);
}

inline float angleBetween(glm::vec3 x, glm::vec3 y){
	return dot(normalize(x), normalize(y)) / 57.2957795131f;
}

inline glm::vec3 axisBetween(glm::vec3 x, glm::vec3 y){
	return cross(normalize(x), normalize(y));
}

inline float manhattan(glm::vec2 x, glm::vec2 y) {
	return abs(x.x - y.x) + abs(x.y - y.y);
}

inline float manhattan(intp x, intp y) {
	return abs(x.x - y.x) + abs(x.y - y.y);
}

inline float euclid(glm::vec2 x, glm::vec2 y) {
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

inline glm::vec3 clamp(glm::vec3 l, glm::vec3 r, glm::vec3 x){
	x.x = mmin(r.x, mmax(l.x, x.x));
	x.y = mmin(r.y, mmax(l.y, x.y));
	x.z = mmin(r.z, mmax(l.z, x.z));
	return x;
}

inline bool rectContains(glm::vec2 rectPoint, glm::vec2 rectDim, glm::vec2 point) {
	return point.x >= rectPoint.x && point.x < rectPoint.x + rectDim.x
		&& point.y >= rectPoint.y && point.y <= rectPoint.y + rectDim.y;
}

inline void drawText(void* font, std::string text) {
	for (int i = 0; i < (int)text.size(); i++) {
		glutBitmapCharacter(font, text[i]);
	}
}

inline double clamp(double l, double r, double x) {
	return mmin(r, mmax(l, x));
}

inline glm::vec2 lerp(glm::vec2 fromPoint, glm::vec2 toPoint, float progress){
	return fromPoint + (toPoint - fromPoint) * progress;
}

inline glm::vec2 coordsFromMap(intp mapCoords, double tileDimension) {
	return glm::vec2(mapCoords.x * tileDimension + tileDimension / 2, mapCoords.y * tileDimension + tileDimension / 2);
}

/*
Intersection with the z = 0 plane.
*/
inline glm::vec3 lineXYPlaneIntersection(glm::vec3 a, glm::vec3 b) {
	glm::vec3 normal(0, 0, 1);
	glm::vec3 ba = b - a;
	float nDotA = dot(normal, a);
	float nDotBA = dot(normal, ba);

	return a + (-nDotA / nDotBA) * ba;
}

inline std::map<std::string, std::string> readListedFiles(std::string directory, std::string listFileName) {
	if (directory[directory.size() - 1] != '/') {
		directory += '/';
	}

	listFileName = directory + listFileName;

	std::map<std::string, std::string> data;

	FILE *fileList = fopen(listFileName.c_str(), "r");
	if (fileList != NULL) {
		char fileName[50] = "";
		while (fscanf(fileList, "%s", fileName) != EOF) {
			std::string nameString(fileName);
			FILE *file = fopen((directory + nameString).c_str(), "r");
			if (file != NULL) {
				std::ifstream t(file);
				std::stringstream buffer;
				buffer << t.rdbuf();
				data[nameString.substr(0, nameString.find_last_of("."))] = buffer.str(); // be careful when there's no '.'
				fclose(file);
			}
		}
		fclose(fileList);
	}

	return data;
}

inline std::vector<std::string> readFile(std::string fileName) {
	std::vector<std::string> data;

	FILE *file = fopen(fileName.c_str(), "r");
	if (file != NULL) {
		std::ifstream t(file);
		std::stringstream buffer;
		buffer << t.rdbuf();
		std::string line;
		while (getline(buffer, line)) {
			data.push_back(line);
		}
		fclose(file);
	}
	else {
		// DEBUG
		std::cout << "Utility.h error: Couldn't open file " << fileName << std::endl;
	}

	return data;
}

inline std::vector<std::string> splitString(std::string line) {
	std::vector<std::string> ret;

	std::stringstream buffer(line);
	std::string word;
	while (buffer.good()) {
		buffer >> word;
		ret.push_back(word);
	}

	return ret;
}

inline std::vector<std::string> splitLines(std::string text) {
	std::vector<std::string> ret;

	std::stringstream buffer(text);
	std::string line;
	while (std::getline(buffer, line, '\n')) {
		ret.push_back(line);
	}

	return ret;
}

#endif