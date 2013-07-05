#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "AABB.h"
#include "material.h"
#include "../math/vector.h"

class Geometry
{
public:
	AABB box;

	Geometry() {}
	virtual ~Geometry();
	virtual void set_box();
	virtual Material& get_material();
	virtual Color3 get_diffuse_color(Vector3 pos);
	virtual bool hit(const Ray& ray , Real& t , Vector3& p , 
		Vector3& n , int& inside);
};

#endif