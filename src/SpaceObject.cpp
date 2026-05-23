#include <cmath>
#include <iostream>

#include "SpaceObject.hpp"

double map(double value, double f1, double l1, double f2, double l2){

	return f2 + (l2 - f2) * ((value - f1) / (l1 - f1));
}


void point3D::project(Camera &_cam, double _posX, double _posY, double _posZ){

	double relPosX = x + _posX - _cam.posX;
	double relPosY = y + _posY - _cam.posY;
	double relPosZ = z + _posZ - _cam.posZ; 
	
	_cam.worldCameraTransform(relPosX, relPosY, relPosZ);

	if(relPosZ < 0.1){

		relPosZ = 0.1;
		onScreen = false;
	}
	else{
		onScreen = true;
	}

	double zConversion = _cam.fov / relPosZ;

	screenX = relPosX * zConversion + (RES[0] / 2);
	screenY = relPosY * zConversion + (RES[1] / 2);
}


void point3D::rotateX(double cosT, double sinT){

	double rotY = y;

	y = rotY * cosT - z * sinT;
	z = rotY * sinT + z * cosT;

}

void point3D::rotateY(double cosT, double sinT){

	double rotX = x;

	x = rotX * cosT + z * sinT;
	z = z * cosT - rotX * sinT;
}

void point3D::rotateZ(double cosT, double sinT){

	double rotX = x;
	double rotY = y;

	x = rotX * cosT - rotY * sinT;
	y = rotX * sinT + rotY * cosT;
}

void starsBackground::generateStars(){
	
	srand(time(0));

	for(int i = 0; i < sizeof(stars) / sizeof(SDL_Point); i++){

		stars[i] = SDL_Point{rand() % RES[0], rand() % RES[1]};
	}
}

void starsBackground::render(SDL_Renderer* renderer, const Camera &_cam){

	double tiltSens = 700;

	SDL_Point container;

	for(auto star : stars){

		//make the stars move with cam tilt and wrap around the screen coordinates
		SDL_RenderDrawPoint(renderer, (static_cast<int>(star.x - _cam.tiltY * tiltSens) % RES[0] + RES[0]) % RES[0], (static_cast<int>(star.y + _cam.tiltX * tiltSens) % RES[1] + RES[1]) % RES[1]);
	}
}

void SpaceObject::calculateForces(const std::map<std::string, SpaceObject> &astros){

	double Fx = 0;
	double Fy = 0;
	double Fz = 0;

	for(const auto astro : astros){

		const SpaceObject &so = astro.second;


		if(so.name != name){

			double xDist = so.posX - posX;
			double yDist = so.posY - posY;
			double zDist = so.posZ - posZ;

			double totalDist = std::sqrt(std::pow(xDist, 2) + std::pow(yDist, 2) + std::pow(zDist, 2));

			double unitVx = xDist / totalDist;
			double unitVy = yDist / totalDist;
			double unitVz = zDist / totalDist;

			double halfGrav = (GKM* so.mass) / std::pow(totalDist, 2);

			double localFx = halfGrav * unitVx;
			double localFy = halfGrav * unitVy;
			double localFz = halfGrav * unitVz;

			//double totalAcc = std::sqrt(std::pow(localFx, 2) + std::pow(localFy, 2) + std::pow(localFz, 2));

			Fx += localFx;
			Fy += localFy;
			Fz += localFz;
		}
	}

	acceleration.x = Fx;
	acceleration.y = Fy;
	acceleration.z = Fz;
}

void SpaceObject::updateVelocity(double deltaT){
	
	velocity.x += acceleration.x * deltaT * 10000;
	velocity.y += acceleration.y * deltaT * 10000;
	velocity.z += acceleration.z * deltaT * 10000;
}

void SpaceObject::updatePosition(double deltaT){

	posX += velocity.x * deltaT * 10000;
	posY += velocity.y * deltaT * 10000;
	posZ += velocity.z * deltaT * 10000;
}

