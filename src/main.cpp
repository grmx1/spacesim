#include <iostream>
#include <vector>
#include <numbers>
#include <string>
#include <map>
#include <unordered_map>
#include <cstdlib>
#include <chrono>
#include <thread> //for fps rating not multithreading

#include "Renderer.hpp"
#include "SpaceObject.hpp"
#include "Input.hpp"
#include "defines.hpp"

//VERY IMPORTANT
//i have to detach all the physics from the rendering process
//that right now is joined most of the times making it impossible
//to, now that i have made time scales accurate ( i think ) be able
//to run a calculation loop multiple times in each frame without a
//single SDL call and then push to the renderer and render all the updated
//info once per frame
//
//im going to push the code before doing this to save the fixed scales
double radToDeg(double radians);

int main(int argc, char* argv[]){

	srand(time(0));
	//parse arguments
	Parser parser(argc, argv);

        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;

        SDL_Init(SDL_INIT_EVERYTHING);

        SDL_CreateWindowAndRenderer(RES[0], RES[1], 0, &window, &renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

			//strcmp returns 0 if true
	if(!parser.flags.mouseDisabled && SDL_SetRelativeMouseMode(SDL_TRUE) < 0){

		std::cerr << "ERR: Could not capture mouse, use --nm to disable mouse support" << std::endl;
		std::cerr << "error code: " << SDL_GetError() << std::endl;
		return 1;
	}

        textRenderer* txtRenderer = new textRenderer(renderer, "../src/Hack-Regular.ttf", 20);

	const std::chrono::duration<double> frameMaxExecTime(1.0 / FRAMECAP);
	auto lastFrameTime = std::chrono::high_resolution_clock::now();
	double deltaTime;
	double deltaTimeMovement;

	//fps counter
	int frameCount = 0;
	double fps = 0;
	auto lastFpsUpdateTime = std::chrono::high_resolution_clock::now();

	std::vector<Camera> cameras;

	if(!parser.flags.forwardTraces){

		cameras.push_back({0, 0, -1e9, DEFAULTFOV * 2, 0, 0, 0});
	}
	else{

		cameras.push_back({});
	}

	int cidx = 0;
	int decoyIdx = 1;

	starsBackground bg;
	bg.generateStars();

	Hud hud;

	hud.stats.push_back({"cam X", &cameras[cidx].posX});
	hud.stats.push_back({"cam Y", &cameras[cidx].posY});
	hud.stats.push_back({"cam Z", &cameras[cidx].posZ});
	hud.stats.push_back({"fov", &cameras[cidx].fov});
	hud.stats.push_back({"tiltX", &cameras[cidx].tiltX});
	hud.stats.push_back({"tiltY", &cameras[cidx].tiltY});
	hud.stats.push_back({"fps", &fps});

	std::vector<Menu*> menus;
	std::vector<std::pair<std::string, double*>> vars;

	std::unordered_map<std::string, SpaceObject> astros;

	//im using Horizon System to manually insert the information
	//Time Specification: Start=2026-05-20 TDB , Stop=2026-06-19, Step=1 (days)
	//                               |name       |x                        |y                       |z                      |mass     |radius   |velocity                                                                       |rotationAngVel 
	astros.try_emplace("Sun",        "Sun",       0.00,                     0.00,                    0.00,                   1.989e30, 6.963e8,  vector3D{0.0, 0.0, 0.0}, 2.865e-6);
	astros.try_emplace("Mercury",    "Mercury",   2.145434416716681E+09,    4.593395729676659E+10,   3.557100426719822E+09,  3.301e23, 2.440e6,  vector3D{-5.843295237937669E+04, 4.048711691994271E+03, 5.690193399634622E+03}, 1.240e-6);
	astros.try_emplace("Venus",      "Venus",     -8.146172176991183E+10,   6.988637972774877E+10,   5.660433614479158E+09,  4.867e24, 6.052e6,  vector3D{-2.293501124201724E+04, -2.676065609340537E+04, 9.556759733306066E+02}, -2.992e-7);
	astros.try_emplace("Earth",      "Earth",     -7.875500891135609E+10,   -1.292607154626929E+11,  7.900696970544755E+06,  5.972e24, 6.371e6,  vector3D{2.496813969720202E+04, -1.560876315415955E+04, 1.480191825459443E-00}, 7.292e-5);
	astros.try_emplace("Mars",       "Mars",      2.062134692129982E+11,    3.819988844201361E+10,   -4.255964981759114E+09, 6.417e23, 3.390e6,  vector3D{-3.483059754828587E+03, 2.589529724627294E+04, 6.280865076797308E+02}, 7.088e-5);
	astros.try_emplace("Jupiter",    "Jupiter",   -3.977215066838948E+11,   6.792281214587531E+11,   6.076947975619853E+09,  1.898e27, 6.991e7,  vector3D{-1.143663847930005E+04, -5.995741422124383E+03, 2.808607691996452E+02}, 1.758e-4);
	astros.try_emplace("Saturn",     "Saturn",    1.407927996180217E+12,    1.539996392108606E+11,   -5.872595060292597E+10, 5.683e26, 5.823e7,  vector3D{-1.589394387680512E+03, 9.579859441017856E+03, -1.038774658231327E+02}, 1.638e-4);
	astros.try_emplace("Uranus",     "Uranus",    1.406265257214856E+12,    2.549899181965824E+12,   -8.764036768092990E+09, 8.681e25, 2.536e7,  vector3D{-6.024687174531799E+03, 2.968845814764818E+03, 8.943727988440786E+01}, -1.012e-4);
	astros.try_emplace("Neptune",    "Neptune",   4.466629254293965E+12,    1.432568203818374E+11,   -1.058809943827548E+11, 1.024e26, 2.462e7,  vector3D{-2.198959283463029E-04, 5.462561967464699E+03, -1.076599005970522E+02}, 1.080e-4);

	for(auto &astro : astros){

		vars.push_back(std::pair<std::string, double*>(astro.second.name, &astro.second.mass));
	}

	MainMenu mmenu(vars);
	menus.push_back(&mmenu);


	if(parser.flags.isCamDecoy){

		//we set the first cam as decoy on the origin
		//and move the cidx to the next we are inserting
		//
		//this will be handled better in a future not handing out
		//hardcoded lvalues as indexes
		cameras.push_back({0, 0, -8e5, DEFAULTFOV, 0, 0, 0});
		decoyIdx = cidx + 1;
		//cidx++;
		astros.try_emplace("CAMERA", "CAMERA", -cameras[decoyIdx].posX, -cameras[decoyIdx].posY, cameras[decoyIdx].posZ, 0, 10e3, vector3D{0.0, 0.0, 0.0}, 0);
	}


	SDL_Event events;
	const Uint8* kbstate = SDL_GetKeyboardState(NULL);

	bool mainLoop = true;
	while(mainLoop){

		//get current time for delta time and fps management
		auto beginFrameTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = beginFrameTime - lastFrameTime;
	
		//calculate delta time
		deltaTime = duration.count();
		deltaTimeMovement = deltaTime * 1000;
	
		//fps counter
		frameCount++;
		std::chrono::duration<double> fpsElapsed = beginFrameTime - lastFpsUpdateTime;

		if(fpsElapsed.count() >= 1.0f){

			fps = (float)frameCount / fpsElapsed.count();

			frameCount = 0;
			lastFpsUpdateTime = beginFrameTime;
		}

		//clear screen
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);


		if(kbstate[SDL_SCANCODE_ESCAPE]){

			mainLoop = false;
		}

		//space objects CALC loop
		for(int i = 0; i < TIMEMODIFIER; i++){
			
			for(auto &astro : astros){

				SpaceObject* so = &astro.second;

				so->rotation = std::fmod(so->rotation + (so->angVelocityRotation * deltaTime), 2 * PI);

				if(so->name != "CAMERA"){

					so->rotateZ(so->angVelocityRotation * deltaTime);

					so->calculateForces(astros);
					so->updateVelocity(deltaTime);
					so->updatePosition(deltaTime);
				}
			}
		}
	
		//space objects RENDER loop
		for(auto &astro : astros){

			SpaceObject* so = &astro.second;

			int oldObjectRes = so->objectRes;

			if(parser.flags.isCamDecoy){

				so->calcObjRes(cameras[decoyIdx]);
			}
			else{

				so->calcObjRes(cameras[cidx]);
			}

			if(so->objectRes != oldObjectRes){

				so->plot();
			}

			if(parser.flags.lock && astros.find(parser.flags.lockname) != astros.end()){

				cameras[cidx].lockToSO(astros.at(parser.flags.lockname));
			}

			//so->orbitY(0.01 * deltaTime);

			//bruh i need to set this as a function inside spaceObject or something not in here lmao
			//for some reason when using decoy i see no traces ill have to look into it
			if(parser.flags.traces){

				so->handleTrace(cameras[cidx], parser.flags.forwardTraces, deltaTime);
			}

			if(parser.flags.isCamDecoy){

				so->project(cameras[cidx], cameras[decoyIdx]);
			}
			else{

				so->project(cameras[cidx]);
			}

			so->render(renderer, txtRenderer, parser.flags.renderLabels, cameras[cidx], parser.flags.forwardTraces);

			if(parser.flags.isCamDecoy){

				so->projectRenderPts(renderer, cameras[cidx]);
			}
		}

		static bool lastMouseState = !parser.flags.mouseDisabled;

		handleKeyboardInput(&events, kbstate, cameras[cidx], deltaTimeMovement, parser.flags.mouseDisabled, menus);
		while(SDL_PollEvent(&events)){

			handleMouseInput(&events, cameras[cidx], deltaTimeMovement, parser.flags.mouseDisabled, menus);
		}

		Uint32 flags = SDL_GetWindowFlags(window);

		if(!parser.flags.mouseDisabled && lastMouseState == false){

			if(SDL_SetRelativeMouseMode(SDL_TRUE) < 0){

				std::cerr << "ERR: Could not capture mouse, use --nm to disable mouse support" << std::endl;
				std::cerr << "error code: " << SDL_GetError() << std::endl;
				return 1;
			}

			lastMouseState = true;

		}
		else if(parser.flags.mouseDisabled && lastMouseState == true){

			if(SDL_SetRelativeMouseMode(SDL_FALSE) < 0){

				std::cerr << "ERR: Could not disable mouse support" << std::endl;
				std::cerr << "error code: " << SDL_GetError() << std::endl;
				return 1;
			}

			lastMouseState = false;
		}

		bg.render(renderer, cameras[cidx]);

		if(mmenu.show){

			mmenu.render(renderer, txtRenderer);
		}

		hud.renderCrossHair(renderer);
		hud.renderStats(txtRenderer);

		SDL_RenderPresent(renderer);

		//last calculations for delta time
		lastFrameTime = beginFrameTime;
		
		auto timeFinished = std::chrono::high_resolution_clock::now();
		auto timeTaken = timeFinished - beginFrameTime;

		if(timeTaken < frameMaxExecTime){

			std::this_thread::sleep_for(frameMaxExecTime - timeTaken);
		}
	}


	return 0;
}

double radToDeg(double radians){

	return radians * 180 / PI;
}
