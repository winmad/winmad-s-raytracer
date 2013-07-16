#include "photonMap.h"
#include "../raytracer/sampler.h"

static FILE *fp = fopen("debug_pm.txt" , "w");

void PhotonIntegrator::init(char *filename , Parameters& para)
{
    nCausticPhotons = 20000;
    
    nIndirectPhotons = 100000;

    knnPhotons = 50;

    maxSqrDis = 100;
    
    maxPathLength = 5;

    maxPhotonShot = 500000;

    causticMap = NULL;
    indirectMap = NULL;

    max_tracing_depth = para.MAX_TRACING_DEPTH;
    samples_per_pixel = para.SAMPLES_PER_PIXEL;

    scene.init(filename , para);
    view_port = scene.view_port;

	height = para.HEIGHT; width = para.WIDTH;

	view_port.delta.x = (view_port.r.x - view_port.l.x) / (Real)width;
	view_port.delta.y = (view_port.r.y - view_port.l.y) / (Real)height;
	view_port.delta.z = 0.0;
}

static Vector3 getReflectDir(const Ray& ray , const Vector3& n)
{
	Vector3 res = -n * (n ^ ray.dir) * 2.0 + ray.dir;
	res.normalize();
	return res;
}

static Vector3 getTransDir(const Ray& ray , const Vector3& n , 
	const Real& refractionIndex , int inside)
{
	Real refraction;
	if (!inside) 
		refraction = refractionIndex;
	else
		refraction = 1.0 / refractionIndex;
	Vector3 N = n * (inside == 0 ? 1.0 : -1.0);
	Real cosI = -(N ^ ray.dir);
	Real cosT = 1.0 - SQR(refraction) * (1.0 - SQR(cosI));
	Vector3 res;
	if (cmp(cosT) > 0)
	{
		res = ray.dir * refraction + N * (refraction * cosI - sqrt(cosT));
		res.normalize();
		return res;
	}
	else
	{
		return Vector3(inf , inf , inf);
	}
}

static Color3 calc_brdf(const Vector3& lightDir , 
                     const Vector3& visionDir , Geometry* g , const Vector3& n)
{
    Vector3 reflectDir = n * (lightDir ^ n) * 2.0 - lightDir;
    reflectDir.normalize();
    Real alpha = reflectDir ^ visionDir;
    Color3 res = g->get_material().diffuse / PI;
    res = res + g->get_material().specular *
        ((10 + 2) / 2 / PI * 
        pow(clamp_val(alpha , 0.0 , 1.0) , 10));
    return res;
}

static Color3 calc_btdf(const Vector3& lightDir ,
                     const Vector3& visionDir , Geometry* g , const Vector3& n)
{
    Vector3 otherHemisphereDir = n * (lightDir ^ (-n)) * 2.0 + lightDir;
    otherHemisphereDir.normalize();
    return calc_brdf(otherHemisphereDir , visionDir , g , n);
}

static void print_KD_tree(PhotonKDtreeNode *tr)
{
    if (tr == NULL)
        return;
    if (tr->photons.size() == 0)
        return;

    fprintf(fp , "-----------------------\n");
    fprintf(fp , "axis = %d, pos = (%.3lf,%.3lf,%.3lf)\n" , tr->axis ,
            tr->photons[0].p.x , tr->photons[0].p.y , tr->photons[0].p.z);
    print_KD_tree(tr->left);
    print_KD_tree(tr->right);
}

