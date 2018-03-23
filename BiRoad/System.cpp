#include "System.h"
#include "Snakable.h"
#include <map>
#include "Eatable.h"
#include <vector>

namespace {
	bool collided(std::vector<Object*> &balls,const Snakable *snake)
	{
		
	}

	template<typename T>
	T* getAttr(const Object &obj)
	{
		auto a = obj.attributes.find(typeid(T));

		if(a==end())
		{
			return nullptr;
		}else
		{
			return a->second;
		}
	}
}

void eatable_system(World& world)
{
	std::vector<Object*> balls;
	for (auto obj : world.objs)
	{
		if(Eatable *e = getAttr<Eatable>(obj))
		{
			balls.push_back(&obj);
			break;
		}
	}


}

void obstacle_system(World& world)
{

}

void snakable_system(World& world)
{
	static std::map<Direction::direction_enum, std::pair<int,int>> offset = 
	{
		{Direction::up,{0,-1}},
		{Direction::down,{0,1}},
		{Direction::left,{-1,0}},
		{Direction::right,{1,0}},
	};
	for(auto obj:world.objs)
	{
		if(Snakable* p = getAttr<Snakable>(obj))
		{
			Point head = p->body.front();
			head.x += offset[p->direction.direction].first;
			head.y += offset[p->direction.direction].second;
			p->body.push_front(head);
		}
	}
}
