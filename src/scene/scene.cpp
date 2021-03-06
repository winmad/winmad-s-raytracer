#include "scene.h"
#include "../raytracer/sampler.h"
#include "../tinyxml/tinyxml.h"

void Scene::add_geometry(Geometry* g)
{
	objlist.push_back(g);
}

void Scene::add_light(PointLight l)
{
	lightlist.push_back(l);
}

Geometry* Scene::intersect(const Ray& ray , Real& t , Vector3& p ,
                           Vector3& n , int& inside)
{
    Geometry *g = NULL;
    g = kdtree.traverse(ray , kdtree.root);
    if (g != NULL)
        g->hit(ray , t , p , n , inside);
    return g;
}

bool Scene::intersect(const Ray& ray)
{
    Geometry *g = NULL;
    g = kdtree.traverse(ray , kdtree.root);
    if (g == NULL)
        return 0;
    else
        return 1;
}

// build-in scene: many balls
void Scene::load_scene()
{
	PointLight l = PointLight(Vector3(-3 , 5 , 1) ,
		Color3(1 , 1 , 1));
	add_light(l);

 	Sphere *s;
 	s = new Sphere(Vector3(2 , 0.8 , 3.0) , 2);
 	s->material.set_material(Color3(0.7 , 0.7 , 1.0) , 
 		Color3(0 , 0 , 0) , Color3(0.2 , 0.2 , 0.2) , 
 		0.2 , 0.8 , 0.7);
 	add_geometry(s);
 
 	s = new Sphere(Vector3(-5.5 , -0.5 , 7) , 2);
 	s->material.set_material(Color3(0.7 , 0.7 , 1.0) , 
 		Color3(0.1 , 0.1 , 0.1) , Color3(0.2 , 0.2 , 0.2) ,
 		0.5 , 0.5 , 0.7);
 	add_geometry(s);
 
 	for (int x = 0; x < 10; x++)
 	{
 		for (int y = 0; y < 10; y++)
 		{
 			s = new Sphere(Vector3(-2.0 + x * 1.2 ,
 				-4.3 + y * 1.5 , 10 + x * 0.3) , 0.3);
 			s->material.set_material(Color3(0.3 , 1.0 , 0.4) ,
 				Color3(0.6 , 0.6 , 0.6) , Color3(0.2 , 0.2 , 0.2) ,
 				0.0 , 0.5 , 0.0);
 			add_geometry(s);
 		}
 	}

	camera = Vector3(0 , 0 , -5);

    view_port.l = Vector3(-4 , -3 , 0);
    view_port.r = Vector3(4 , 3 , 0);
}

