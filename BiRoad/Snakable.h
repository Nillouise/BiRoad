#pragma once
#include "ECS.h"
#include "Point.h"
#include <deque>

class Snakable:public ECS
{
public:
	std::deque<Point> body;
	int id;//snakeType 的id最好不是跟objec的id一样吧
	Direction direction = Direction(Direction::down);
	Direction next_direction = Direction(Direction::down);
	bool isCollided = false;
	Snakable(int id=0):id(id){}
};
