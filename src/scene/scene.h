#ifndef SCENE_H
#define SCENE_H

#include "light.h"
#include "ray.h"
#include "geometry.h"
#include "sphere.h"
#include "triangle.h"
#include "mesh.h"
#include "KDtree.h"
#include "../parameters.h"
#include <vector>

class ViewPort
{
public:
	Vector3 l , r;
	Vector3 delta;

	ViewPort() {}

	ViewPort(Real xmin , Real xmax , Real ymin , Real ymax , 
		Real zmin , Real zmax) 
		: l(xmin , ymin , zmin) , r(xmax , ymax , zmax) {}
};

class Scene
{
public:
	static const int point_light_num = POINT_LIGHT_NUM;
	
	std::vector<PointLight> lightlist;
    std::vector<Triangle> area_lightlist;
	std::vector<Geometry*> objlist;
	KDtree kdtree;
	Vector3 camera;
	ViewPort view_port;

	Scene() {}

	void add_geometry(Geometry* g);

	void add_light(PointLight l);

	void load_scene();

	void load_scene(char* filename);

	void init(char* filename);
};

#endif
