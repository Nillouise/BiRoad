#pragma once
#include "ECS.h"
#include "Point.h"


class Position:public ECS
{
public:
	Point point;
	Position(Point p):point(p){}
};
