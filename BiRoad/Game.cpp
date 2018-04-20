#include "Game.h"
#include <iostream>
#include "TextureManager.h"
#include "System.h"
#include "Snakable.h"
#include "Obstacle.h"
#include "Eatable.h"
#include "Position.h"
#include <boost/date_time/microsec_time_clock.hpp>
#include "Tool.h"
#include "Constant.h"
using std::cout;
using std::shared_ptr;
using std::make_shared;
namespace
{

}


bool Game::init(Starter& starter)
{
	m_bRunning = true; // everything inited successfully,start the main loop
	world.height = starter.height / starter.pxSize;
	world.width = starter.width / starter.pxSize;
	world.self_id = starter.selfId;
	this->starter = starter;

	if(!initRender(starter))
	{
		return false;
	}

	return true;
}

void Game::handleEvents()
{
	SDL_Event event;
	if (SDL_PollEvent(&event))
	{
		std::string keyname;
		switch (event.type)
		{
		case SDL_KEYDOWN:
			keyname = SDL_GetKeyName(event.key.keysym.sym);
			input(world, keyname);
			printf("key %s down! \n", keyname.c_str());
			break;
		case SDL_QUIT:
			m_bRunning = false;
			break;
		default:
			break;
		}
	}
}

void Game::update()
{
	static bool startGame = false;
	
	//这里利用client网络的信息生成蛇和球。这里只是在startgame的时候初始化信息
	if (!startGame)
	{
		std::stringstream ss(Tool::theClient()->getFrameMsg(true));
		string line;
		while (getline(ss,line))
		{
			using namespace Constant::GameMsg;
			std::map<string, string> m = Tool::deserial_item_map(line);
			if(m.find(Constant::GameMsg::isFinishInitMsg)!=m.end())
			{
				startGame = true;
			}else if(m.find(Constant::GameMsg::randomSeed)!=m.end())
			{
				string s = m[Constant::GameMsg::randomSeed];
				long long seed = std::stoll(s);
				Tool::clientRandomEngine.seed(seed);
			}else if(m.find(Constant::GameMsg::objType)!=m.end()&&m[Constant::GameMsg::objType]==Constant::GameMsg::snakeType)
			{
				shared_ptr<Object> snake = make_shared<Object>();
				shared_ptr<Snakable> snakable = std::make_shared<Snakable>();
				snakable->body.push_front(Point(stoi(m[pointC]),stoi(m[pointR])));
				snakable->id = stoi(m[snakeId]);
				snake->attributes[typeid(Snakable).name()] = snakable;
				snake->attributes[typeid(Obstacle).name()] = make_shared<Obstacle>();
				world.objs.insert(snake);
			}else if(m.find(Constant::GameMsg::objType)!=m.end()&&m[Constant::GameMsg::objType]==Constant::GameMsg::ballType)
			{
				shared_ptr<Object> ballType = make_shared<Object>();
				ballType->attributes[typeid(Eatable).name()] = make_shared<Eatable>();
				ballType->attributes[typeid(Position).name()] = make_shared<Position>(Point(stoi(m[pointC]), stoi(m[pointR])));
				world.objs.insert(ballType);
			}
		}
//		world.self_id = 0;
//		world.current_frame_numb = 0;
//
//		shared_ptr<Object> snakeType = make_shared<Object>();
//		shared_ptr<Snakable> snakable = std::make_shared<Snakable>();
//		snakable->body.push_front(Point(10, 10));
//		snakable->id = 0;
//		snakeType->attributes[typeid(Snakable).name()] = snakable;
//		snakeType->attributes[typeid(Obstacle).name()] = make_shared<Obstacle>();
//		world.objs.insert(snakeType);
//
//		shared_ptr<Object> ballType = make_shared<Object>();
//		ballType->attributes[typeid(Eatable).name()] = make_shared<Eatable>();
//		ballType->attributes[typeid(Position).name()] = make_shared<Position>(Point{ 7, 4 });
//		world.objs.insert(ballType);
	}

	static long long preUpdateTime = 0;
	if(!Tool::theClient()->popFrameMsg(false).empty())
	{
		preUpdateTime = GetTickCount();
		network_system(world);
		snakable_system(world);
		eatable_system(world);
		obstacle_system(world);
		death_system(world);
		world.current_frame_numb++;
		robot(world, world.self_id);
	}
}

void Game::render()
{
	render_system(world, this);
}

void Game::clean()
{
	std::cout << "cleaning game\n";
	m_pWindow = nullptr;
	m_pRenderer = nullptr;
	SDL_Quit();
}

bool Game::initRender(Starter& starter)
{
	int flags = 0;
	if (starter.fullScreen)
	{
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	// attempt to initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		std::cout << "SDL init success\n";
		// init the window
		m_pWindow = shared_ptr<SDL_Window>(
			SDL_CreateWindow(starter.title.c_str(), starter.xpos, starter.ypos, starter.width, starter.height, flags),
			[](SDL_Window* window)
		{
			SDL_DestroyWindow(window);
		}
		);
		if (m_pWindow) // window init success
		{
			std::cout << "window creation success\n";
			m_pRenderer = shared_ptr<SDL_Renderer>(
				SDL_CreateRenderer(m_pWindow.get(), -1, 0),
				[](SDL_Renderer *renderer)
			{
				SDL_DestroyRenderer(renderer);
			}
			);
			if (m_pRenderer) // renderer init success
			{
				std::cout << "renderer creation success\n";
				SDL_SetRenderDrawColor(m_pRenderer.get(), 100, 255, 255, 255);
			}
			else
			{
				std::cout << "renderer init fail\n";
				return false; // renderer init fail
			}
		}
		else
		{
			std::cout << "window init fail\n";
			return false; // window init fail
		}
	}
	else
	{
		std::cout << "SDL init fail\n";
		return false; // SDL init fail
	}
	std::cout << "init success\n";
	//init the resource
	if (!TheTextureManager::Instance()->load("resource/assets/animate-alpha.png", TheTextureManager::TextId::animate, m_pRenderer))
	{
		std::cout << "load assets folder error";
		return false;
	}
	if (!TheTextureManager::Instance()->loadRect(TheTextureManager::TextId::self_snake, m_pRenderer, 20, 20, m_selfSnakeColor[0], m_selfSnakeColor[1], m_selfSnakeColor[2]))
	{
		return false;
	}
	if (!TheTextureManager::Instance()->loadRect(TheTextureManager::TextId::ball, m_pRenderer, 20, 20, m_ballColor[0], m_ballColor[1], m_ballColor[2]))
	{
		return false;
	}
	if (!TheTextureManager::Instance()->loadRect(TheTextureManager::TextId::virtualPath, m_pRenderer, 20, 20, 255, 255,255))
	{
		return false;
	}
	return true;
}