void SpaceObject::orbitX(double theta){

	double cosT = cos(theta);
	double sinT = sin(theta);

	double ogY = posY;

	posY = ogY * cosT - posZ * sinT;
	posZ = ogY * sinT + posZ * cosT;
}

void SpaceObject::orbitY(double theta){

	double cosT = cos(theta);
	double sinT = sin(theta);

	double ogX = posX;

	posX = ogX * cosT + posZ * sinT;
	posZ = posZ * cosT - ogX * sinT;
}

void SpaceObject::orbitZ(double theta){

	double cosT = cos(theta);
	double sinT = sin(theta);

	double ogX = posX;
	double ogY = posY;

	posX = ogX * cosT - ogY * sinT;
	posY = ogX * sinT + ogY * cosT;
}

void SpaceObject::rotateX(double theta){

	double cosT = cos(theta);
	double sinT = sin(theta);

	for(point3D &pt : points){

		pt.rotateX(cosT, sinT);
	}
}

void SpaceObject::rotateY(double theta){

	double cosT = cos(theta);
	double sinT = sin(theta);

	for(point3D &pt : points){

		pt.rotateY(cosT, sinT);
	}
}

void SpaceObject::rotateZ(double theta){

	double cosT = cos(theta);
	double sinT = sin(theta);

	for(point3D &pt : points){

		pt.rotateZ(cosT, sinT);
	}
}

Camera::Camera(double _posX, double _posY, double _posZ, double _fov, double _tiltX, double _tiltY, double _tiltZ){

	posX = _posX;
	posY = _posY;
	posZ = _posZ;
	fov = _fov;

	tiltX = _tiltX;
	tiltY = _tiltY;
	tiltZ = _tiltZ;
}

void Camera::lockToSO(SpaceObject &so){

	posX = so.posX + so.radius * 4;
	posY = so.posY;
	posZ = so.posZ;
}

void Camera::moveX(double speed, float deltaT){

	speed = speed * deltaT;
	posX += cos(tiltY) * speed;
	posZ -= sin(tiltY) * speed;
}

void Camera::moveY(double speed, float deltaT){

	speed = speed * deltaT;
	posY += speed;
}

void Camera::moveZ(double speed, float deltaT){

	speed = speed * deltaT;
	posX += cos(tiltX) * sin(tiltY) * speed;
	posY -= sin(tiltX) * speed; // negative due to inverted Y in SDL
	posZ += cos(tiltX) * cos(tiltY) * speed;
}

void Camera::worldCameraTransform(double &relX, double &relY, double &relZ){

	if(tiltY){

		double cosT = cos(-tiltY);
		double sinT = sin(-tiltY);

		double ogX = relX;
		
		relX = ogX * cosT + relZ * sinT;
		relZ = relZ * cosT - ogX * sinT;
	}

	if(tiltX){

		double cosT = cos(-tiltX);
		double sinT = sin(-tiltX);

		double ogY = relY;

		relY = ogY * cosT - relZ * sinT;
		relZ = ogY * sinT + relZ * cosT;
	}
}

void SpaceObject::calcObjRes(Camera &_cam){

	bool tiny = false;

	double dx = posX - _cam.posX;
	double dy = posY - _cam.posY;
	double dz = posZ - _cam.posZ;

	double newRes;

	//i need to have the range of values from MINRES to MAXRES
	//be mapped out accross a higher value of distances
	//
	//basically it reaches the lowest amount of detail way to fast
	double objCamDistance = std::sqrt((dx*dx) + (dy*dy) + (dz*dz));

	double screenR = radius * _cam.fov / (objCamDistance + 1.0);
	//double screenR = (radius * _cam.fov) / (objCamDistance + 1.0);

	if(screenR > TINYSIZE){

		newRes = screenR / LOD;
	}
	else{
		newRes = TINYRES;
		tiny = true;
	}
	//double newRes = screenR > 20 ? screenR / LOD : 5;

	
	if(!tiny && newRes > MAXRES){

		objectRes = MAXRES;
	}
	else if(!tiny && newRes < MINRES){

		objectRes = MINRES;
	}
	else{
		objectRes = newRes;
	}

}

