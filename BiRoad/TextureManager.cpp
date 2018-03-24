#include "TextureManager.h"
#include <SDL_image.h>
#include <SDL.h>
std::shared_ptr<TextureManager> TextureManager::s_pInstance = nullptr;


bool TextureManager::load(const std::string &fileName, TextId id, std::shared_ptr<SDL_Renderer> pRenderer)
{
	SDL_Surface* pTempSurface = IMG_Load(fileName.c_str());
	if (pTempSurface == nullptr)
	{
		return false;
	}
	SDL_Texture* pTexture =
		SDL_CreateTextureFromSurface(pRenderer.get(), pTempSurface);
	SDL_FreeSurface(pTempSurface);
	// everything went ok, add the texture to our list
	if (pTexture != nullptr)
	{
		m_textureMap[id] = std::shared_ptr<SDL_Texture>(pTexture,
			[](SDL_Texture*p)
		{
			SDL_DestroyTexture(p);
		});
		return true;
	}
	// reaching here means something went wrong
	return false;
}


bool TextureManager::loadRect(TextId id, std::shared_ptr<SDL_Renderer> pRenderer, int width, int height, int r, int g, int b)
{
	SDL_Surface *pTempSurface = nullptr;
	pTempSurface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);
	SDL_FillRect(pTempSurface, nullptr, SDL_MapRGB(pTempSurface->format, r, g, b));
	SDL_Texture* pTexture =
		SDL_CreateTextureFromSurface(pRenderer.get(), pTempSurface);
	SDL_FreeSurface(pTempSurface);
	if (pTexture != nullptr)
	{
		m_textureMap[id] = std::shared_ptr<SDL_Texture>(pTexture,
			[](SDL_Texture*p)
		{
			SDL_DestroyTexture(p);
		});
		return true;
	}
	return false;
}


void TextureManager::draw(TextId id, int x, int y, int
	width, int height, std::shared_ptr<SDL_Renderer> pRenderer,
	SDL_RendererFlip flip)
{
	SDL_Rect srcRect;
	SDL_Rect destRect;
	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.w = destRect.w = width;
	srcRect.h = destRect.h = height;
	destRect.x = x;
	destRect.y = y;
	SDL_RenderCopyEx(pRenderer.get(), m_textureMap[id].get(), &srcRect,
		&destRect, 0, nullptr, flip);
}


/**
 * FIXME：这里可能不用cache会比较好
 */
void TextureManager::drawText(const std::string& text, int x, int y, std::shared_ptr<SDL_Renderer> pRenderer, SDL_Color White)
{
	//std::cout << "ERROR:" << TTF_GetError() << std::endl; //此函数可以获得TTF执行过程中的错误

	static std::map<std::string, SDL_Texture*> cache;

	if (cache.find(text) == cache.end())
	{
		SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans.get(), text.c_str(), White); // as TTF_RenderText_Solid could only be used on SDL_Surface then you have to create the surface first
		SDL_Texture* Message = SDL_CreateTextureFromSurface(pRenderer.get(), surfaceMessage); //now you can convert it into a texture
		SDL_FreeSurface(surfaceMessage);
		cache[text] = Message;
	}


	SDL_Texture* Message = cache[text]; //now you can convert it into a texture

	SDL_Rect Message_rect; //create a rect
	Message_rect.x = x;  //controls the rect's c coordinate 
	Message_rect.y = y; // controls the rect's r coordinte
	Message_rect.w = text.length() * 16; // controls the width of the rect
	Message_rect.h = 30; // controls the height of the rect

						 //Mind you that (0,0) is on the top left of the window/screen, think a rect as the text's box, that way it would be very simple to understance

						 //Now since it's a texture, you have to put RenderCopy in your game loop area, the area where the whole code executes

	SDL_RenderCopy(pRenderer.get(), Message, nullptr, &Message_rect); //you put the renderer's name first, the Message, the crop size(you can ignore this if you don't want to dabble with cropping), and the rect which is the size and coordinate of your texture

															 //Don't forget too free your surface and texture
}


void TextureManager::drawFrame(TextId id, int x, int y, int
	width, int height, int currentRow, int currentFrame, 
	std::shared_ptr<SDL_Renderer> pRenderer, SDL_RendererFlip flip)
{
	SDL_Rect srcRect;
	SDL_Rect destRect;
	srcRect.x = width * currentFrame;
	srcRect.y = height * (currentRow - 1);
	srcRect.w = destRect.w = width;
	srcRect.h = destRect.h = height;
	destRect.x = x;
	destRect.y = y;
	SDL_RenderCopyEx(pRenderer.get(), m_textureMap[id].get(), &srcRect,
		&destRect, 0, nullptr, flip);
}
