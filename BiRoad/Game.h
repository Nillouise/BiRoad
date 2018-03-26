#pragma once
#include "Starter.h"
#include <SDL.h>
#include <memory>
#include "World.h"

using std::shared_ptr;
class Game
{
	friend void render_system(World &world, Game *game);
	friend void robot(World &world, int snakeId);
public:
	bool init(Starter &starter);
	void handleEvents();
	void update();
	void render();
	void clean();
	// a function to access the private running variable
	bool running() const { return m_bRunning; }
private:
	Starter starter;
	int m_selfSnakeColor[3] = { 255,0,0 };
	int m_ballColor[3] = { 255,255,0 };
	int m_screenWidth = 0;
	int m_screenHeight = 0;
	bool m_printHint = false;
	bool m_bRunning = false;
	shared_ptr<SDL_Window> m_pWindow;
	shared_ptr<SDL_Renderer> m_pRenderer;
	//此函数是用来加载SDL窗口资源，拆出来是因为这部分太长了
	bool initRender(Starter& starter);
	World world;
};

extern Game *g_game;