void PhotonIntegrator::buildPhotonMap(Scene& scene)
{
    if (scene.lightlist.size() <= 0)
        return;

    causticPhotons.clear();
    indirectPhotons.clear();
    
    bool causticDone = 0 , indirectDone = 0;
    int nShot = 0;

    bool specularPath;
    int nIntersections;
    Real t;
    Vector3 p , n;
    int inside;
    
    while (!causticDone || !indirectDone)
    {
        nShot++;

        /* generate initial photon ray */
        int k = rand() % (int)scene.lightlist.size();
        Color3 alpha = scene.lightlist[k].color;
        
        Vector3 dir = uniform_sample_dir_on_sphere();
        Ray photonRay = Ray(scene.lightlist[k].pos , dir);

        alpha = alpha * (4.0 * PI) * (scene.lightlist.size());

        if (cmp(alpha.r + alpha.g + alpha.b) > 0)
        {
            specularPath = 1;
            nIntersections = 0;
            Geometry *g = scene.intersect(photonRay , t , p , n , inside);
            while (g != NULL)
            {
                nIntersections++;
                bool hasNonSpecular = (cmp(g->get_material().shininess) == 0 &&
                                      cmp(g->get_material().transparency) == 0);
                if (hasNonSpecular)
                {
                    Photon photon(p , -photonRay.dir , alpha);
                    if (specularPath && nIntersections > 1)
                    {
                        if (!causticDone)
                        {
                            causticPhotons.push_back(photon);
                            if (causticPhotons.size() == nCausticPhotons)
                            {
                                causticDone = 1;
                                nCausticPaths = nShot;
                                causticMap = new PhotonKDtree();
                                causticMap->init(causticPhotons);
                                causticMap->build_tree(causticMap->root , 0);
                            }
                        }
                    }
                    else
                    {
                        if (nIntersections > 1 && !indirectDone)
                        {
                            indirectPhotons.push_back(photon);
                            if (indirectPhotons.size() == nIndirectPhotons)
                            {
                                indirectDone = 1;
                                nIndirectPaths = nShot;
                                
                                indirectMap = new PhotonKDtree();
                                indirectMap->init(indirectPhotons);
                                indirectMap->build_tree(indirectMap->root , 0);
                            }
                        }
                    }
                }
                if (nIntersections > maxPathLength)
                    break;

                /* find new photon ray direction */
                /* handle specular reflection and transmission first */
                Vector3 dir;
                if (cmp(g->get_material().shininess) > 0)
                {
                    dir = getReflectDir(photonRay , n);
                    Color3 brdf = calc_brdf(-photonRay.dir , dir , g , n);
                    Real cosTerm = fabs(n ^ photonRay.dir);
                    alpha = (alpha | brdf) * cosTerm;
                    if (cmp(alpha.r + alpha.g + alpha.b) <= 0)
                        break;
                    photonRay = Ray(p + dir * (10.0 * eps) , dir);
                }
                else if (cmp(g->get_material().transparency) > 0)
                {
                    dir = getTransDir(photonRay , n , g->get_material().refractionIndex , inside);
                    if (cmp(dir.sqr_length() - 100) > 0)
                        break;
                    Color3 btdf = calc_btdf(-photonRay.dir , dir , g , -n);
                    Real cosTerm = fabs(n ^ photonRay.dir);
                    alpha = (alpha | btdf) * cosTerm;
                    if (cmp(alpha.r + alpha.g + alpha.b) <= 0)
                        break;
                    photonRay = Ray(p + dir * (10.0 * eps) , dir);
                }
                else
                {
                    /* handle non-specular reflection by cosine sampling on
                       hemisphere */
                    dir = sample_dir_on_hemisphere(n);
                    Color3 brdf = calc_brdf(-photonRay.dir , dir , g , n);
                    alpha = (alpha | brdf) * PI;
                    specularPath = 0;
                    if (cmp(alpha.r + alpha.g + alpha.b) <= 0)
                        break;
                    photonRay = Ray(p + dir * (10.0 * eps) , dir);
                }
                /* Possibly terminate by Russian Roulette */
                if (nIntersections > 3)
                {
                    if (cmp(drand48() - 0.5) <= 0)
                        break;
                    alpha = alpha / 0.5;
                }
                g = scene.intersect(photonRay , t , p , n , inside);
            }
        }
    }
}

static Real kernel(const Photon *photon , const Vector3& p , const Real& msd)
{
    Vector3 d = p - photon->p;
    Real res = (1.0 - d.sqr_length() / msd);
    res = 3.0 / PI * SQR(res);
    return res;
}

