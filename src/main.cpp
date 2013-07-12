#include "scene/scene.h"
#include "raytracer/raytracer.h"
#include "raytracer/path_tracer.h"
#include "photonMapping/photonMap.h"
#include "parameters.h"
#include <opencv2/opencv.hpp>
#include "tinyxml/tinyxml.h"

Parameters para;
Raytracer raytracer;
Pathtracer pathtracer;
PhotonIntegrator photonIntegrator;

int main(int argc , char* argv[])
{
    para.load_parameters("src/parameters.para");
	if (!strcmp(argv[3] , "-r"))
	{
		raytracer.init(argv[1] , para);
		raytracer.render(argv[2]);
	}
	else if (!strcmp(argv[3] , "-p"))
	{
		pathtracer.init(argv[1] , para);
		pathtracer.render(argv[2]);
	}
	else if (!strcmp(argv[3] , "-pm"))
	{
		photonIntegrator.init(argv[1] , para);
		photonIntegrator.buildPhotonMap(photonIntegrator.scene);
		photonIntegrator.render(argv[2]);
	}
	else
	{
		printf("error!\n");
	}
	return 0;
}