//this one works decently
/*
void SpaceObject::calcObjRes(Camera &_cam){

	double dx = posX - _cam.posX;
	double dy = posY - _cam.posY;
	double dz = posZ - _cam.posZ;

	double objCamDistance = std::sqrt((dx*dx) + (dy*dy) + (dz*dz));
	
	double logZoom = std::log10(_cam.fov + 1.0) * 3;

	double newRes = LOD * radius * logZoom / std::pow(objCamDistance, RESDECAYMODIFIER);

	if(newRes > MAXRES){

		objectRes = MAXRES;
	}
	else if(newRes < MINRES){

		objectRes = MINRES;
	}
	else{
		objectRes = newRes;
	}
}
*/

void SpaceObject::project(Camera &_cam){

	//draw trace
	if(!trace.empty()){
	
		std::list<point3D>::iterator it;

		for(it = trace.begin(); it != trace.end(); it++){

			it->project(_cam, 0, 0, 0);
		}
	}

	for(int i = 0; i < points.size(); i++){

		points[i].project(_cam, posX, posY, posZ);
	}

	bool outsideScreen = true;
	for(int i = 0; i < points.size(); i++){

		//flag to not render pixels outside the view of the camera
		int nx = (i + 1) % points.size();
		int eq_nxl = (i + objectRes) % points.size();

		if((points[i].screenX > 0 && points[i].screenX < RES[0] && points[i].screenY > 0 && points[i].screenY < RES[1])
			|| (points[nx].screenX > 0 && points[nx].screenX < RES[0] && points[nx].screenY > 0 && points[nx].screenY < RES[1])
			|| (points[eq_nxl].screenX > 0 && points[eq_nxl].screenX < RES[0] && points[eq_nxl].screenY > 0 && points[eq_nxl].screenY < RES[1])){

			points[i].onScreen = true;
			outsideScreen = false;
			
		}
		else{
			points[i].onScreen = false;
		}

	}

	if(outsideScreen){

		objectRes = TINYRES;
		plot();
	}
}


//this is an overload of project meant to take in a decoy camera
//that will set the points to not be rendered as if that was the main camera
//while the actual projection happens on the real camera
void SpaceObject::project(Camera &_cam, Camera &_decoy){

	for(int i = 0; i < points.size(); i++){

		points[i].project(_decoy, posX, posY, posZ);
	}

	bool outsideScreen = true;
	for(int i = 0; i < points.size(); i++){

		//flag to not render pixels outside the view of the camera
		int nx = (i + 1) % points.size();
		int eq_nxl = (i + objectRes) % points.size();
		
		if((points[i].screenX > 0 && points[i].screenX < RES[0] && points[i].screenY > 0 && points[i].screenY < RES[1])
			|| (points[nx].screenX > 0 && points[nx].screenX < RES[0] && points[nx].screenY > 0 && points[nx].screenY < RES[1])
			|| (points[eq_nxl].screenX > 0 && points[eq_nxl].screenX < RES[0] && points[eq_nxl].screenY > 0 && points[eq_nxl].screenY < RES[1])){

			points[i].onScreen = true;
			outsideScreen = false;
		}
		else{
			points[i].onScreen = false;
		}

	}

	if(outsideScreen){

		objectRes = TINYRES;
		plot();
	}

	for(int i = 0; i < points.size(); i++){

		points[i].project(_cam, posX, posY, posZ);
	}
}

