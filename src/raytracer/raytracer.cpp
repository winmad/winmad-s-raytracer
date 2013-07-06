#include "raytracer.h"
#include "sampler.h"
#include "phong.h"

void Raytracer::init()
{
	scene.init();
	view_port = scene.view_port;

	height = 512; width = 512;

	view_port.delta.x = (view_port.r.x - view_port.l.x) / (Real)width;
	view_port.delta.y = (view_port.r.y - view_port.l.y) / (Real)height;
	view_port.delta.z = 0.0;
}

Geometry* getCrossPointAll(Raytracer& raytracer ,const Ray& ray , Real& t , 
	Vector3& p , Vector3& n , int& inside)
{
	Geometry *g = NULL;
	g = raytracer.scene.kdtree.traverse(ray , 
		raytracer.scene.kdtree.root);
	if (g != NULL)
	{
		g->hit(ray , t , p , n , inside);
	}
	return g;
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
        (52 / 2 / PI * pow(clamp_val(alpha , 0.0 , 1.0) , 50));
    return res;
}

Color3 direct_illumination(Raytracer& raytracer , Geometry* g ,
                           const Vector3& p , const Vector3& n ,
                           const Vector3& visionDir)
{
    Color3 res = Color3(0.0 , 0.0 , 0.0);
    Vector3 lightDir;
    Real cosTerm;
    Color3 brdfTerm;
    Ray ray;
    Real _t;
    Vector3 _p , _n;
    int _inside;

    int N = 4;
    
    for (int i = 0; i < N; i++)
    {
        int k = rand() % raytracer.scene.lightlist.size();
        lightDir = raytracer.scene.lightlist[k].pos - p;
        lightDir.normalize();

        ray = Ray(p + lightDir * eps * 10.0 , lightDir);
        if (getCrossPointAll(raytracer , ray , _t , _p , _n , _inside) == NULL)
            continue;
        
        brdfTerm = calc_brdfTerm(lightDir , visionDir , g , n);

        cosTerm = n ^ lightDir;

        res = res + (raytracer.scene.lightlist[k].color | brdfTerm) *
            (cosTerm * raytracer.scene.lightlist.size());
    }
    res = res / N * PI;
    return res;
}

Color3 indirect_illumination(Raytracer& raytracer, Geometry* g ,
                             const Vector3& p , const Vector3& n ,
                             const Vector3& visionDir , int dep)
{
    Color3 res = Color3(0.0 , 0.0 , 0.0);
    Vector3 reflectDir;
    Ray ray;
    Color3 tmp , brdfTerm;

    int N = 4;
    
    for (int i = 0; i < N; i++)
    {
        reflectDir = sample_dir_on_hemisphere(n);
        ray = Ray(p + reflectDir * eps * 10.0 , reflectDir);
        tmp = raytracer.raytracing(ray , dep + 1);

        brdfTerm = calc_brdfTerm(reflectDir , visionDir , g , n);

        res = res + (tmp | brdfTerm);
    }
    res = res / N;
    return res;
}

Color3 Raytracer::raytracing(const Ray& ray , int dep)
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

void Raytracer::render(char *filename)
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
