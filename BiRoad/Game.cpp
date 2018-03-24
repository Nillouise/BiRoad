#include "Game.h"
#include<iostream>
#include "TextureManager.h"
#include "System.h"
using std::cout;
using std::shared_ptr;
namespace
{

}


bool Game::init(Starter& starter)
{
	if(!initRender(starter))
	{
		return false;
	}
	m_bRunning = true; // everything inited successfully,start the main loop
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

}

void Game::render()
{
	SDL_RenderClear(m_pRenderer.get()); // clear the renderer to the draw color
								  //暂停时，绘制相应的信息

	for(auto &obj :world.objs)
	{
		obj
	}


	if (g_starter->is_set_my_computer_to_server())
	{
		if (m_pauseTime > 0)
		{
			TheTextureManager::Instance()->drawText("press S to start game", 10, 60, m_pRenderer, { 255,255,255,100 });
			TheTextureManager::Instance()->drawText("press Q to quit game", 10, 90, m_pRenderer, { 255,255,255,100 });
		}
	}
	else
	{
		if (m_pauseTime > 0)
		{
			TheTextureManager::Instance()->drawText("wait for server to start game", 5, 5, m_pRenderer);
		}
	}
	//绘制ball
	for (auto a : m_interact->m_maze->balls)
	{
		TheTextureManager::Instance()->draw("ball", a.c * 20, a.r * 20, 20, 20, m_pRenderer);
	}
	//绘制snake
	for (auto a : m_interact->m_snakeMap)
	{
		auto b = a.second;
		for (auto a : b->body)
		{
			TheTextureManager::Instance()->draw("snakeUnit", a.c * 20, a.r * 20, 20, 20, m_pRenderer);
		}
	}
	SDL_RenderPresent(m_pRenderer.get()); // draw to the screen
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
		if (!m_pWindow) // window init success
		{
			std::cout << "window creation success\n";
			m_pRenderer = shared_ptr<SDL_Renderer>(
				SDL_CreateRenderer(m_pWindow.get(), -1, 0),
				[](SDL_Renderer *renderer)
			{
				SDL_DestroyRenderer(renderer);
			}
			);
			if (m_pRenderer != 0) // renderer init success
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
	if (!TheTextureManager::Instance()->load("assets/animate-alpha.png", TheTextureManager::TextId::animate, m_pRenderer))
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
	return true;
}

