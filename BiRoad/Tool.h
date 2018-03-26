#pragma once
#include <string>
#include <vector>
#include "Point.h"
#include <random>
#include"World.h"

class Tool
{
public:
	static std::vector<std::string> split(const std::string& s, const std::string &delimiter);
	static Point getRandomBall(int height, int width);
	static bool newBallConflictWithWorld(const World &world, const Point &p);
	template<typename T>
	static T* getAttr(const Object &obj);
	static std::shared_ptr<Object> get_self_snake(World &world);
	static std::shared_ptr<Object> get_snake(World &world, int id);
	static bool isConverseDirect(const Direction::direction_enum &e1, const Direction::direction_enum &e2);
	static Point nextDirectPoint(const Direction::direction_enum &e, const Point &p);
	static bool noOuterPoint(const World &world, const Point &p);

	static string serial_map(const std::map<string, string> &keyval);
	//这里是反序列化一个user内的所有item
	static std::map<string, string> deserial_item_map(const string &s);

};


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