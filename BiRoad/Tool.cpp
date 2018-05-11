#include "Tool.h"
#include "Snakable.h"
#include "Eatable.h"
#include "Position.h"
#include "Constant.h"
using std::map;
std::default_random_engine Tool::clientRandomEngine;
std::vector<std::string> Tool::split(const std::string& s, const std::string& delimiter)
{
	std::vector<std::string> res;
	auto start = 0U;
	auto end = s.find(delimiter);
	while (end != std::string::npos)
	{
		res.push_back(s.substr(start, end - start));
		start = end + (int)delimiter.length();
		end = s.find(delimiter, start);
	}
	res.push_back(s.substr(start, end));
	return res;
}


Point Tool::getRandomBall(int height, int width)
{
	std::uniform_int_distribution<std::mt19937::result_type> randomHeight(0+1, height);
	std::uniform_int_distribution<std::mt19937::result_type> randomWidth(0+1, width);
	return Point(randomHeight(clientRandomEngine), randomWidth(clientRandomEngine));
}


bool Tool::newBallConflictWithWorld(const World& world,const Point &point)
{
	for(auto &a:world.objs)
	{
		if(auto p = getAttr<Snakable>(*a))
		{
			for(auto &b:p->body)
			{
				if(point==b)
				{
					return true;
				}
			}
		}
		if(auto p = getAttr<Eatable>(*a))
		{
			if(auto p = getAttr<Position>(*a))
			{
				if(p->point==point)
				{
					return true;
				}
			}
		}
	}
	return false;
}


std::shared_ptr<Object> Tool::get_self_snake(World& world)
{
	for (auto a : world.objs)
	{
		if (Snakable *s = Tool::getAttr<Snakable>(*a))
		{
			if (s->id == world.self_id)
			{
				return a;
			}
		}
	}
	return nullptr;
}


std::shared_ptr<Object> Tool::get_snake(World& world, int id)
{
	for (auto a : world.objs)
	{
		if (Snakable *s = Tool::getAttr<Snakable>(*a))
		{
			if (s->id == id)
			{
				return a;
			}
		}
	}
	return nullptr;
}


bool Tool::isConverseDirect(const Direction::direction_enum& e1, const Direction::direction_enum& e2)
{
	return (e1 == Direction::down && e2 == Direction::up)
		|| (e1 == Direction::up && e2 == Direction::down)
		|| (e1 == Direction::left && e2 == Direction::right)
		|| (e1 == Direction::right && e2 == Direction::left);
}


Point Tool::nextDirectPoint(const Direction::direction_enum& e,const Point &p)
{
	static std::map<Direction::direction_enum, std::pair<int, int>> offset =
	{
		{ Direction::up,{ 0,-1 } },
		{ Direction::down,{ 0,1 } },
		{ Direction::left,{ -1,0 } },
		{ Direction::right,{ 1,0 } },
	};
	Point res;
	res.c = p.c + offset[e].first;
	res.r = p.r + offset[e].second;
	return res;
}


bool Tool::noOuterPoint(const World& world, const Point& p)
{
	if(p.r<1||p.r>world.height||
		p.c<1||p.c>world.width)
	{
		return false;
	}
	return true;
}


string Tool::serial_map(const map<string, string> &keyval)
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
map<string, string> Tool::deserial_item_map(const string &s)
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

bool Tool::newlineEnd(string& s)
{
	if(!s.empty())
	{
		if(s.back()!='\n')
		{
			s += '\n';
			return true;
		}
	}
	return false;
}

namespace
{
	Client::pClient client;
	
}


Client::pClient Tool::theClient()
{
	return client;
}

void Tool::theClient(Client::pClient client)
{
	::client = client;
}
