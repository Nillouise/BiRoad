#pragma once
#include "ECS.h"
#include "Point.h"
#include <deque>

class Snakable:public ECS
{
public:
	std::deque<Point> body;
	Direction direction = Direction(Direction::up);
	bool isCollided;
};
