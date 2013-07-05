#ifndef MATERIAL_H
#define MATERIAL_H

#include "../math/color.h"
#include "../math/vector.h"

class Material
{
public:
	Color3 diffuse;
	Color3 specular;
	Color3 ambient;
	Real shininess;
	Real transparency;
	Real refractionIndex;

	Material() 
	{
		diffuse = Color3(0.0 , 0.0 , 0.0);
		specular = Color3(0.0 , 0.0 , 0.0);
		ambient = Color3(0.0 , 0.0 , 0.0);

		shininess = 0.0;
		transparency = 0.0;
		refractionIndex = 0.0;
	}

	void set_material(const Color3& _diffuse , const Color3& _specular ,
		const Color3& _ambient , const Real& _shininess , 
		const Real& _transparency , const Real& _refractionIndex)
	{
		diffuse = _diffuse;
		specular = _specular;
		ambient = _ambient;
		shininess = _shininess;
		transparency = _transparency;
		refractionIndex = _refractionIndex;
	}
};

#endif