// load .scene, xml format
void Scene::load_scene(char* filename)
{
	TiXmlDocument doc = TiXmlDocument(filename);
	doc.LoadFile();

	TiXmlElement *root = doc.RootElement();
	
	TiXmlElement *it = root->FirstChildElement();

	while (it)
	{
		if (it->ValueStr() == "camera")
		{
			TiXmlElement *attr = it->FirstChildElement();
			attr->Attribute("x" , &camera.x);
			attr->Attribute("y" , &camera.y);
			attr->Attribute("z" , &camera.z);
			
		}
		else if (it->ValueStr() == "viewport")
		{
			TiXmlElement *attr = it->FirstChildElement();
			attr->Attribute("x1" , &view_port.l.x);
			attr->Attribute("x2" , &view_port.r.x);
			attr->Attribute("y1" , &view_port.l.y);
			attr->Attribute("y2" , &view_port.r.y);
			attr->Attribute("z1" , &view_port.l.z);
			attr->Attribute("z2" , &view_port.r.z);
		}
		else if (it->ValueStr() == "point_light")
		{
			TiXmlElement *attr = it->FirstChildElement();
			PointLight l;
			attr->Attribute("x" , &l.pos.x);
			attr->Attribute("y" , &l.pos.y);
			attr->Attribute("z" , &l.pos.z);
			attr = attr->NextSiblingElement();
			attr->Attribute("r" , &l.color.r);
			attr->Attribute("g" , &l.color.g);
			attr->Attribute("b" , &l.color.b);

			add_light(l);
		}
        else if (it->ValueStr() == "area_light")
        {
            TiXmlElement *attr = it->FirstChildElement();
            Mesh mesh;
            std::string filename = attr->Attribute("path");
            mesh.load(filename.c_str());

            Color3 color;
            attr = attr->NextSiblingElement();
            attr->Attribute("r" , &color.r);
            attr->Attribute("g" , &color.g);
            attr->Attribute("b" , &color.b);
            
            Triangle t;
			for (int i = 0; i < mesh.triangles.size(); i++)
			{
				t = Triangle(mesh.vertices[mesh.triangles[i].v[0]] ,
					mesh.vertices[mesh.triangles[i].v[1]] ,
					mesh.vertices[mesh.triangles[i].v[2]]);
                t.get_material().diffuse = color;
				area_lightlist.push_back(t);
			}
        }
		else if (it->ValueStr() == "object")
		{
			TiXmlElement *attr = it->FirstChildElement();
			Mesh mesh;
			std::string filename = attr->Attribute("path");
			mesh.load(filename.c_str());

			attr = attr->NextSiblingElement();
			Color3 diffuse;
			attr->Attribute("r" , &diffuse.r);
			attr->Attribute("g" , &diffuse.g);
			attr->Attribute("b" , &diffuse.b);

            attr = attr->NextSiblingElement();
            Color3 specular;
            attr->Attribute("r" , &specular.r);
            attr->Attribute("g" , &specular.g);
            attr->Attribute("b" , &specular.b);
                
			attr = attr->NextSiblingElement();
			Color3 ambient;
			attr->Attribute("r" , &ambient.r);
			attr->Attribute("g" , &ambient.g);
			attr->Attribute("b" , &ambient.b);

			attr = attr->NextSiblingElement();
			Real shininess = 0.0;
			attr->Attribute("shininess" , &shininess);

            attr = attr->NextSiblingElement();
            Real transparency = 0.0;
            attr->Attribute("transparency" , &transparency);

            attr = attr->NextSiblingElement();
            Real refractionIndex = 0.0;
            attr->Attribute("refractionIndex" , &refractionIndex);
            
			Triangle *t;
			for (int i = 0; i < mesh.triangles.size(); i++)
			{
				t = new Triangle(mesh.vertices[mesh.triangles[i].v[0]] ,
					mesh.vertices[mesh.triangles[i].v[1]] ,
					mesh.vertices[mesh.triangles[i].v[2]]);
				t->get_material().diffuse = diffuse;
				t->get_material().specular = specular;
				t->get_material().ambient = ambient;
				t->get_material().shininess = shininess;
                t->get_material().transparency = transparency;
                t->get_material().refractionIndex = refractionIndex;

				add_geometry(t);
			}
		}
		else if (it->ValueStr() == "sphere")
		{
            TiXmlElement *attr = it->FirstChildElement();
            Vector3 origin;
			attr->Attribute("x" , &origin.x);
            attr->Attribute("y" , &origin.y);
            attr->Attribute("z" , &origin.z);

            attr = attr->NextSiblingElement();
            Real radius;
            attr->Attribute("r" , &radius);

			attr = attr->NextSiblingElement();
			Color3 diffuse;
			attr->Attribute("r" , &diffuse.r);
			attr->Attribute("g" , &diffuse.g);
			attr->Attribute("b" , &diffuse.b);

            attr = attr->NextSiblingElement();
            Color3 specular;
            attr->Attribute("r" , &specular.r);
            attr->Attribute("g" , &specular.g);
            attr->Attribute("b" , &specular.b);
                
			attr = attr->NextSiblingElement();
			Color3 ambient;
			attr->Attribute("r" , &ambient.r);
			attr->Attribute("g" , &ambient.g);
			attr->Attribute("b" , &ambient.b);

			attr = attr->NextSiblingElement();
			Real shininess = 0.0;
			attr->Attribute("shininess" , &shininess);

            attr = attr->NextSiblingElement();
            Real transparency = 0.0;
            attr->Attribute("transparency" , &transparency);

            attr = attr->NextSiblingElement();
            Real refractionIndex = 0.0;
            attr->Attribute("refractionIndex" , &refractionIndex);

            Sphere *s;
			s = new Sphere(origin , radius);
            s->get_material().diffuse = diffuse;
            s->get_material().specular = specular;
            s->get_material().ambient = ambient;
            s->get_material().shininess = shininess;
            s->get_material().transparency = transparency;
            s->get_material().refractionIndex = refractionIndex;

            add_geometry(s);
		}
		it = it->NextSiblingElement();
	}
}

void Scene::init(char* filename , Parameters& para)
{
	point_light_num = para.POINT_LIGHT_NUM;
	
	//load_scene();
	load_scene(filename);

    /* sample point light from area light */
    lightlist = sample_points_on_area_light(area_lightlist , point_light_num);
    
	kdtree.init(objlist);
	kdtree.build_tree(kdtree.root , 1);
}
