#include <iostream>
#include <memory>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cstring>

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

	struct numericInput{

		int x, y;
		int w, h;

		bool selected;

		double* var;

		SDL_Rect body;

		char buffer[32];
		int bufferIdx;

		void parseInput();
		int readInput(SDL_Event* _events, const Uint8* _kbstate);

		numericInput(double* _var, int _x, int _y, int _w, int _h);
	};

	struct Button{

		int ID;
		int x, y;
		int w, h;

		std::unique_ptr<numericInput> nmi;

		std::string displayName;

		bool clicked;

		SDL_Rect body;

		int mouseOnButton(Sint32 mouseX, Sint32 mouseY);

		void click();

		Button(int _ID, int _x, int _y, int _w, int _h, std::string _displayName, bool _clicked, double* numInVar = nullptr);
	};

	std::vector<Button> buttons;

	static bool show;
	void render(SDL_Renderer* renderer, textRenderer* txtRenderer);
};

class MainMenu : public Menu{

	public:

	MainMenu(std::vector<std::pair<std::string, double*>> vars);
};

void handleMouseInput(SDL_Event* _events, Camera &_cam, float deltaTime, bool mouseDisabled, std::vector<Menu*> &menus);
void handleKeyboardInput(SDL_Event* _events, const Uint8* _kbstate, Camera &_cam, float deltaTime, bool &mouseDisabled, std::vector<Menu*> &menus);


