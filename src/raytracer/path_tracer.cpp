#include "path_tracer.h"
#include "sampler.h"

void Pathtracer::init(char *filename)
{
	scene.init(filename);
	view_port = scene.view_port;

	height = HEIGHT; width = WIDTH;

	view_port.delta.x = (view_port.r.x - view_port.l.x) / (Real)width;
	view_port.delta.y = (view_port.r.y - view_port.l.y) / (Real)height;
	view_port.delta.z = 0.0;
}

Geometry* getCrossPointAll(Pathtracer& pathtracer ,const Ray& ray , Real& t , 
	Vector3& p , Vector3& n , int& inside)
{
	Geometry *g = NULL;
	g = pathtracer.scene.kdtree.traverse(ray , 
		pathtracer.scene.kdtree.root);
	if (g != NULL)
	{
		g->hit(ray , t , p , n , inside);
	}
    else
    {
        t = inf;
    }
	return g;
}

Real calc_t(const Vector3& st , const Vector3& ed , const Vector3& dir)
{
    if (cmp(dir.x) != 0)
        return (ed.x - st.x) / dir.x;
    else if (cmp(dir.y) != 0)
        return (ed.y - st.y) / dir.y;
    else if (cmp(dir.z) != 0)
        return (ed.z - st.z) / dir.z;
    return inf;
}

/* Phong model */
Color3 calc_brdfTerm(const Vector3& lightDir , const Vector3& visionDir ,
                     Geometry* g , const Vector3& n)
{
    Vector3 reflectDir = n * 2.0 - lightDir;
    reflectDir.normalize();
    Real alpha = reflectDir ^ visionDir;
    Color3 res = g->get_material().diffuse / PI;
    res = res + g->get_material().specular *
        ((PHONG_POWER_INDEX + 2) / 2 / PI * 
        pow(clamp_val(alpha , 0.0 , 1.0) , PHONG_POWER_INDEX));
    return res;
}

Color3 direct_illumination(Pathtracer& pathtracer , Geometry* g ,
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
    
    for (int i = 0; i < SAMPLES_OF_LIGHT; i++)
    {
        int k = rand() % pathtracer.scene.lightlist.size();
        lightDir = pathtracer.scene.lightlist[k].pos - p;
        lightDir.normalize();

        ray = Ray(p + lightDir * (eps * 10.0) , lightDir);
        getCrossPointAll(pathtracer , ray , _t , _p , _n , _inside);
        t_light = calc_t(p , pathtracer.scene.lightlist[k].pos , lightDir);
        if (cmp(t_light - _t) > 0)
            continue;
        
        brdfTerm = calc_brdfTerm(lightDir , visionDir , g , n);

        //cosTerm = clamp_val(n ^ lightDir * (-1.0) , 0.0 , 1.0);
        cosTerm = 1;

        res = res + (pathtracer.scene.lightlist[k].color | brdfTerm) *
            cosTerm;
    }
    res = res / SAMPLES_OF_LIGHT;
    return res;
}

Color3 indirect_illumination(Pathtracer& pathtracer, Geometry* g ,
                             const Vector3& p , const Vector3& n ,
                             const Vector3& visionDir , int dep)
{
    Color3 res = Color3(0.0 , 0.0 , 0.0);
    Vector3 reflectDir;
    Ray ray;
    Color3 tmp , brdfTerm;

    Real absorb = 1.0 - (g->get_material().specular.r + g->get_material().specular.g + g->get_material().specular.b) / 3.0;

    if (cmp(drand48() - absorb) <= 0)
        return res;
    
    for (int i = 0; i < SAMPLES_OF_HEMISPHERE; i++)
    {
        reflectDir = sample_dir_on_hemisphere(n);
        ray = Ray(p + reflectDir * (eps * 10.0) , reflectDir);
        tmp = pathtracer.raytracing(ray , dep + 1);

        brdfTerm = calc_brdfTerm(reflectDir , visionDir , g , n);

        res = res + (tmp | brdfTerm);
    }
    res = res / SAMPLES_OF_HEMISPHERE * PI;
    return res / (1.0 - absorb);
}

Color3 Pathtracer::raytracing(const Ray& ray , int dep)
{
	if (dep > max_tracing_depth)
		return Color3(0.0 , 0.0 , 0.0);

	Vector3 p , n;
	Real t;
	Geometry *g = NULL;
	int inside;

	g = getCrossPointAll(*this , ray , t , p , n , inside);

	if (g == NULL)
		return Color3(0.0 , 0.0 , 0.0);

	Color3 direct , indirect , res;
    direct = direct_illumination(*this , g , p , n , -ray.dir);
    indirect = indirect_illumination(*this , g , p , n , -ray.dir , dep);
    res = direct + indirect;
    return res;
}

void Pathtracer::render(char *filename)
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
