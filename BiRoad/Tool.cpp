#include "Tool.h"
#include "Snakable.h"
#include "Eatable.h"
#include "Position.h"

std::vector<std::string> Tool::split(const std::string& s, const std::string& delimiter)
{
	std::vector<std::string> res;
	auto start = 0U;
	auto end = s.find(delimiter);
	while (end != std::string::npos)
	{
		res.push_back(s.substr(start, end - start));
		start = end + delimiter.length();
		end = s.find(delimiter, start);
	}
	res.push_back(s.substr(start, end));
	return res;
}

Point Tool::getRandomBall(int height, int width)
{
	static bool init = false;
	static std::mt19937 rng;
	if (!init)
	{
		rng.seed(std::random_device()());
		init = true;
	}
	std::uniform_int_distribution<std::mt19937::result_type> randomHeight(0+1, height);
	std::uniform_int_distribution<std::mt19937::result_type> randomWidth(0+1, width);
	return Point(randomHeight(rng), randomWidth(rng));
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
template <typename T>
T* Tool::getAttr(const Object& obj)
{
	std::map<string, std::shared_ptr<ECS>>::const_iterator a = obj.attributes.find(typeid(T).name());
	if (a == obj.attributes.end())
	{
		return nullptr;
	}
	else
	{
		return dynamic_cast<T*>(a->second.get());
	}
}