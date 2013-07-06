#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "../math/color.h"
#include "../math/vector.h"
#include "../scene/scene.h"
#include <opencv2/opencv.hpp>

class Raytracer
{
public:
	static const int max_tracing_depth = 2;
    
    static const int samples_per_pixel = 1;

	int width , height;
	ViewPort view_port;

	Scene scene;

	Raytracer() {}

	void init();

	Color3 raytracing(const Ray& ray , int dep);

	void render(char *filename);
};

#endif
