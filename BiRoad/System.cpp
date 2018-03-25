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
#include <queue>
#include<array>


using std::to_string;
using std::map;
using std::cout;
using std::endl;
using std::vector;

namespace {
	std::string receive_message_buffer;
	std::mutex receiver_message_mutex;
	std::string send_message_buffer;
	std::mutex send_message_mutex;


	std::map<std::string, Direction::direction_enum> convertor{ 
		{ Constant::up,Direction::up }, { Constant::right,Direction::right },
		{ Constant::down,Direction::down },{ Constant::left,Direction::left }
	};

	std::map<Direction::direction_enum,std::string> convertor2{
		{ Direction::up,Constant::up },{ Direction::right,Constant::right},
		{ Direction::down,Constant::down},{ Direction::left,Constant::left}
	};

	string serial_map(const map<string, string> &keyval)
	{
		string res;
		int i = 0;
		for (auto a : keyval)
		{
			if (i != 0)
			{
				res += Constant::item_delimiter;
			}
			i++;
			res += a.first + Constant::equal_delimiter + a.second;
		}
		return res;
	}


	//这里是反序列化一个user内的所有item
	map<string, string> deserial_item_map(const string &s)
	{
		map<string, string> res;
		auto items = Tool::split(s, Constant::item_delimiter);
		for (auto &item : items)
		{
			auto keyval = Tool::split(item, Constant::equal_delimiter);
			if (keyval.size() == 2)
			{
				res[keyval[0]] = keyval[1];
			}
		}
		return res;
	}


	bool collided(std::vector<Object*> &balls, const Snakable *snake)
	{
		for (auto ball : balls)
		{
			if (Position *e = Tool::getAttr<Position>(*ball))
			{
				for (const auto &s : snake->body)
				{
					if (s.c == e->point.c&&s.r == e->point.r)
					{
						return true;
					}
				}
			}
		}
		return false;
	}


	bool outOfWorld(const World &world, const Snakable *snake)
	{
		for (auto &a : snake->body)
		{
			if (a.c<1 || a.c>world.width || a.r<1 || a.r>world.height)
			{
				return true;
			}
		}
		return false;
	}
}


