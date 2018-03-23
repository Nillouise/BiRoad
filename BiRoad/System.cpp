#include "System.h"
#include "Snakable.h"
#include <map>
#include "Eatable.h"
#include <vector>
#include "Position.h"

namespace {
	template<typename T>
	T* getAttr(const Object &obj);

	bool collided(std::vector<Object*> &balls,const Snakable *snake)
	{
		for(auto ball:balls)
		{
			if(Position *e = getAttr<Position>(*ball))
			{
				for(const auto &s: snake->body)
				{
					if(s.x==e->point.x&&s.y==e->point.y)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	template<typename T>
	T* getAttr(const Object &obj)
	{
		std::map<type_info, std::shared_ptr<ECS>>::const_iterator a = obj.attributes.find(typeid(T));
		if(a==end())
		{
			return nullptr;
		}else
		{
			return a->second.get();
		}
	}
}

void eatable_system(World& world)
{
	std::vector<Object*> balls;

	for (auto obj : world.objs)
	{
		if(Eatable *e = getAttr<Eatable>(*obj))
		{
			balls.push_back(obj.get());
		}
	}
	for(auto &a:world.objs)
	{
		if(Snakable *s = getAttr<Snakable>(*a))
		{
			//如果蛇没有碰到球，尾端应该要被删除的
			if(!collided(balls,s))
			{
				s->body.pop_back();
			}
		}
	}
}

void obstacle_system(World& world)
{
	std::vector<Object*> snakes;

	for (auto &obj : world.objs)
	{
		if (Snakable *e = getAttr<Snakable>(*obj))
		{
			snakes.push_back(obj.get());
		}
	}
	std::map<Point, int> maze;
	for(auto a:snakes)
	{
		Snakable *e1 = getAttr<Snakable>(*a);
		for(auto &b:e1->body)
		{
			int vis = ++maze[b];
			if(vis>1)
			{
				e1->isCollided = true;
				++world.current_frame_has_collide;
			}
		}
	}
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
		if(Snakable* p = getAttr<Snakable>(*obj.get()))
		{
			Point head = p->body.front();
			head.x += offset[p->direction.direction].first;
			head.y += offset[p->direction.direction].second;
			p->body.push_front(head);
		}
	}
}

