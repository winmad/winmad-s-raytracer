#include "color.h"

const Color3 Color3::Black = Color3(0.0 , 0.0 , 0.0);
const Color3 Color3::White = Color3(1.0 , 1.0 , 1.0);
const Color3 Color3::Red = Color3(1.0 , 0.0 , 0.0);
const Color3 Color3::Green = Color3(0.0 , 1.0 , 0.0);
const Color3 Color3::Blue = Color3(0.0 , 0.0 , 1.0);

const Color3 operator +(const Color3& left , const Color3& right)
{
	Color3 res = Color3(left.r + right.r , left.g + right.g , left.b + right.b);
	res.clamp();
	return res;
}

const Color3 operator -(const Color3& left , const Color3& right)
{
	Color3 res = Color3(left.r - right.r , left.g - right.g , left.b - right.b);
	res.clamp();
	return res;
}

const Color3 operator *(const Color3& left , const Real& right)
{
	Color3 res = Color3(left.r * right , left.g * right , left.b * right);
	res.clamp();
	return res;
}

const Color3 operator |(const Color3& left , const Color3& right)
{
	Color3 res = Color3(left.r * right.r , left.g * right.g , left.b * right.b);
	res.clamp();
	return res;
}

void print_color3(FILE* fp , const Color3& color)
{
	fprintf(fp , "(%.3lf,%.3lf,%.3lf)" , color.r , color.g , color.b);
}