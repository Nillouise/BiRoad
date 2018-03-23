#pragma once
#include "ECS.h"
#include <set>
#include<memory>
class Object
{
public:
	std::set<std::shared_ptr<ECS>> attributes;
};

class Direction
{
public:
	enum direction_enum
	{
		up,down,left,right
	};
	Direction(direction_enum direction)
	{
		this->direction = direction;
	}
	direction_enum direction;
};