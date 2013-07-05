#ifndef RAY_H
#define RAY_H

#include "../math/vector.h"

class Ray
{
public:
	Vector3 origin , dir;

	Ray() {}
	Ray(Vector3 origin , Vector3 dir)
		: origin(origin) , dir(dir) {this->dir.normalize();}

	Ray(const Ray& ray) 
	{
		*this = ray;
	}
};

#endif