#include "Input.hpp"

bool Menu::show = false;

Parser::Parser(int argc, char* argv[]){

	flags.isCamDecoy = false;
	flags.mouseDisabled = false;
	flags.renderLabels = false;
	flags.lock = false;
	flags.lockname = "";
	flags.traces = false;
	flags.forwardTraces = false;

	char* wrongArgs[16];
	int waSize = 0;

	for(int i = 1; i < argc; i++){

		if(!strcmp(argv[i], "--nm")){

			flags.mouseDisabled = true;
		}
		else if(!strcmp(argv[i], "--decoy")){

			flags.isCamDecoy = true;
		}
		else if(!strcmp(argv[i], "--labels")){

			flags.renderLabels = true;
		}
		else if(!strcmp(argv[i], "--lock")){

			if(i + 1 < argc){

				flags.lock = true;
				flags.lockname = argv[++i];
			}
		}
		else if(!strcmp(argv[i], "--traces")){

			flags.traces = true;
		}

		else if(!strcmp(argv[i], "--forward")){

			flags.forwardTraces = true;
		}
		else{
			wrongArgs[waSize++] = argv[i];
		}
	}

	for(int i = 0; i < waSize; i++){

		std::cout << "Invalid argument: " << wrongArgs[i] << std::endl;
	}
}

void Menu::Button::click(){

	clicked = !clicked;
};

Menu::Button::Button(int _ID, int _x, int _y, int _w, int _h, std::string _displayName, bool _clicked) 
	: ID(_ID), x(_x), y(_y), w(_w), h(_h), displayName(_displayName), clicked(_clicked){

	body.x = _x;
	body.y = _y;
	body.w = _w;
	body.h = _h;
}

int Menu::Button::mouseOnButton(Sint32 mouseX, Sint32 mouseY){

	if(mouseX >= x && mouseX <= (x + w) && mouseY >= y && mouseY <= (y + h)){

		return 1;
	}

	return 0;
}

void Menu::render(SDL_Renderer* renderer, textRenderer* txtRenderer){

	Uint8 oldR, oldG, oldB, oldA;

	SDL_GetRenderDrawColor(renderer, &oldR, &oldG, &oldB, &oldA);

	SDL_Color textColor = {255, 255, 255, 255};

	for(const Button &bt : buttons){

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(renderer, &bt.body);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawRect(renderer, &bt.body);
	
		int txtW;
		TTF_SizeText(txtRenderer->font, bt.displayName.c_str(), &txtW, nullptr);

		if(bt.clicked){

			textColor = {0, 255, 0, 255};
		}
		else{
			textColor = {255, 0, 0, 255};
		}

		txtRenderer->renderText(bt.x + (bt.w / 2) - (txtW / 2), bt.y + (bt.h / 2), bt.displayName, textColor);
	}

	SDL_SetRenderDrawColor(renderer, oldR, oldG, oldB, oldA);
}

MainMenu::MainMenu(){

	show = false;

	buttons.emplace_back(1, 10, 50, 150, 100, "Test1");
	buttons.emplace_back(2, 10, 200, 150, 100, "Test2");
	buttons.emplace_back(3, 10, 350, 150, 100, "Test3");
};

void handleMouseInput(SDL_Event* _events, Camera &_cam, float deltaTime, bool mouseDisabled, std::vector<Menu*> &menus){

	static bool mousePressed = (_events->type == SDL_MOUSEBUTTONDOWN && _events->button.state == SDL_PRESSED) ? true : false;

	if(!mouseDisabled){
		
		if(_events->type == SDL_MOUSEMOTION){

			if(_events->motion.yrel){
			
				double inputTiltX = _events->motion.yrel * (MOUSESENS / _cam.fov);
		
				if((_cam.tiltX - inputTiltX) < -1.57){
			
					_cam.tiltX = -1.57;
				}
				else if((_cam.tiltX - inputTiltX) > 1.57){
				
					_cam.tiltX = 1.57;
				}
				else{
					
					_cam.tiltX -= inputTiltX;
				}
			}

			if(_events->motion.xrel){

				_cam.tiltY = std::fmod(_cam.tiltY + (_events->motion.xrel * (MOUSESENS / _cam.fov)), 2 * PI);
			}
		}

		if(_events->type == SDL_MOUSEWHEEL){

			if(_events->wheel.y > 0 || _cam.fov + FOVSPEED * _events->wheel.y * 1.5 > MINFOV){

				_cam.fov += FOVSPEED * _events->wheel.y * 1.5;
			}
		}
	}
	else{

		if(Menu::show){

			for(auto &menu : menus){

				if(mousePressed == false){

					if(_events->type == SDL_MOUSEBUTTONDOWN && _events->button.state == SDL_PRESSED && mousePressed == false){

						mousePressed = true;
						for(auto &bt : menu->buttons){

							if(bt.mouseOnButton(_events->button.x, _events->button.y)){

								bt.click();
							}
						}
					}
				}
				else if(_events->type == SDL_MOUSEBUTTONUP && _events->button.state == SDL_RELEASED){

					mousePressed = false;
				}
			}
		}
	}
}

