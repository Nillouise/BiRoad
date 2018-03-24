#pragma once
#include<iostream>
#include<SDL.h>
#include <SDL_ttf.h>
#include <map>
#include<memory>

class TextureManager
{
public:
	//Ã¿¸öäÖÈ¾Í¼Æ¬µÄid
	enum class TextId
	{
		self_snake,
		other_snake,
		ball,
		animate,
	};
	bool load(const std::string &fileName, TextId id, std::shared_ptr<SDL_Renderer> pRenderer);
	bool loadRect(TextId id, std::shared_ptr<SDL_Renderer> pRenderer, int width, int height, int r, int g, int b);
	void draw(TextId id, int x, int y, int width, int height,
		std::shared_ptr<SDL_Renderer> pRenderer, SDL_RendererFlip flip = SDL_FLIP_NONE);
	void drawText(const std::string &text, int x, int y, std::shared_ptr<SDL_Renderer> pRenderer, SDL_Color White = { 255,255,255 });
	void drawFrame(TextId id, int x, int y, int width,
		int height, int currentRow, int currentFrame, std::shared_ptr<SDL_Renderer> pRenderer,
		SDL_RendererFlip flip = SDL_FLIP_NONE);
	static std::shared_ptr<TextureManager> Instance()
	{
		if (!s_pInstance)
		{
			s_pInstance = std::shared_ptr<TextureManager>(new TextureManager());
			return s_pInstance;
		}
		return s_pInstance;
	}
private:
	TextureManager()
	{
		TTF_Init();
		Sans = std::shared_ptr<TTF_Font>(TTF_OpenFont("resource/ttf/OpenSans-Regular.ttf", 24),
			[](TTF_Font* p)
		{
			TTF_CloseFont(p);
		}); //this opens a font style and sets a size
	}

	static std::shared_ptr<TextureManager> s_pInstance;
	std::map<TextId, std::shared_ptr<SDL_Texture>> m_textureMap;
	std::shared_ptr<TTF_Font> Sans;
};
typedef TextureManager TheTextureManager;