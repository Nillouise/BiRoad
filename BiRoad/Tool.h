#pragma once
#include <string>
#include <vector>
#include "Point.h"
#include <random>
#include"World.h"

class Tool
{
public:
	static std::vector<std::string> split(const std::string& s, const std::string &delimiter);
	static Point getRandomBall(int height, int width);
	static bool newBallConflictWithWorld(const World &world,const Point &p);
	template<typename T>
	static  T* getAttr(const Object &obj);
};



