#include "scene/scene.h"
#include "raytracer/raytracer.h"
#include <opencv2/opencv.hpp>
#include "tinyxml/tinyxml.h"

Raytracer raytracer;

int main()
{
	raytracer.init();
	raytracer.render("result_10:40.jpg");
	return 0;
}
