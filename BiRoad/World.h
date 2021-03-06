#pragma once
#include <set>
#include "Object.h"

class World
{
public:
	std::set< std::shared_ptr<Object>> objs;
	int current_frame_has_collide = 0;
	int current_frame_numb = 0;
	int width, height;
	int self_id;
};
