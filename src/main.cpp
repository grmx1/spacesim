#include <iostream>
#include <vector>
#include <numbers>
#include <string>
#include <map>
#include <chrono>
#include <thread> //for fps rating not multithreading

#include "Renderer.hpp"
#include "SpaceObject.hpp"
#include "Input.hpp"
#include "defines.hpp"

double radToDeg(double radians);

int main(int argc, char* argv[]){

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

	//fps counter
	int frameCount = 0;
	double fps = 0;
	auto lastFpsUpdateTime = std::chrono::high_resolution_clock::now();

	std::vector<Camera> cameras;
	cameras.push_back({});

	int cidx = 0;

	Hud hud;

	hud.stats.push_back({"cam X", &cameras[cidx].posX});
	hud.stats.push_back({"cam Y", &cameras[cidx].posY});
	hud.stats.push_back({"cam Z", &cameras[cidx].posZ});
	hud.stats.push_back({"fov", &cameras[cidx].fov});
	hud.stats.push_back({"tiltX", &cameras[cidx].tiltX});
	hud.stats.push_back({"tiltY", &cameras[cidx].tiltY});
	hud.stats.push_back({"fps", &fps});

	std::map<std::string, SpaceObject> astros;
	
	//im using Horizon System to manually insert the information
	//Time Specification: Start=2026-05-20 TDB , Stop=2026-06-19, Step=1 (days)
	//                               |name       |x                        |y                       |z                      |mass     |radius   |velocity                                                                       |rotationAngVel 
	astros.try_emplace("Sun",        "Sun",       0.00,                     0.00,                    0.00,                   1.989e30, 6.963e5,  vector3D{0.0, 0.0, 0.0}, 28.7e-3);
	astros.try_emplace("Mercury",    "Mercury",   2.145434416716681E+06,    4.593395729676659E+07,   3.557100426719822E+06,  3.301e23, 2.440e3,  vector3D{-5.843295237937669E+01, 4.048711691994271E+00, 5.690193399634622E+00}, 12.4e-3);
	astros.try_emplace("Venus",      "Venus",     -8.146172176991183E+07,   6.988637972774877E+07,   5.660433614479158E+06,  4.867e24, 6.052e3,  vector3D{-2.293501124201724E+01, -2.676065609340537E+01, 9.556759733306066E-01}, -2.99e-3);
	astros.try_emplace("Earth",      "Earth",     -7.875500891135609E+07,   -1.292607154626929E+08,  7.900696970544755E+03,  5.972e24, 6.371e3,  vector3D{2.496813969720202E+01, -1.560876315415955E+01, 1.480191825459443E-03}, 727.2e-3);
	astros.try_emplace("Mars",       "Mars",      2.062134692129982E+08,    3.819988844201361E+07,   -4.255964981759114E+06, 6.417e23, 3.390e3,  vector3D{-3.483059754828587E+00, 2.589529724627294E+01, 6.280865076797308E-01}, 708.8e-3);
	astros.try_emplace("Jupiter",    "Jupiter",   -3.977215066838948E+08,   6.792281214587531E+08,   6.076947975619853E+06,  1.898e27, 6.991e4,  vector3D{-1.143663847930005E+01, -5.995741422124383E+00, 2.808607691996452E-01}, 1760.0e-3);
	astros.try_emplace("Saturn",     "Saturn",    1.407927996180217E+09,    1.539996392108606E+08,   -5.872595060292597E+07, 5.683e26, 5.823e4,  vector3D{-1.589394387680512E+00, 9.579859441017856E+00, -1.038774658231327E-01}, 1630.0e-3);
	astros.try_emplace("Uranus",     "Uranus",    1.406265257214856E+09,    2.549899181965824E+09,   -8.764036768092990E+06, 8.681e25, 2.536e4,  vector3D{-6.024687174531799E+00, 2.968845814764818E+00, 8.943727988440786E-02}, 1010.0e-3);
	astros.try_emplace("Neptune",    "Neptune",   4.466629254293965E+09,    1.432568203818374E+08,   -1.058809943827548E+08, 1.024e26, 2.462e4,  vector3D{-2.198959283463029E-01, 5.462561967464699E+00, -1.076599005970522E-01}, 1080.0e-3);
	
	//gigantic thing for testng
	//astros.try_emplace("AAA", "AAA", 1000.00, 0.00, 0.00, 1.00, 500.00, 10.06e-3, 708.8e-3);

	if(parser.flags.isCamDecoy){

		//we set the first cam as decoy on the origin
		//and move the cidx to the next we are inserting
		//
		//this will be handled better in a future not handing out
		//hardcoded lvalues as indexes
		cameras.push_back({});
		cidx++;
		astros.try_emplace("CAMERA", "CAMERA", -cameras[0].posX - 5, -cameras[0].posY, cameras[0].posZ, 0, 10, vector3D{0.0, 0.0, 0.0}, 0);
	}


	SDL_Event events;
	const Uint8* kbstate = SDL_GetKeyboardState(NULL);

	bool mainLoop = true;
	while(mainLoop){

		//get current time for delta time and fps management
		auto beginFrameTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = beginFrameTime - lastFrameTime;
	
		//calculate delta time
		deltaTime = duration.count() * 60 * 1000;
		if(deltaTime > 0.1f * 60) deltaTime = 0.1f * 60; //10 fps cap so lag doesnt send camera flying
	
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
	
		handleInput(&events, kbstate, cameras[cidx], deltaTime, parser.flags.mouseDisabled);
	
		//space objects loop
		for(auto &astro : astros){

			SpaceObject* so = &astro.second;

			int oldObjectRes = so->objectRes;

			if(parser.flags.isCamDecoy){

				so->calcObjRes(cameras[0]);
			}
			else{

				so->calcObjRes(cameras[cidx]);
			}

			if(so->objectRes != oldObjectRes){

				so->plot();
			}

			so->rotation = std::fmod(so->rotation + (so->angVelocityRotation * deltaTime), 2 * PI);

			so->rotateY(so->angVelocityRotation * deltaTime);
			

		
			so->calculateForces(astros);
			so->updateVelocity(deltaTime);
			so->updatePosition(deltaTime);
			//so->orbitY(0.01 * deltaTime);
			

			//something wrong with the size check or the cycles check because when comparing it only shows like 2 traces otherwise it works fine

			if(so->name != "Camera"){

				if(so->renderCycles >= TRACEUPDATERATE){

					if(so->trace.size() > MAXTRACESIZE){

						so->trace.pop_front();
					}
					so->trace.push_back({so->posX, so->posY, so->posZ});
					so->renderCycles = 0;
				}
				else{

					so->renderCycles++;
				}

			}

			if(parser.flags.isCamDecoy){

				so->project(cameras[cidx], cameras[0]);
			}
			else{

				so->project(cameras[cidx]);
			}

			so->render(renderer, txtRenderer, parser.flags.renderLabels, cameras[cidx]);

			if(parser.flags.isCamDecoy){

				so->projectRenderPts(renderer, cameras[cidx]);
			}
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
