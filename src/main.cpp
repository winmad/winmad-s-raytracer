#include "scene/scene.h"
#include "raytracer/raytracer.h"
#include "raytracer/path_tracer.h"
#include <opencv2/opencv.hpp>
#include "tinyxml/tinyxml.h"

Raytracer raytracer;
Pathtracer pathtracer;

int main(int argc , char* argv[])
{
	//raytracer.init(argv[1]);
	//raytracer.render(argv[2]);
	pathtracer.init(argv[1]);
	pathtracer.render(argv[2]);
	return 0;
}
