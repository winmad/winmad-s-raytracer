#include "scene/AABB.h"
#include "scene/geometry.h"
#include "scene/sphere.h"
#include "scene/KDtree.h"
#include "scene/ray.h"

FILE* fp = fopen("debug.txt" , "w");

int main()
{
// 	AABB box = AABB(-1 , 1 , -1 , 1 , -1 , -1);
	Vector3 o = Vector3(2 , 2 , 5);
	Vector3 dir = Vector3(0 , 0 , -1);
	Ray ray = Ray(o , dir);
// 	if (box.hit(ray))
// 		fprintf(fp , "hit!\n");
// 	else
// 		fprintf(fp , "miss!\n");
	int tot = 2;
	Geometry* objlist[10000];
	Sphere *s;
	s = new Sphere(Vector3(0 , 0 , 0) , 1);
	objlist[0] = s;
	s = new Sphere(Vector3(-20 , -20 , -20) , 1);
	objlist[1] = s;
// 	s = new Sphere(Vector3(12 , 2 , 2) , 7);
// 	objlist[1] = s;
// 	s = new Sphere(Vector3(7 , 7 , 4) , 3);
// 	objlist[2] = s;
// 	s = new Sphere(Vector3(2 , 3 , 5) , 6);
// 	objlist[3] = s;

// 	for (int i = 0; i < tot; i++)
// 	{
// 		Real x , y , z , r;
// 		x = rand() % 100000;
// 		y = rand() % 100000;
// 		z = rand() % 100000;
// 		r = rand() % 100;
// 		s = new Sphere(Vector3(x , y , z) , r);
// 		objlist[i] = s;
// 	}

	KDtree tree;
	tree.init(tot , objlist);
	tree.build_tree(tree.root , 1);

	if (tree.traverse(ray , tree.root) == NULL)
		fprintf(fp , "miss\n");
	else
		fprintf(fp , "hit\n");
	/*print_tree(fp , tree.root);*/
	return 0;
}