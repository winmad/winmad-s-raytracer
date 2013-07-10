#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "../math/color.h"
#include "../math/vector.h"
#include "../scene/scene.h"
#include "../parameters.h"
#include <opencv2/opencv.hpp>

class Raytracer
{
public:
	int max_tracing_depth;

	int width , height;
	ViewPort view_port;

	Scene scene;

	Raytracer() {}

	void init(char *filename , Parameters& para);

	Color3 raytracing(const Ray& ray , int dep);

	void render(char *filename);
};

#endif
