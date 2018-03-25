#pragma once
#include "ECS.h"
#include <map>
#include<memory>
#include <unordered_map>
#include "Eatable.h"
#include "Position.h"
#include "Snakable.h"
#include "Obstacle.h"

using std::shared_ptr;
using std::make_shared;
using std::string;
namespace std
{
}


class Object
{
public:
	std::map<string,std::shared_ptr<ECS>> attributes;
	int id;
	static shared_ptr<Object> factory()
	{
		return make_shared<Object>();
	}

	static shared_ptr<Object> factoryBall(Position p=Point{0,0})
	{
		shared_ptr<Object> res = make_shared<Object>();
		res->attributes[typeid(Eatable).name()] = make_shared<Eatable>();
		res->attributes[typeid(Position).name()] = make_shared<Position>(p);

		return res;
	}

	static shared_ptr<Object> factorySnake(int id)
	{
		shared_ptr<Object> res = make_shared<Object>();
		res->attributes[typeid(Snakable).name()] = make_shared<Snakable>(id);
		res->attributes[typeid(Obstacle).name()] = make_shared<Obstacle>();
		return res;
	}

};

