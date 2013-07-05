#ifndef COLOR_H
#define COLOR_H

#include "vector.h"

class Color3
{
public:
	static const size_t DIM = 3;

	static const Color3 Black;
	static const Color3 White;
	static const Color3 Red;
	static const Color3 Green;
	static const Color3 Blue;

	Real r , g , b;

	Color3() 
		: r(0.0) , g(0.0) , b(0.0) {}

	Color3(Real r , Real g , Real b)
		: r(r) , g(g) , b(b) {}

	Color3(const Color3& color)
	{
		*this = color;
	}

	Color3& operator =(const Color3& color)
	{
		r = color.r; g = color.g; b = color.b;
		return *this;
	}
	
	void clamp()
	{
		r = clamp_val(r , 0.0 , 1.0);
		g = clamp_val(g , 0.0 , 1.0);
		b = clamp_val(b , 0.0 , 1.0);
	}
	
	unsigned char R()
	{
		return (unsigned char)(r * 255.0);
	}

	unsigned char G()
	{
		return (unsigned char)(g * 255.0);
	}

	unsigned char B()
	{
		return (unsigned char)(b * 255.0);
	}
};

const Color3 operator +(const Color3& , const Color3&);
const Color3 operator -(const Color3& , const Color3&);
const Color3 operator *(const Color3& , const Real&);
const Color3 operator |(const Color3& , const Color3&);

void print_color3(FILE* fp , const Color3& color);

#endif