Color3 PhotonIntegrator::estimate(PhotonKDtree *map , int nPaths , int knn ,
                                  Geometry* g , const Vector3& p ,
                                  const Vector3& n , const Vector3& wo ,
                                  Real maxSqrDis)
{
    Color3 res = Color3(0.0 , 0.0 , 0.0);
    
    if (map == NULL)
        return res;
    
    std::vector<ClosePhoton> kPhotons;
    Photon photon;
    photon.p = p;

    Real searchSqrDis = maxSqrDis;
    Real msd; /* max square distance */
    while (kPhotons.size() < knn)
    {
        msd = searchSqrDis;
        kPhotons.clear();
        map->search_k_photons(kPhotons , map->root , photon , knn , msd);
        searchSqrDis *= 2.0;
    }
    
    int nFoundPhotons = kPhotons.size();

    if (nFoundPhotons == 0)
        return res;
    
    Vector3 nv;
    if (cmp(wo ^ n) < 0)
        nv = -n;
    else
        nv = n;

    Real scale = 1.0 / (PI * msd * nFoundPhotons);
    
    for (int i = 0; i < nFoundPhotons; i++)
    {
        /*
        Real k = kernel(kPhotons[i].photon , p , msd);
        k = 1.0 / PI;
        Real scale = k / (nPaths * msd);
        */
        if (cmp(nv ^ kPhotons[i].photon->wi) > 0)
        {
            Color3 brdf = calc_brdf(kPhotons[i].photon->wi , wo , g , nv);
            res = res + (brdf | kPhotons[i].photon->alpha) * scale;
        }
        else
        {
            Color3 btdf = calc_btdf(kPhotons[i].photon->wi , wo , g , nv);
            res = res + (btdf | kPhotons[i].photon->alpha) * scale;
        }
    }
    return res;
}

Color3 PhotonIntegrator::raytracing(const Ray& ray , int dep)
{
    Color3 res = Color3(0.0 , 0.0 , 0.0);

    if (dep > max_tracing_depth)
        return res;

    Vector3 p , n;
    Real t;
    Geometry *g = NULL;
    int inside;

    g = scene.intersect(ray , t , p , n , inside);

    if (g == NULL)
        return res;
    
    res = res + estimate(indirectMap , nIndirectPaths , knnPhotons ,
                         g , p , n , -ray.dir , maxSqrDis);
    
    res = res + estimate(causticMap , nCausticPaths , knnPhotons ,
      g , p , n , -ray.dir , maxSqrDis);

    return res;
}

void PhotonIntegrator::render(char *filename)
{
	IplImage *img = 0;
	img = cvCreateImage(cvSize(width , height) , 
		IPL_DEPTH_8U , 3);
	Color3 res;
	Real y = view_port.l.y;
	for (int i = 0; i < height; i++)
	{
		Real x = view_port.l.x;
		for (int j = 0; j < width; j++)
		{
            res = Color3(0.0 , 0.0 , 0.0);
            for (int k = 0; k < samples_per_pixel; k++)
            {
                Vector3 v0 = Vector3(x - 0.5 * view_port.delta.x ,
                                     y - 0.5 * view_port.delta.y ,
                                     view_port.l.z);
                
                Vector3 v1 = Vector3(x + 0.5 * view_port.delta.x ,
                                     y - 0.5 * view_port.delta.y ,
                                     view_port.l.z);
                
                Vector3 v2 = Vector3(x - 0.5 * view_port.delta.x ,
                                     y + 0.5 * view_port.delta.y ,
                                     view_port.l.z);
                
                Vector3 pos_on_viewport = sample_on_rectangle_stratified(
                    v0 , v1 , v2 , k , samples_per_pixel);
                
                Ray ray = Ray(scene.camera , 
                              pos_on_viewport - scene.camera);

                Color3 tmp = raytracing(ray , 0);
                tmp.clamp();
                res = res + tmp;
            }
            res = res * (1.0 / (Real)samples_per_pixel);
            /*
            fprintf(fp , "c=(%.3lf,%.3lf,%.3lf)\n" ,
                    res.r , res.g , res.b);
            */
			int h = img->height;
			int w = img->width;
			int step = img->widthStep;
			int channels = img->nChannels;

			uchar *data = (uchar*)img->imageData;
			data[(h - i - 1) * step + j * channels + 0] = res.B();
			data[(h - i - 1) * step + j * channels + 1] = res.G();
			data[(h - i - 1) * step + j * channels + 2] = res.R();

			x += view_port.delta.x;
		}
		y += view_port.delta.y;
	}
	cvSaveImage(filename , img);
}
