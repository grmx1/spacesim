#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <array>
#include <map>
#include <cmath>
#include <list>

#include "Renderer.hpp"

//required forward declarations
class SpaceObject;

//helper functions

const double PI = M_PI;
const double HALF_PI = M_PI / 2;
const double G = 6.6743e-11;
double map(double value, double f1, double l1, double f2, double l2);

class Camera;

struct vector3D{

	double x, y, z;
};

//position
struct point3D{

	int screenX, screenY;
	double x, y, z;
	double rX, rY, rZ; //rotated X Y Z
	bool onScreen;

	void project(Camera &_cam, double _posX, double _posY, double _posZ);

	void rotateX(double cosT, double sinT);
	void rotateY(double cosT, double sinT);
	void rotateZ(double cosT, double sinT);

};

class Camera{

	public:

	double fov;

	double posX, posY, posZ;
	double tiltX, tiltY, tiltZ;

	Camera(double _posX = -8e11, double _posY = -5e11, double _posZ = -3e11, double _fov = DEFAULTFOV, double _tiltX = -0.5, double _tiltY = 1.3, double _tiltZ = 0);

	void lockToSO(SpaceObject &so);
	void moveX(double speed, float deltaT);
	void moveY(double speed, float deltaT);
	void moveZ(double speed, float deltaT);

	void worldCameraTransform(double &relX, double &relY, double &relZ);
};

class starsBackground{

	public:

	void generateStars();
	void render(SDL_Renderer* renderer, const Camera &_cam);

	private:

	SDL_Point stars[1000];
};

//physical objects
class SpaceObject{

	public:

	std::string name;

	double posX, posY, posZ;

	std::vector<point3D> points;
	std::array<point3D, MAXTRACESIZE> trace;
	std::array<SDL_Point, MAXTRACESIZE> traceSDL;
	int traceHead;
	int traceFull;

	struct {

		double x, y, z;
	} velocity;
	
	struct {

		double x, y, z;
	} acceleration;

	double angVelocityRotation;

	double mass;
	double radius;
	double F;

	double rotation;

	int renderCycles;
	int objectRes;

	SpaceObject(std::string _name, double _x, double _y, double _z, double _mass, double _radius, vector3D initVelocity, double _angVelocityRotation);

	void calcObjRes(Camera &_cam);
	void plot();

	void handleTrace(Camera &_cam, bool forward, double deltaT);

	//display
	void render(SDL_Renderer* renderer, textRenderer* _txtRenderer, bool renderLabels, Camera &_cam, bool forward = false);

	void project(Camera &_cam);
	void project(Camera &_cam, Camera &_decoy);

	//motion
	void rotateX(double theta);
	void rotateY(double theta);
	void rotateZ(double theta);

	void calculateForces(const std::map<std::string, SpaceObject> &astros);

	void updateVelocity(double deltaT);
	void updatePosition(double deltaT);

	void orbitX(double theta);
	void orbitY(double theta);
	void orbitZ(double theta);
	
	void projectRenderPts(SDL_Renderer* renderer, Camera &_cam);
};
