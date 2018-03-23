#include "System.h"
#include "Snakable.h"
#include <map>

void eatable_system(World& world)
{

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
		for(const auto &attr:obj.attributes)
		{
			ECS *e = attr.get();
			if(Snakable *p = dynamic_cast<Snakable*>(e))
			{
				Point head = p->body.front();
				head.x += offset[p->direction.direction].first;
				head.y += offset[p->direction.direction].second;
				p->body.push_front(head);
			}
		}
	}
}
