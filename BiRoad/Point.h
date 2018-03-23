#pragma once

class Direction
{
public:
	enum direction_enum
	{
		up, down, left, right
	};
	Direction(direction_enum direction)
	{
		this->direction = direction;
	}
	direction_enum direction;
};

class Point
{
public:
	int x, y;
	Point(int x = 0,int y = 0):
		x(0),y(0){}
	bool operator==(const Point&p)
	{
		return x == p.x&&y == p.y;
	}
	bool operator<(const Point&p)
	{
		return x == p.x ? y < p.y : x < p.x;
	}
};