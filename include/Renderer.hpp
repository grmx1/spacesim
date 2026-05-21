#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <sstream>
#include <vector>
#include <utility>

#include "defines.hpp"

inline int RES[2] = {2560,1440};
inline double centerX = RES[0] / 2;
inline double centerY = RES[1] / 2;

class textRenderer{

    public:

        SDL_Renderer* renderer;
        TTF_Font* font;

        textRenderer(SDL_Renderer* ren, const char* fontPath, int fontSize);

        ~textRenderer();

        void renderText(int x, int y, const std::string& text, SDL_Color color);

	template <typename T>
	void renderVariable(int x, int y, const std::string& label, T value, SDL_Color color){

		std::ostringstream oss;
		oss << label << ": " << value;
		renderText(x, y, oss.str(), color);
	}
};

class Hud{

	private:

	struct{
	
		int length = 10;
		SDL_Color color = {255, 255, 255, 255};
	} crosshair;
	
	public:

	std::vector<std::pair<std::string, double*>> stats;

	void renderCrossHair(SDL_Renderer* renderer);
	
	void renderStats(textRenderer* _txtRenderer){

		for(int i = 0; i < stats.size(); i++){


			_txtRenderer->renderVariable(10, i * 20 + 20, stats[i].first, *stats[i].second, {255, 255, 255});
		}
	}


};
