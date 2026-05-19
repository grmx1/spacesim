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
	
	/*
	//rotation-orbit ratio accurate
	astros.try_emplace("Sun", 0.00, 0.00, 0.00, 1.00, 50.00, 0.00, 28.7e-3);
	astros.try_emplace("Mercury", 78.00, 0.00, 0.00, 1.00, 15.00, 8.26e-3, 12.4e-3);
	astros.try_emplace("Venus", 144.00, 0.00, 0.00, 1.00, 20.00, 3.23e-3, -2.99e-3);
	astros.try_emplace("Earth", 200.00, 0.00, 0.00, 1.00, 20.00, 1.99e-3, 727.2e-3);
	astros.try_emplace("Mars", 304.00, 0.00, 0.00, 1.00, 16.00, 1.06e-3, 708.8e-3);
	astros.try_emplace("Jupiter", 1200.00, 0.00, 0.00, 1.00, 40.00, 1.67e-3, 1760.0e-3);
	astros.try_emplace("Saturn", 2180.00, 0.00, 0.00, 1.00, 35.00, 9.29e-3, 1630.0e-3);
	astros.try_emplace("Uranus", 3840.00, 0.00, 0.00, 1.00, 28.00, 2.37e-3, 1010.0e-3);
	astros.try_emplace("Neptune", 6000.00, 0.00, 0.00, 1.00, 28.00, 1.21e-3, 1080.0e-3);
	*/

	//i want to have rotation-orbit-distance-size scaled here at some point just to see how it looks
	astros.try_emplace("Sun", "Sun", 0.00, 0.00, 0.00, 1.00, 50.00, 0.00, 28.7e-3);
	//sun rotation looks weird now because of the constant recalculations (idk why only on the sun)
	//i will try to de-couple the rotation angle from the points in the vector so it stays consistent
	astros.try_emplace("Mercury", "Mercury", 78.00, 0.00, 0.00, 1.00, 15.00, 8.26e-3, 12.4e-3);
	astros.try_emplace("Venus", "Venus", 144.00, 0.00, 0.00, 1.00, 20.00, 3.23e-3, -2.99e-3);
	astros.try_emplace("Earth", "Earth", 200.00, 0.00, 0.00, 1.00, 20.00, 1.99e-3, 727.2e-3);
	astros.try_emplace("Mars", "Mars", 304.00, 0.00, 0.00, 1.00, 16.00, 1.06e-3, 708.8e-3);
	astros.try_emplace("Jupiter", "Jupiter", 1200.00, 0.00, 0.00, 1.00, 40.00, 1.67e-3, 1760.0e-3);
	astros.try_emplace("Saturn", "Saturn", 2180.00, 0.00, 0.00, 1.00, 35.00, 9.29e-3, 1630.0e-3);
	astros.try_emplace("Uranus", "Uranus", 3840.00, 0.00, 0.00, 1.00, 28.00, 2.37e-3, 1010.0e-3);
	astros.try_emplace("Neptune", "Neptune", 6000.00, 0.00, 0.00, 1.00, 28.00, 1.21e-3, 1080.0e-3);

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
		astros.try_emplace("CAMERA", "CAMERA", -cameras[0].posX - 5, -cameras[0].posY, cameras[0].posZ, 0, 10, 0, 0);
	}


	SDL_Event events;
	const Uint8* kbstate = SDL_GetKeyboardState(NULL);

	bool mainLoop = true;
	while(mainLoop){

		//get current time for delta time and fps management
		auto beginFrameTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> duration = beginFrameTime - lastFrameTime;
	
		//calculate delta time
		deltaTime = duration.count() * 60;
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
			so->orbitY(so->angVelocityOrbit * deltaTime);
			so->trace.push_back({so->posX, so->posY, so->posZ});

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
