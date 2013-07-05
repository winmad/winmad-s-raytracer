#include "geometry.h"

Geometry::~Geometry() {}

void Geometry::set_box()
{
}

Material& Geometry::get_material()
{
	Material material;
	return material;
}

Color3 Geometry::get_diffuse_color(Vector3 pos) 
{
	return Color3(0 , 0 , 0);
}

bool Geometry::hit(const Ray& ray , Real& t , 
	Vector3& p , Vector3& n , int& inside) 
{
	return 0;
}
