#include "sphere.h"

Material& Sphere::get_material()
{
	return material;
}

Color3 Sphere::get_diffuse_color(Vector3 pos)
{
	return material.diffuse;
}

bool Sphere::hit(const Ray& ray , Real& t , Vector3& p , 
	Vector3& n , int& inside)
{
	Real tmp1 , tmp2;
 	if (!box.hit(ray , tmp1 , tmp2))
 	{
 		t = inf;
 		return 0;
 	}

	Vector3 oc = center - ray.origin;
	Real l_oc = oc ^ oc;
	Real t_ca = oc ^ ray.dir;
	if (cmp(t_ca) < 0)
	{
		t = inf;
		return 0;
	}
	Real t_hc = SQR(radius) - l_oc + SQR(t_ca);
	if (cmp(t_hc) <= 0)
	{
		t = inf;
		return 0;
	}
	Real t1 , t2 , d = sqrt(t_hc);
	t1 = t_ca - d; t2 = t_ca + d;
	if (cmp(t2) <= 0)
	{
		t = inf;
		return 0;
	}
	else
	{
		if (cmp(t1) <= 0)
		{
			t = t2;
			inside = 1;
		}
		else
		{
			t = t1;
			inside = 0;
		}
	}
	p = ray.origin + ray.dir * t;
	n = p - center;
	n.normalize();
	return 1;
}