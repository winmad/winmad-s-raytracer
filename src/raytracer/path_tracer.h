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
	int max_tracing_depth;
    
    int samples_per_pixel;
    
    int samples_of_light;
    
    int samples_of_hemisphere;
    
    int phong_power_index;

	int width , height;
	ViewPort view_port;

	Scene scene;

	Pathtracer() {}

	void init(char *filename , Parameters& para);

	Color3 raytracing(const Ray& ray , int dep);

	void render(char *filename);
};

#endif
