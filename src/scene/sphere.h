#ifndef SPHERE_H
#define SPHERE_H

#include "../math/vector.h"
#include "material.h"
#include "geometry.h"
#include "AABB.h"

class Sphere : public Geometry
{
public:
	Vector3 center;
	Real radius;
	Material material;

	void set_box()
	{
		box = AABB(center.x - radius , center.x + radius ,
			center.y - radius , center.y + radius ,
			center.z - radius , center.z + radius);
	}

	Sphere() {}

	Sphere(Vector3 c , Real r)
		: center(c) , radius(r)
	{
		set_box();
	}

	Sphere(const Sphere& s)
	{
		*this = s;
	}
	
	~Sphere() {}

	Material& get_material();

	Color3 get_diffuse_color(Vector3 pos);

	bool hit(const Ray& ray , Real& t , 
		Vector3& p , Vector3& n , int& inside);
};

#endif