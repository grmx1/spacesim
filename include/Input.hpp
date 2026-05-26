#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "defines.hpp"
#include "SpaceObject.hpp"

class Parser{

	public:

	struct{

		bool isCamDecoy;
		bool mouseDisabled;
		bool renderLabels;
		bool lock;
		std::string lockname;
		bool traces;
		bool forwardTraces;
	} flags;

	Parser(int argc, char* argv[]);
};

class Menu{

	public:

	struct Button{

		int ID;
		int x, y;
		int w, h;

		std::string displayName;

		bool clicked;

		SDL_Rect body;

		int mouseOnButton(Sint32 mouseX, Sint32 mouseY);

		void click();

		Button(int _ID, int _x, int _y, int _w, int _h, std::string _displayName, bool _clicked = false);
	};

	std::vector<Button> buttons;

	static bool show;
	void render(SDL_Renderer* renderer, textRenderer* txtRenderer);
};

class MainMenu : public Menu{

	public:

	MainMenu();
};

void handleMouseInput(SDL_Event* _events, Camera &_cam, float deltaTime, bool mouseDisabled, std::vector<Menu*> &menus);
void handleKeyboardInput(SDL_Event* _events, const Uint8* _kbstate, Camera &_cam, float deltaTime, bool &mouseDisabled);


