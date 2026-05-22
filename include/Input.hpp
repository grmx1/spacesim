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

void handleInput(SDL_Event* _events, const Uint8* _kbstate, Camera &_cam, float deltaTime, bool mouseDisabled);
