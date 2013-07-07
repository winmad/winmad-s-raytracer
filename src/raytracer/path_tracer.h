#ifndef PATH_TRACER_H
#define PATH_TRACER_H

#include "../math/color.h"
#include "../math/vector.h"
#include "../scene/scene.h"
#include "../parameters.h"
#include <opencv2/opencv.hpp>

class Pathtracer
{
public:
	static const int max_tracing_depth = MAX_TRACING_DEPTH;
    
    static const int samples_per_pixel = SAMPLES_PER_PIXEL;

	int width , height;
	ViewPort view_port;

	Scene scene;

	Pathtracer() {}

	void init();

	Color3 raytracing(const Ray& ray , int dep);

	void render(char *filename);
};

#endif