void SpaceObject::render(SDL_Renderer* renderer, textRenderer* _txtRenderer, bool renderLabels, Camera &_cam, bool forward){

	//draw trace
	if(!trace.empty()){
	
		std::list<point3D>::iterator it;

		for(it = trace.begin(); std::next(it) != trace.end(); it++){

			std::list<point3D>::iterator next = std::next(it);

			if(it->onScreen && next->onScreen){

				//SDL_RenderDrawPoint(renderer, trace[i].screenX, trace[i].screenY);
				SDL_RenderDrawLine(renderer, it->screenX, it->screenY, next->screenX, next->screenY);
			}
			if(forward){

				it->z += 7000000;
			}
		}
	}

	int pSize = points.size();
	
	for(int i = 0; i < objectRes; i++){
	
		for(int j = 0; j < objectRes; j++){

			//pt in current line
			int idx = j + (i * objectRes);

			if(points[idx].onScreen){
				
				//next pt wrapping around line
				int nx = (idx + 1) % objectRes + i * objectRes;

				//pt below in the grid
				int eq_nxl = (idx + objectRes) % pSize;

				//drawing triangles
				SDL_RenderDrawLine(renderer, points[idx].screenX, points[idx].screenY, points[nx].screenX, points[nx].screenY);
				SDL_RenderDrawLine(renderer, points[idx].screenX, points[idx].screenY, points[eq_nxl].screenX, points[eq_nxl].screenY);
				SDL_RenderDrawLine(renderer, points[eq_nxl].screenX, points[eq_nxl].screenY, points[nx].screenX, points[nx].screenY);
			}
		}
	}

	if(renderLabels){

		point3D center;
	
		center.x = posX;
		center.y = posY;
		center.z = posZ;

		center.project(_cam, 0, 0, 0);

		double relPosZ = center.z + posZ - _cam.posZ;
		double zConversion = _cam.fov / relPosZ;

		double screenR = relPosZ * zConversion * 0.5 - _cam.fov;
	
		int txtW;
		TTF_SizeText(_txtRenderer->font, name.c_str(), &txtW, nullptr);
		//SDL_RenderDrawLine(renderer, center.screenX, center.screenY, center.screenX, center.screenY + screenR + 25);
		//render names
		_txtRenderer->renderText(center.screenX - txtW / 2, center.screenY /*+ screenR*/ - 25, name, {255, 255, 255}); // TEMPORARELY REPLACED THIS WITH THE ONE BELOW
		//render rotation in radians
		//_txtRenderer->renderVariable(center.screenX - txtW / 2, center.screenY + screenR - 25, "", rotation, {255, 255, 255});
		//render resolution
		_txtRenderer->renderVariable(center.screenX - txtW / 2, center.screenY - 25, "", objectRes, {255, 255, 255});
	}
}

void SpaceObject::plot(){

	points.clear();

	for(int i = 0; i < objectRes + 1; i++){
	
		float lat = map(i, 0, objectRes, -HALF_PI, HALF_PI);

		float cosLat = cos(lat);
		float sinLat = sin(lat);

		for(int j = 0; j < objectRes; j++){

			float lon = map(j, 0, objectRes, -PI, PI);

			float cosLon = cos(lon);
			float sinLon = sin(lon);

			float x = radius * sinLon * cosLat;
			float y = radius * sinLon * sinLat;
			float z = radius * cosLon;

			points.push_back({x, y, z, 0, 0});
		}
	}

	rotateZ(rotation);
}

//this is meant for decoy and debugging
void SpaceObject::projectRenderPts(SDL_Renderer* renderer, Camera &_cam){

	for(point3D pt : points){

		pt.project(_cam, posX, posY, posZ);
		SDL_RenderDrawPoint(renderer, pt.screenX, pt.screenY);
	}
}

SpaceObject::SpaceObject(std::string _name, double _x, double _y, double _z, double _mass, double _radius, vector3D initVelocity, double _angVelocityRotation){

	name = _name;

	posX = _x;
	posY = _y;
	posZ = _z;

	mass = _mass;
	radius = _radius;

	rotation = 0;

	velocity.x = initVelocity.x;
	velocity.y = initVelocity.y;
	velocity.z = initVelocity.z;

	angVelocityRotation = _angVelocityRotation;

	renderCycles = 0;
	objectRes = 20;

	plot();
}


