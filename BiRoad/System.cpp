#include "System.h"
#include "Snakable.h"
#include <map>
#include "Eatable.h"
#include <vector>
#include "Position.h"
#include <mutex>
#include <iostream>
#include <string>
#include "Constant.h"
#include "Game.h"
#include "TextureManager.h"
#include "Tool.h"

using std::to_string;
using std::map;
using std::cout;
using std::endl;

namespace {
	std::string receive_message_buffer;
	std::mutex receiver_message_mutex;
	std::string send_message_buffer;
	std::mutex send_message_mutex;


	std::map<std::string, Direction::direction_enum> convertor{ { Constant::up,Direction::up },
		{ Constant::right,Direction::right },{ Constant::down,Direction::down },{ Constant::left,Direction::left } 
	};


	string serial_map(const map<string,string> &keyval)
	{
		string res;
		int i = 0;
		for(auto a:keyval)
		{
			if(i!=0)
			{
				res += Constant::item_delimiter;
			}
			i++;
			res += a.first + Constant::equal_delimiter + a.second;
		}
		return res;
	}


	//这里是反序列化一个user内的所有item
	map<string,string> deserial_item_map(const string &s)
	{
		map<string, string> res;
		auto items = Tool::split(s, Constant::item_delimiter);
		for(auto &item:items)
		{
			auto keyval = Tool::split(item, Constant::equal_delimiter);
			if(keyval.size()==2)
			{
				res[keyval[0]] = keyval[1];
			}
		}
		return res;
	}


	bool isConverseDirect(const Direction::direction_enum &e1,const Direction::direction_enum &e2)
	{
		return (e1 == Direction::down && e2 == Direction::up)
			|| (e1 == Direction::up && e2 == Direction::down)
			|| (e1 == Direction::left && e2 == Direction::right)
			|| (e1 == Direction::right && e2 == Direction::left);
	}


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
					if(s.c==e->point.c&&s.r==e->point.r)
					{
						return true;
					}
				}
			}
		}
		return false;
	}


	bool outOfWorld(const World &world,const Snakable *snake)
	{
		for(auto &a: snake->body)
		{
			if(a.c<1||a.c>world.width||a.r<1||a.r>world.height)
			{
				return true;
			}
		}
		return false;
	}


	template<typename T>
	T* getAttr(const Object &obj)
	{
		std::map<type_info, std::shared_ptr<ECS>>::const_iterator a = obj.attributes.find(typeid(T));
		if(a== obj.attributes.end())
		{
			return nullptr;
		}else
		{
			return a->second.get();
		}
	}


	std::shared_ptr<Object> get_self_snake(World &world)
	{
		for(auto a:world.objs)
		{
			if(Snakable *s = getAttr<Snakable>(*a))
			{
				if(s->id==world.self_id)
				{
					return a;
				}
			}
		}
		return nullptr;
	}
}


void eatable_system(World& world)
{
	std::vector<Object*> balls;

	for (auto &obj : world.objs)
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
		if (outOfWorld(world, e1))
		{
			if (!e1->isCollided)
			{
				++world.current_frame_has_collide;
			}
			e1->isCollided = true;
		}
		for(auto &b:e1->body)
		{
			int vis = ++maze[b];
			if(vis>1)
			{
				if(!e1->isCollided)
				{
					++world.current_frame_has_collide;
				}
				e1->isCollided = true;
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
	for(auto &obj:world.objs)
	{
		if(Snakable* p = getAttr<Snakable>(*obj))
		{
			Point head = p->body.front();
			head.c += offset[p->direction.direction].first;
			head.r += offset[p->direction.direction].second;
			p->body.push_front(head);
		}
	}
}


void death_system(World &world)
{
	for(auto a = world.objs.begin();a != world.objs.end();)
	{
		if (Snakable *e = getAttr<Snakable>(**a))
		{
			if (e->isCollided)
			{
				a = world.objs.erase(a);
			}else
			{
				++a;
			}
		}else
		{
			++a;
		}
	}
}


void input(World &world,const std::string &keyname)
{
	send_message_mutex.lock();
	try
	{
		auto dire = convertor[keyname];
		if(auto obj = get_self_snake(world))
		{
			if(Snakable *snake = getAttr<Snakable>(*obj))
			{
				if(!isConverseDirect(snake->direction.direction,dire))
				{
					map<string, string> keyval;
					keyval[Constant::current_frame_numb] = to_string(world.current_frame_numb);
					keyval[Constant::press_key] = keyname;
					keyval[Constant::self_id] = to_string(world.self_id);
					send_message_buffer = serial_map(keyval);
				}
			}
		}
	}catch(...)
	{
		std::cerr << "inpute error" << std::endl;
	}
	send_message_mutex.unlock();
}


void render_system(World &world,Game *game)
{
	SDL_RenderClear(game->m_pRenderer.get()); // clear the renderer to the draw color

	if(game->m_printHint)
	{
		TheTextureManager::Instance()->drawText("press S to start game", 10, 60, game->m_pRenderer, { 255,255,255,100 });
		TheTextureManager::Instance()->drawText("press Q to quit game", 10, 90, game->m_pRenderer, { 255,255,255,100 });
	}

	for (auto &obj : world.objs)
	{
		if (Snakable* snakable = getAttr<Snakable>(*obj))
		{
			for(auto &body:snakable->body)
			{
				TheTextureManager::Instance()->draw(TheTextureManager::TextId::self_snake, 
					(body.c - 1) * game->starter.width, (body.r - 1) * game->starter.height, 
					game->starter.pxSize, game->starter.pxSize, game->m_pRenderer);
			}
		}

		if(Eatable* eatable = getAttr<Eatable>(*obj))
		{
			if(Position * pos = getAttr<Position>(*obj))
			{
				TheTextureManager::Instance()->draw(TheTextureManager::TextId::ball, pos->point.c * 20, pos->point.r * 20, 20, 20, game->m_pRenderer);
			}
		}
	}

	SDL_RenderPresent(game->m_pRenderer.get());
}


void network_system(World& world)
{
	send_message_mutex.lock();
	try
	{
		string tmp = send_message_buffer;
		send_message_buffer = "";
		map<string, string> kv = deserial_item_map(tmp);

		if(kv[Constant::self_id] == to_string(world.self_id))
		{
			if(auto &p =  get_self_snake(world))
			{
				if(auto snake = getAttr<Snakable>(*p) )
				{
					auto next_direction = convertor[kv[Constant::press_key]];
					if(!isConverseDirect(next_direction, snake->direction))
					{
						snake->next_direction = next_direction;
					}
				}
			}
		}
	}catch(...)
	{
		cout << "network system error" << endl;
	}
	send_message_mutex.unlock();
}