void eatable_system(World& world)
{
	std::vector<Object*> balls;

	for (auto &obj : world.objs)
	{
		if (Eatable *e = Tool::getAttr<Eatable>(*obj))
		{
			balls.push_back(obj.get());
		}
	}
	for (auto &a : world.objs)
	{
		if (Snakable *s = Tool::getAttr<Snakable>(*a))
		{
			//如果蛇没有碰到球，尾端应该要被删除的
			bool isCollided = false;
			for (auto ball : balls)
			{
				if (Position *e = Tool::getAttr<Position>(*ball))
				{
					for (const auto &u : s->body)
					{
						if (u.c == e->point.c&&u.r == e->point.r)
						{
							if (Eatable *b = Tool::getAttr<Eatable>(*ball))
							{
								b->isEated = true;
							}
							isCollided = true;
							break;
						}
					}
				}

			}
			if (!isCollided)
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
		if (Snakable *e = Tool::getAttr<Snakable>(*obj))
		{
			snakes.push_back(obj.get());
		}
	}
	std::map<Point, int> maze;
	for (auto a : snakes)
	{
		Snakable *e1 = Tool::getAttr<Snakable>(*a);
		if (outOfWorld(world, e1))
		{
			if (!e1->isCollided)
			{
				++world.current_frame_has_collide;
			}
			e1->isCollided = true;
		}
		for (auto &b : e1->body)
		{
			if (++maze[b] > 1)
			{
				if (!e1->isCollided)
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
	static std::map<Direction::direction_enum, std::pair<int, int>> offset =
	{
		{Direction::up,{0,-1}},
		{Direction::down,{0,1}},
		{Direction::left,{-1,0}},
		{Direction::right,{1,0}},
	};
	for (auto &obj : world.objs)
	{
		if (Snakable* p = Tool::getAttr<Snakable>(*obj))
		{
			Point head = Tool::nextDirectPoint(p->next_direction, p->body.front());
			p->direction = p->next_direction;
			p->body.push_front(head);
		}
	}
}


void death_system(World &world)
{
	//蛇与蛇自己的碰撞
	for (auto a = world.objs.begin(); a != world.objs.end();)
	{
		if (Snakable *e = Tool::getAttr<Snakable>(**a))
		{
			if (e->isCollided)
			{
				a = world.objs.erase(a);
			}
			else
			{
				++a;
			}
		}
		else
		{
			++a;
		}
	}
	//ball被蛇碰到
	int disappearball = 0;
	for (auto a = world.objs.begin(); a != world.objs.end();)
	{
		if (Eatable *e = Tool::getAttr<Eatable>(**a))
		{
			if (e->isEated)
			{
				a = world.objs.erase(a);
				disappearball++;
			}
			else
			{
				++a;
			}
		}
		else
		{
			++a;
		}
	}
	for (int i = 0; i < disappearball; i++)
	{
		Point p;
		do
		{
			p = Tool::getRandomBall(world.height, world.width);
		} while (Tool::newBallConflictWithWorld(world, p));
		auto a = Object::factoryBall(p);
		world.objs.insert(a);
	}
}


void input(World &world, const std::string &keyname)
{
	send_message_mutex.lock();
	try
	{
		auto dire = convertor[keyname];
		if (auto obj = Tool::get_self_snake(world))
		{
			if (Snakable *snake = Tool::getAttr<Snakable>(*obj))
			{
				if (!Tool::isConverseDirect(snake->direction.direction, dire))
				{
					map<string, string> keyval;
					keyval[Constant::current_frame_numb] = to_string(world.current_frame_numb);
					keyval[Constant::press_key] = keyname;
					keyval[Constant::self_id] = to_string(world.self_id);
					send_message_buffer = serial_map(keyval);
				}
			}
		}
	}
	catch (...)
	{
		std::cerr << "inpute error" << std::endl;
	}
	send_message_mutex.unlock();
}

void input(World &world,Direction::direction_enum dir)
{
	input(world, convertor2[dir]);
}

void robot(World &world, int snakeId)
{
	if (shared_ptr<Object> obj = Tool::get_snake(world, snakeId))
	{
		if (Snakable *snake = Tool::getAttr<Snakable>(*obj))
		{
			struct Star
			{
				Point p = { 0,0 };
				int real = 0, predict = 0;
				Direction::direction_enum dir;
				Star* prePoint = nullptr;
				int lenToStart = 10000;
				bool operator<(const Star &rhs)const
				{
					return real + predict < rhs.real + rhs.predict;
				}
				static int nextPrediction(World &world, const Point &point)
				{
					int shortPath = 1e8;
					for (auto &a : world.objs)
					{
						if (Eatable *e = Tool::getAttr<Eatable>(*a))
						{
							if (Position *p = Tool::getAttr<Position>(*a))
							{
								shortPath = std::min(shortPath, abs(p->point.c - point.c) + abs(p->point.r - point.r));
							}
						}
					}
					return shortPath;
				}
			};

			vector<vector<int>> obstacle(world.height, vector<int>(world.width, 0));
			vector<vector<std::array<Star, 4>>> dist(world.height, vector<std::array<Star, 4>>(world.width));

			//设置obstacle表
			for (auto &o : world.objs)
			{
				if (auto obs = Tool::getAttr<Obstacle>(*o))
				{
					if (auto pos = Tool::getAttr<Snakable>(*o))
					{
						for (Point &p : pos->body)
						{
							obstacle[p.r][p.c] = 1;
						}
					}
					else if (auto pos = Tool::getAttr<Position>(*o))
					{
						obstacle[pos->point.r][pos->point.c] = 1;
					}
				}
				else
				{
					if (auto eat = Tool::getAttr<Eatable>(*o))
					{
						if (auto pos = Tool::getAttr<Position>(*o))
						{
							obstacle[pos->point.r][pos->point.c] = 2;
						}
					}
				}
			}

			std::priority_queue<Star*> q;
			dist[snake->body.begin()->r][snake->body.begin()->c][snake->direction] =
				{ *snake->body.begin(),0, Star::nextPrediction(world,*snake->body.begin()),snake->direction,nullptr,0 };
			q.push(&dist[snake->body.begin()->r][snake->body.begin()->c][snake->direction]);

			Direction::direction_enum resNextStep;

			while (!q.empty())
			{
				Star *u = q.top(); q.pop();
				if (obstacle[u->p.r][u->p.c] == 2)
				{
					Star *prePoint = u;
					Star *curPoint = u;
					while (curPoint->prePoint != nullptr)
					{
						resNextStep = prePoint->dir;
						prePoint = curPoint;
					}
					break;
				}
				using E = Direction::direction_enum;
				for (auto en : { E::up, E::down,E::left,E::right })
				{
					if (!Tool::isConverseDirect(en, u->dir))
					{
						Point nextPoint = Tool::nextDirectPoint(en, u->p);
						if (Tool::noOuterPoint(world, nextPoint))
						{
							auto &dest = dist[nextPoint.r][nextPoint.c][en];
							auto &source = dist[u->p.r][u->p.c][u->dir];
							if (obstacle[nextPoint.r][nextPoint.c] != 1 && dest.lenToStart < source.lenToStart + 1)
							{
								dest.lenToStart = source.lenToStart + 1;
								dest.p = nextPoint;
								dest.dir = en;
								dest.prePoint = &source;
								dest.predict = Star::nextPrediction(world, nextPoint);
								dest.real = dest.lenToStart;
								q.push(&dist[dest.p.r][dest.p.c][dest.dir]);
							}
						}
					}
				}
			}

			input(world, resNextStep);
		}
	}
}



void render_system(World &world, Game *game)
{
	SDL_RenderClear(game->m_pRenderer.get()); // clear the renderer to the draw color

	if (game->m_printHint)
	{
		TheTextureManager::Instance()->drawText("press S to start game", 10, 60, game->m_pRenderer, { 255,255,255,100 });
		TheTextureManager::Instance()->drawText("press Q to quit game", 10, 90, game->m_pRenderer, { 255,255,255,100 });
	}

	for (auto &obj : world.objs)
	{
		if (Snakable* snakable = Tool::getAttr<Snakable>(*obj))
		{
			for (auto &body : snakable->body)
			{
				TheTextureManager::Instance()->draw(TheTextureManager::TextId::self_snake,
					(body.c - 1) * game->starter.pxSize, (body.r - 1) * game->starter.pxSize,
					game->starter.pxSize, game->starter.pxSize, game->m_pRenderer);
			}
		}

		if (Eatable* eatable = Tool::getAttr<Eatable>(*obj))
		{
			if (Position * pos = Tool::getAttr<Position>(*obj))
			{
				TheTextureManager::Instance()->draw(TheTextureManager::TextId::ball,
					(pos->point.c - 1) * game->starter.pxSize,
					(pos->point.r - 1) * game->starter.pxSize,
					game->starter.pxSize,
					game->starter.pxSize,
					game->m_pRenderer);
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

		if (kv[Constant::self_id] == to_string(world.self_id))
		{
			if (auto p = Tool::get_self_snake(world))
			{
				if (auto snake = Tool::getAttr<Snakable>(*p))
				{
					auto next_direction = convertor[kv[Constant::press_key]];
					if (!Tool::isConverseDirect(next_direction, snake->direction))
					{
						snake->next_direction = next_direction;
					}
				}
			}
		}
	}
	catch (...)
	{
		cout << "network system error" << endl;
	}
	send_message_mutex.unlock();
}
