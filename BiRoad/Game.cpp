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

