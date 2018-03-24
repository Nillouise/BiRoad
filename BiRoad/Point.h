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
	operator direction_enum ()const
	{
		return direction;
	}
};

class Point
{
public:
	int c, r;
	Point(int c = 0,int r = 0):
		c(c),r(r){}
	bool operator==(const Point&p) const
	{
		return c == p.c&&r == p.r;
	}
	bool operator<(const Point&p) const
	{
		return c == p.c ? r < p.r : c < p.c;
	}
};