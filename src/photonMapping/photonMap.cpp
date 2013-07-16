#include "photonMap.h"
#include "../raytracer/sampler.h"

static FILE *fp = fopen("debug_pm.txt" , "w");

void PhotonIntegrator::init(char *filename , Parameters& para)
{
    nCausticPhotons = 20000;
    
    nIndirectPhotons = 100000;

    knnPhotons = 50;

    maxSqrDis = 1000;
    
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
                                visualize(causticPhotons , view_port ,
                                          scene , "caustic_photon_map.bmp");
                                
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
                                visualize(indirectPhotons , view_port ,
                                          scene , "indirect_photon_map.bmp");
                                
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

static Color3 estimate(PhotonKDtree *map , int nPaths , int knn ,
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

static Real calc_t(const Vector3& st , const Vector3& ed , const Vector3& dir)
{
    if (cmp(dir.x) != 0)
        return (ed.x - st.x) / dir.x;
    else if (cmp(dir.y) != 0)
        return (ed.y - st.y) / dir.y;
    else if (cmp(dir.z) != 0)
        return (ed.z - st.z) / dir.z;
    return inf;
}

static Color3 directIllumination(Scene& scene , Geometry* g ,
                           const Vector3& p , const Vector3& n ,
                           const Vector3& visionDir)
{
    Color3 res = Color3(0.0 , 0.0 , 0.0);
    Vector3 lightDir;
    Real cosTerm;
    Color3 brdfTerm;
    Ray ray;
    Real _t , t_light;
    Vector3 _p , _n;
    int _inside;
    
    for (int i = 0; i < 8; i++)
    {
        int k = rand() % scene.lightlist.size();
        lightDir = scene.lightlist[k].pos - p;
        lightDir.normalize();

        ray = Ray(p + lightDir * (eps * 10.0) , lightDir);
        scene.intersect(ray , _t , _p , _n , _inside);
        t_light = calc_t(p , scene.lightlist[k].pos , lightDir);
        if (cmp(t_light - _t) > 0)
            continue;
        
        brdfTerm = calc_brdf(lightDir , visionDir , g , n);

        cosTerm = clamp_val(n ^ lightDir , 0.0 , 1.0);
        //cosTerm = 1;

        res = res + (scene.lightlist[k].color | brdfTerm) *
            cosTerm;
    }
    res = res / 8;
    return res;
}

static Color3 finalGathering(PhotonKDtree *map , Scene& scene , Geometry *g ,
                             const Vector3& p , const Vector3& n , const Vector3& wo ,
                             int gatherSamples , int knn , Real maxSqrDis)
{
    Color3 res = Color3(0.0 , 0.0 , 0.0);
    for (int i = 0; i < gatherSamples; i++)
    {
        Vector3 wi = sample_dir_on_hemisphere(n);
        Ray ray = Ray(p + wi * (10.0 * eps) , wi);

        Real _t;
        Vector3 _p , _n;
        int _inside;
        
        Geometry *_g = scene.intersect(ray , _t , _p , _n , _inside);

        if (_g == NULL)
            continue;
        
        Color3 tmp = estimate(map , 0 , knn , _g , _p , _n , -wi , maxSqrDis);

        Color3 brdf = calc_brdf(wi , wo , g , n);
        res = res + (tmp | brdf) * PI;
    }
    res = res / gatherSamples;
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
    Ray reflectRay , transRay;

    g = scene.intersect(ray , t , p , n , inside);

    if (g == NULL)
        return res;
    
    //res = res + directIllumination(scene , g , p , n , -ray.dir);
      
    //res = res + estimate(indirectMap , nIndirectPaths , knnPhotons , g , p , n , -ray.dir , maxSqrDis);

    //Color3 t1 = estimate(indirectMap , nIndirectPaths , knnPhotons , g , p , n , -ray.dir , maxSqrDis);
    //Color3 t2 = finalGathering(indirectMap , scene , g , p , n , -ray.dir , 50 , knnPhotons , maxSqrDis);
    
    res = res + finalGathering(indirectMap , scene , g , p , n , -ray.dir , 50 , knnPhotons , maxSqrDis);
    /*
    res = res + estimate(causticMap , nCausticPaths , knnPhotons , g , p , n , -ray.dir , maxSqrDis);

    if (cmp(g->get_material().shininess) > 0)
    {
        Vector3 dir = getReflectDir(ray , n);
        reflectRay = Ray(p + dir * (10.0 * eps) , dir);
        res = res + raytracing(reflectRay , dep + 1);
    }

    if (cmp(g->get_material().transparency) > 0)
    {
        Vector3 dir = getTransDir(ray , n , g->get_material().refractionIndex , inside);
        if (cmp(dir.sqr_length()) <= 1)
        {
            transRay = Ray(p + dir * (10.0 * eps) , dir);
            res = res + raytracing(transRay , dep + 1);
        }
    }
    */
    return res;
}

void PhotonIntegrator::visualize(const std::vector<Photon>& photons ,
                                 const ViewPort& viewPort ,
                                 Scene& scene , char *filename)
{
    IplImage *img = 0;
    img = cvCreateImage(cvSize(width , height) ,
                        IPL_DEPTH_8U , 3);
    Color3 res;

    Color3 **col;
    int **cnt;

    col = new Color3*[height];
    cnt = new int*[height];
    for (int i = 0; i < height; i++)
    {
        col[i] = new Color3[width];
        cnt[i] = new int[width];
        for (int j = 0; j < width; j++)
        {
            col[i][j] = Color3(0.0 , 0.0 , 0.0);
            cnt[i][j] = 0;
        }
    }

    for (int i = 0; i < photons.size(); i++)
    {
        Photon photon = photons[i];
        Vector3 p = photon.p;
        Real x , y , z0 , t;
        z0 = viewPort.l.z;
        Vector3 dir = p - scene.camera;
        if (cmp(dir.z) != 0)
        {
            t = (z0 - p.z) / dir.z;
            x = p.x + t * dir.x;
            y = p.y + t * dir.y;
        }
        else
        {
            t = inf;
            x = 0;
            y = 0;
        }

        Ray shadowRay = Ray(p - dir * (10.0 * eps) , -dir);
        if (scene.intersect(shadowRay) != NULL)
            continue;
        
        int r , c;
        r = round((y - viewPort.l.y) / (viewPort.r.y - viewPort.l.y) * (double)height);
        c = round((x - viewPort.l.x) / (viewPort.r.x - viewPort.l.x) * (double)width);
        r = std::max(0 , std::min(r , height - 1));
        c = std::max(0 , std::min(c , width - 1));
        cnt[r][c]++;
        col[r][c] = col[r][c] + photon.alpha;
    }

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            col[i][j] = col[i][j] / ((double)cnt[i][j] * 4 * PI * 100);
            int h = img->height;
			int w = img->width;
			int step = img->widthStep;
			int channels = img->nChannels;

			uchar *data = (uchar*)img->imageData;
			data[(h - i - 1) * step + j * channels + 0] = col[i][j].B();
			data[(h - i - 1) * step + j * channels + 1] = col[i][j].G();
			data[(h - i - 1) * step + j * channels + 2] = col[i][j].R();
        }
    }

    for (int i = 0; i < height; i++)
    {
        delete[] col[i];
        delete[] cnt[i];
    }
    delete[] col;
    delete[] cnt;
    cvSaveImage(filename , img);
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
