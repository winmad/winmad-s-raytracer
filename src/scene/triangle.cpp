#include "triangle.h"

Material& Triangle::get_material()
{
	return material;
}

Color3 Triangle::get_diffuse_color(Vector3 pos)
{
	return material.diffuse;
}

bool Triangle::hit(const Ray& ray , Real& t , 
	Vector3& p , Vector3& n , int& inside)
{
	Real A = p0.x - p1.x;
	Real B = p0.y - p1.y;
	Real C = p0.z - p1.z;

	Real D = p0.x - p2.x;
	Real E = p0.y - p2.y;
	Real F = p0.z - p2.z;

	Real G = ray.dir.x;
	Real H = ray.dir.y;
	Real I = ray.dir.z;

	Real J = p0.x - ray.origin.x;
	Real K = p0.y - ray.origin.y;
	Real L = p0.z - ray.origin.z;

	Real EIHF = E * I - H * F;
	Real GFDI = G * F - D * I;
	Real DHEG = D * H - E * G;

	Real denom = (A * EIHF + B * GFDI + C * DHEG);

	Real beta = (J * EIHF + K * GFDI + L * DHEG) / denom;
	if (cmp(beta) < 0 || cmp(beta - 1) > 0)
	{
		t = inf;
		return 0;
	}

	Real AKJB = A * K - J * B;
	Real JCAL = J * C - A * L;
	Real BLKC = B * L - K * C;

	Real gamma = (I * AKJB + H * JCAL + G * BLKC) / denom;
	if (cmp(gamma) < 0 || cmp(beta + gamma - 1) > 0)
	{
		t = inf;
		return 0;
	}

	t = -(F * AKJB + E * JCAL + D * BLKC) / denom;
	if (cmp(t) <= 0)
	{
		t = inf;
		return 0;
	}
	p = ray.origin + ray.dir * t;
	n = (p1 - p0) * (p2 - p0);
	n.normalize();
	if (cmp(ray.dir ^ n) < 0)
		inside = 0;
	else 
		inside = 1;
	return 1;
}