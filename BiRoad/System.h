#pragma once
#include <set>
#include "World.h"
#include "Game.h"

void eatable_system(World &world);

void obstacle_system(World &world);

void snakable_system(World &world);

void death_system(World &world);

void input(World &world, const std::string &keyname);

void render_system(World &world, Game *game);