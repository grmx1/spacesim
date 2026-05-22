#include "Input.hpp"

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

void handleInput(SDL_Event* _events, const Uint8* _kbstate, Camera &_cam, float deltaTime, bool mouseDisabled){

	int SPEEDVAR = SPEED;

	while(SDL_PollEvent(_events)){
	
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


