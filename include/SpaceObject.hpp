#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>

#include "Renderer.hpp"

//required forward declarations
class SpaceObject;

//helper functions

const double PI = M_PI;
const double HALF_PI = M_PI / 2;
double map(double value, double f1, double l1, double f2, double l2);

class Camera;

//position
struct point3D{

	double x, y, z;
	double rX, rY, rZ; //rotated X Y Z
	double screenX, screenY;
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

	Camera(double _posX = 0, double _posY = 0, double _posZ = -200, double _fov = DEFAULTFOV, double _tiltX = 0, double _tiltY = 0, double _tiltZ = 0);

	void moveX(double speed, float deltaT);
	void moveY(double speed, float deltaT);
	void moveZ(double speed, float deltaT);

	void worldCameraTransform(double &relX, double &relY, double &relZ);
};


//physical objects
class SpaceObject{

	public:

	std::string name;

	double posX, posY, posZ;

	std::vector<point3D> points;
	std::vector<point3D> trace;

	double velocity;
	double angVelocityOrbit;
	double angVelocityRotation;

	double mass;
	double radius;
	double F;

	double rotation;

	int objectRes;

	SpaceObject(std::string _name, double _x, double _y, double _z, double _mass, double _radius, double angVelocityOrbit, double angVelocityRotation);

	void calcObjRes(Camera &_cam);
	void plot();

	//display
	void render(SDL_Renderer* renderer, textRenderer* _txtRenderer, bool renderLabels, Camera &_cam);

	void project(Camera &_cam);
	void project(Camera &_cam, Camera &_decoy);

	//motion
	void rotateX(double theta);
	void rotateY(double theta);
	void rotateZ(double theta);

	void orbitX(double theta);
	void orbitY(double theta);
	void orbitZ(double theta);
	
	void projectRenderPts(SDL_Renderer* renderer, Camera &_cam);
};
