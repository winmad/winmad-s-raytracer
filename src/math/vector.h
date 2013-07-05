#ifndef VECTOR_H
#define VECTOR_H

#include "math.h"

class Vector3
{
public:
	static const size_t DIM = 3;

	static const Vector3 Zero;
	static const Vector3 Ones;
	static const Vector3 UnitX;
	static const Vector3 UnitY;
	static const Vector3 UnitZ;

	Real x , y , z;

	Vector3()
		: x(0) , y(0) , z(0) {}

	Vector3(Real x , Real y , Real z)
		: x(x) , y(y) , z(z) {}

	Vector3(const Vector3& v)
	{
		*this = v;
	}

	Vector3& operator =(const Vector3& v)
	{
		x = v.x; y = v.y; z = v.z;
		return *this;
	}

	const Vector3 operator -() const
	{
		return Vector3(-x , -y , -z);
	}

	Real sqr_length()
	{
		return SQR(x) + SQR(y) + SQR(z);
	}

	void normalize()
	{
		Real len = sqrt(this->sqr_length());
		x /= len; y /= len; z /= len;
	}
};

const Vector3 operator +(const Vector3& , const Vector3&);
const Vector3 operator -(const Vector3& , const Vector3&);
const Real operator ^(const Vector3& , const Vector3&);
const Vector3 operator |(const Vector3& , const Vector3&);
const Vector3 operator *(const Vector3& , const Real&);
const Vector3 operator *(const Vector3& , const Vector3&);
const Vector3 operator /(const Vector3& , const Real&);
bool operator ==(const Vector3& , const Vector3&);
bool operator !=(const Vector3& , const Vector3&);

void print_vector3(FILE* fp , const Vector3& v);

#endif