#pragma once
#include "ECS.h"
#include <map>
#include<memory>
class Object
{
public:
	std::map<type_info,std::shared_ptr<ECS>> attributes;
};