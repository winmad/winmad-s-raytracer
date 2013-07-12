#ifndef PHOTON_MAP_H
#define PHOTON_MAP_H

#include "../scene/scene.h"
#include "../parameters.h"
#include "photonKDtree.h"
#include <opencv2/opencv.hpp>

static std::vector<Photon> causticPhotons;
static std::vector<Photon> indirectPhotons;

class PhotonIntegrator
{
public:
    int nCausticPhotons , nIndirectPhotons;
    int knnPhotons;
    Real maxSqrDis;
    int maxPathLength , maxPhotonShot;

    int nCausticPaths , nIndirectPaths;
    PhotonKDtree *causticMap;
    PhotonKDtree *indirectMap;

	int max_tracing_depth;
	int samples_per_pixel;	
	
    int width , height;
    ViewPort view_port;

    Scene scene;

    PhotonIntegrator() {}
    
    void init(char *filename , Parameters& para);
    
    void buildPhotonMap(Scene& scene);

    Color3 estimate(PhotonKDtree *map , int nPaths , int knn ,
                    Geometry* g , const Vector3& p , 
                    const Vector3& n , const Vector3& wo , 
                    Real maxSqrDis);

    Color3 raytracing(const Ray& ray , int dep);
    
    void render(char *filename);
};

#endif
