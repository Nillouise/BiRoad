#pragma once
#include "ECS.h"
#include <map>
#include<memory>
#include <unordered_map>

using std::string;
namespace std
{
}


class Object
{
public:
	std::map<string,std::shared_ptr<ECS>> attributes;
	int id;
};
