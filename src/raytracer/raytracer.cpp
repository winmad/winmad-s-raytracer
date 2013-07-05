#include "raytracer.h"
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

Real getDirectCoefficient(Raytracer& raytracer , const Vector3& p)
{
	Ray ray = Ray(raytracer.scene.lightlist[0].pos , 
		p - raytracer.scene.lightlist[0].pos);
	Geometry *g = raytracer.scene.kdtree.traverse(ray ,
		raytracer.scene.kdtree.root);

	Real t;
	Vector3 _p , _n;
	int _inside;

	if (g == NULL)
		return 1.0;

	g->hit(ray , t , _p , _n , _inside);

	if (ray.origin + ray.dir * t == p)
		return 1.0;
	else 
		return 0.0;
}

/*
 * Old version
 */

// Geometry* getCrossPointAll(Raytracer& raytracer ,const Ray& ray , Real& t , 
// 	 	Vector3& p , Vector3& n , int& inside)
// {
// 	Vector3 tmp_p , tmp_n;
// 	Real tmp_t;
// 	int tmp_inside;
// 	Geometry *g = NULL;
// 	t = inf;
// 	for (int i = 0; i < raytracer.scene.objlist.size(); ++i)
// 	{
// 		raytracer.scene.objlist[i]->hit(ray , tmp_t , 
// 			tmp_p , tmp_n , tmp_inside);
// 		if (cmp(tmp_t - t) < 0)
// 		{
// 			t = tmp_t;
// 			p = tmp_p;
// 			n = tmp_n;
// 			g = raytracer.scene.objlist[i];
// 			inside = tmp_inside;
// 		}
// 	}
// 	return g;
// }
// 
// Real getT(const Ray& ray , const Vector3& point)
// {
// 	if (cmp(ray.dir.x) != 0) return (point.x - ray.origin.x) / ray.dir.x;
// 	if (cmp(ray.dir.y) != 0) return (point.y - ray.origin.y) / ray.dir.y;
// 	if (cmp(ray.dir.z) != 0) return (point.z - ray.origin.z) / ray.dir.z;
// }
// 
// Real getDirectCoefficient(Raytracer& raytracer , const Vector3& point)
// {
// 	Real res = 1.0;
// 	Vector3 lightSource = raytracer.scene.lightlist[0].pos;
// 	Ray ray = Ray(lightSource , point - lightSource);
// 	Real max_t = getT(ray , point);
// 	Real tmp_t;
// 	Vector3 tmp_p , tmp_n;
// 	int inside;
// 	for (int i = 0; i < raytracer.scene.objlist.size(); ++i)
// 	{
// 		raytracer.scene.objlist[i]->hit(ray , tmp_t , 
// 			tmp_p , tmp_n , inside);
// 		if (cmp(tmp_t - max_t) < 0)
// 		{
// 			res *= raytracer.scene.objlist[i]->get_material().transparency;
// 		}
// 	}
// 	return res;
// }

Vector3 getReflectDir(const Ray& ray , const Vector3& n)
{
	Vector3 res = -n * (n ^ ray.dir) * 2.0 + ray.dir;
	res.normalize();
	return res;
}

Vector3 getTransDir(const Ray& ray , const Vector3& n , 
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

	Color3 phongRes , reflectRes , transRes;
	Vector3 reflectDir , transDir;
	Ray reflectRay , transRay;

	Real directCoe = getDirectCoefficient(*this , p);
	phongRes = getPhong(p , -ray.dir , n , scene.lightlist[0].pos ,
		scene.lightlist[0].color , g , directCoe);

	if (g->get_material().shininess > 0)
	{
		reflectDir = getReflectDir(ray , n);
		reflectRay = Ray(p + reflectDir * (2 * eps) , reflectDir);
		reflectRes = raytracing(reflectRay , dep + 1);
	}

	if (g->get_material().transparency > 0 &&
		cmp(g->get_material().refractionIndex) != 0)
	{
		transDir = getTransDir(ray , n , 
			g->get_material().refractionIndex , inside);
		if (cmp(transDir.sqr_length() - 1) <= 0)
		{
			transRay = Ray(p + transDir * (2 * eps) , transDir);
			transRes = raytracing(transRay , dep + 1);
		}
		else
		{
			transRes = Color3(0.0 , 0.0 , 0.0);
		}
	}

	Color3 res;
	res = phongRes * (1 - inside) + 
		reflectRes * g->get_material().shininess +
		transRes * g->get_material().transparency;
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
			Ray ray = Ray(scene.camera , 
				Vector3(x , y , 0) - scene.camera);

			res = raytracing(ray , 0);
			res.clamp();

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