void handleKeyboardInput(SDL_Event* _events, const Uint8* _kbstate, Camera &_cam, float deltaTime, bool &mouseDisabled){

	int SPEEDVAR = SPEED;

	static bool keyPressedMenu = _kbstate[SDL_SCANCODE_M] ? true : false;

	if(_kbstate[SDL_SCANCODE_M] && keyPressedMenu == false){

		Menu::show = !Menu::show;
		mouseDisabled = !mouseDisabled;

		keyPressedMenu = true;
	}
	else if(!_kbstate[SDL_SCANCODE_M] && keyPressedMenu == true){

		keyPressedMenu = false;
	}



	//increase speed
	if(_kbstate[SDL_SCANCODE_LSHIFT]){

		SPEEDVAR *= SPEEDSHIFTMOD;
	}

	//_camera Z
	if(_kbstate[SDL_SCANCODE_W]){

		_cam.moveZ(SPEEDVAR, deltaTime); // negative due to inverted Y in SDL
	}
	if(_kbstate[SDL_SCANCODE_S]){

		_cam.moveZ(-SPEEDVAR, deltaTime);
	}

	//_camera X
	if(_kbstate[SDL_SCANCODE_A]){

		_cam.moveX(-SPEEDVAR, deltaTime);
	}
	if(_kbstate[SDL_SCANCODE_D]){

		_cam.moveX(SPEEDVAR, deltaTime);
	}

	//_camera Y
	if(_kbstate[SDL_SCANCODE_SPACE]){

		_cam.moveY(-SPEEDVAR, deltaTime);
	}
	if(_kbstate[SDL_SCANCODE_LCTRL]){

		_cam.moveY(SPEEDVAR, deltaTime);
	}
	
	//camera fov
	if(_kbstate[SDL_SCANCODE_MINUS] && (_cam.fov - FOVSPEED * deltaTime) > MINFOV){
	
		_cam.fov -= FOVSPEED * deltaTime;
	}
	if(_kbstate[SDL_SCANCODE_EQUALS]){

		_cam.fov += FOVSPEED * deltaTime;
	}
	
	//camera tilt X
	if(_kbstate[SDL_SCANCODE_UP]){

		_cam.tiltX = _cam.tiltX + (CAMSPEED * deltaTime * (15 / _cam.fov)) > 1.57 ? 1.57 : _cam.tiltX + (CAMSPEED * deltaTime * (15 / _cam.fov));
	}
	if(_kbstate[SDL_SCANCODE_DOWN]){

		_cam.tiltX = _cam.tiltX - (CAMSPEED * deltaTime * (15 / _cam.fov)) < -1.57 ? -1.57 : _cam.tiltX - (CAMSPEED * deltaTime * (15 / _cam.fov));
	}

	//camera tilt Y
	//
	//value is multiplied by 1500 / fov to make it adapt on distance
	if(_kbstate[SDL_SCANCODE_LEFT]){

		_cam.tiltY = std::fmod(_cam.tiltY - (CAMSPEED * deltaTime * (15 / _cam.fov)), 2 * PI);
	}
	if(_kbstate[SDL_SCANCODE_RIGHT]){

		_cam.tiltY = std::fmod(_cam.tiltY + (CAMSPEED * deltaTime * (15 / _cam.fov)), 2 * PI);
	}
}


