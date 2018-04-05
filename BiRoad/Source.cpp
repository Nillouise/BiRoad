#include<time.h>
#include <asio.hpp>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>
#include "Game.h"
#include "Server.h"
#include <thread>
#include "Client.h"
#include "Tool.h"
#include "Constant.h"

Game *g_game;

void server(int port,int groupSize)
{
	Server server(port, groupSize);
	server.init(100, 100, 5);
}


void client(const string &ip, int port)
{
	std::cout << "launch theClient" << std::endl;
	shared_ptr<Client> ptr = make_shared<Client>(ip, port);
	Tool::theClient(ptr);
	ptr->ioService.run();
}


int main(int argc, char* argv[])
{
	//	加了下面两句即可在/SUBSYSTEM:WINDOWS 的条件下用控制台输出
	AllocConsole();
	freopen("CONOUT$", "w", stdout);

	Starter starter;
	starter.init();
	//TODO： 这里应该判断一下是否启动server
	std::thread t1(server, 4444, 1);
	std::thread t2(client, "127.0.0.1", 4444);
	while (!Tool::theClient()->isInit&&!Tool::theClient()->isDown){};
	starter.width = stoi(Tool::theClient()->initData[Constant::GameMsg::width]);
	starter.height = stoi(Tool::theClient()->initData[Constant::GameMsg::height]);
	starter.pxSize = stoi(Tool::theClient()->initData[Constant::GameMsg::pxWidth]);

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


	t1.join();

	return 0;
}
