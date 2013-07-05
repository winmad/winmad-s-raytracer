#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "../math/vector.h"
#include "material.h"
#include "geometry.h"
#include "AABB.h"

class Triangle : public Geometry
{
public:
	Vector3 p0 , p1 , p2;
	Material material;

	void set_box()
	{
		box = AABB(std::min(p0.x , std::min(p1.x , p2.x)) ,  
			std::max(p0.x , std::max(p1.x , p2.x)) ,
			std::min(p0.y , std::min(p1.y , p2.y)) ,
			std::max(p0.y , std::max(p1.y , p2.y)) ,
			std::min(p0.z , std::min(p1.z , p2.z)) ,
			std::max(p0.z , std::max(p1.z , p2.z)));
	}

	Triangle() {}
	
	Triangle(Vector3 _p0 , Vector3 _p1 , Vector3 _p2)
		: p0(_p0) , p1(_p1) , p2(_p2)
	{
		set_box();
	}

	~Triangle() {}

	Material& get_material();

	Color3 get_diffuse_color(Vector3 pos);

	bool hit(const Ray& ray , Real& t , 
		Vector3& p , Vector3& n , int& inside);
};

#endif