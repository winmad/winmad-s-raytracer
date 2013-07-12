#include "path_tracer.h"
#include "sampler.h"

void Pathtracer::init(char *filename , Parameters& para)
{
	max_tracing_depth = para.MAX_TRACING_DEPTH;
	samples_per_pixel = para.SAMPLES_PER_PIXEL;
	samples_of_light = para.SAMPLES_OF_LIGHT;
	samples_of_hemisphere = para.SAMPLES_OF_HEMISPHERE;
	
	phong_power_index = para.PHONG_POWER_INDEX;

	scene.init(filename , para);
	view_port = scene.view_port;

	height = para.HEIGHT; width = para.WIDTH;

	view_port.delta.x = (view_port.r.x - view_port.l.x) / (Real)width;
	view_port.delta.y = (view_port.r.y - view_port.l.y) / (Real)height;
	view_port.delta.z = 0.0;
}

static Geometry* getCrossPointAll(Pathtracer& pathtracer ,const Ray& ray , Real& t , 
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

/* Phong model */
static Color3 calc_brdfTerm(Pathtracer& pathtracer , const Vector3& lightDir , 
                     const Vector3& visionDir , Geometry* g , const Vector3& n)
{
    Vector3 reflectDir = n * (lightDir ^ n) * 2.0 - lightDir;
    reflectDir.normalize();
    Real alpha = reflectDir ^ visionDir;
    Color3 res = g->get_material().diffuse / PI;
    res = res + g->get_material().specular *
        ((pathtracer.phong_power_index + 2) / 2 / PI * 
        pow(clamp_val(alpha , 0.0 , 1.0) , pathtracer.phong_power_index));
    return res;
}

static Color3 direct_illumination(Pathtracer& pathtracer , Geometry* g ,
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
    
    for (int i = 0; i < pathtracer.samples_of_light; i++)
    {
        int k = rand() % pathtracer.scene.lightlist.size();
        lightDir = pathtracer.scene.lightlist[k].pos - p;
        lightDir.normalize();

        ray = Ray(p + lightDir * (eps * 10.0) , lightDir);
        getCrossPointAll(pathtracer , ray , _t , _p , _n , _inside);
        t_light = calc_t(p , pathtracer.scene.lightlist[k].pos , lightDir);
        if (cmp(t_light - _t) > 0)
            continue;
        
        brdfTerm = calc_brdfTerm(pathtracer , lightDir , visionDir , g , n);

        //cosTerm = clamp_val(n ^ lightDir , 0.0 , 1.0);
        cosTerm = 1;

        res = res + (pathtracer.scene.lightlist[k].color | brdfTerm) *
            cosTerm;
    }
    res = res / pathtracer.samples_of_light;
    return res;
}

static Vector3 getTransDir(const Vector3 &rayDir , const Vector3& n , 
	const Real& refractionIndex , int inside)
{
	Real refraction;
	if (!inside) 
		refraction = refractionIndex;
	else
		refraction = 1.0 / refractionIndex;
	Vector3 N = n * (inside == 0 ? 1.0 : -1.0);
	Real cosI = -(N ^ rayDir);
	Real cosT = 1.0 - SQR(refraction) * (1.0 - SQR(cosI));
	Vector3 res;
	if (cmp(cosT) > 0)
	{
		res = rayDir * refraction + N * (refraction * cosI - sqrt(cosT));
		res.normalize();
		return res;
	}
	else
	{
		return Vector3(inf , inf , inf);
	}
}

//static FILE *fp = fopen("debug.txt" , "w");

static Color3 indirect_illumination(Pathtracer& pathtracer, Geometry* g ,
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

    Color3 reflectRes = Color3(0.0 , 0.0 , 0.0);
    
    for (int i = 0; i < pathtracer.samples_of_hemisphere; i++)
    {
        reflectDir = sample_dir_on_hemisphere(n);
        ray = Ray(p + reflectDir * (eps * 10.0) , reflectDir);
        tmp = pathtracer.raytracing(ray , dep + 1);

        brdfTerm = calc_brdfTerm(pathtracer , reflectDir , visionDir , g , n);

        reflectRes = reflectRes + (tmp | brdfTerm);
    }
    reflectRes = reflectRes / pathtracer.samples_of_hemisphere * PI;
    
    res = reflectRes;
    
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

    // transmission
    Color3 transRes = Color3(0.0 , 0.0 , 0.0);
    
    if (cmp(g->get_material().transparency) > 0 &&
        cmp(g->get_material().refractionIndex) != 0)
    {
        Vector3 transDir = getTransDir(ray.dir , n , g->get_material().refractionIndex , inside);
        
        if (cmp(transDir.sqr_length() - 1) <= 0)
        {
            Ray transRay = Ray(p + transDir * (eps * 10.0) , transDir);
            transRes = raytracing(transRay , dep + 1);
            transRes = transRes * g->get_material().transparency;
        }
    }
    
	Color3 emit , direct , indirect , res;
    emit = Color3(0 , 0 , 0);
    direct = direct_illumination(*this , g , p , n , -ray.dir);
    indirect = indirect_illumination(*this , g , p , n , -ray.dir , dep);
    res = emit + direct + indirect + transRes;
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
