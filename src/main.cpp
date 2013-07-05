#include "scene/scene.h"
#include "raytracer/raytracer.h"
#include <opencv2/opencv.hpp>
#include "tinyxml/tinyxml.h"

Raytracer raytracer;

FILE* fp = fopen("debug.txt" , "w");

int main()
{
	raytracer.init();
	raytracer.render("result.jpg");
	return 0;
}