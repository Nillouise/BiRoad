#include<time.h>
#include <asio.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include "Game.h"
#include "Server.h"

Game *g_game;

int main(int argc, char* argv[])
{
	//	�����������伴����/SUBSYSTEM:WINDOWS ���������ÿ���̨���
	AllocConsole();
	freopen("CONOUT$", "w", stdout);

//	Starter starter;
//	starter.init();
//	Game game;
//	g_game = &game;
//	game.init(starter);
//
//	while(game.running())
//	{
//		game.handleEvents();
//		game.update();
//		game.render();
//	}
//	game.clean();
//
	Server server(4000,2);
	server.init();

	return 0;
}
