#include "vector.h"

const Vector3 Vector3::Zero = Vector3(0.0 , 0.0 , 0.0);
const Vector3 Vector3::Ones = Vector3(1.0 , 1.0 , 1.0);
const Vector3 Vector3::UnitX = Vector3(1.0 , 0.0 , 0.0);
const Vector3 Vector3::UnitY = Vector3(0.0 , 1.0 , 0.0);
const Vector3 Vector3::UnitZ = Vector3(0.0 , 0.0 , 1.0);

const Vector3 operator +(const Vector3& left , const Vector3& right)
{
	return Vector3(left.x + right.x , left.y + right.y , left.z + right.z);
}

const Vector3 operator -(const Vector3& left , const Vector3& right)
{
	return Vector3(left.x - right.x , left.y - right.y , left.z - right.z);
}

const Real operator ^(const Vector3& left , const Vector3& right)
{
	return left.x * right.x + left.y * right.y + left.z * right.z;
}

const Vector3 operator |(const Vector3& left , const Vector3& right)
{
	return Vector3(left.x * right.x , left.y * right.y , left.z * right.z);
}

const Vector3 operator *(const Vector3& left , const Vector3& right)
{
	return Vector3(left.y * right.z - left.z * right.y ,
				   left.z * right.x - left.x * right.z ,
				   left.x * right.y - left.y * right.x);
}

const Vector3 operator *(const Vector3& left , const Real& right)
{
	return Vector3(left.x * right , left.y * right , left.z * right);
}

const Vector3 operator /(const Vector3& left , const Real& right)
{
	if (cmp(right) == 0) return Vector3(inf , inf , inf);
	return Vector3(left.x / right , left.y / right , left.z / right);
}

bool operator ==(const Vector3& left , const Vector3& right)
{
	return (cmp(left.x - right.x) == 0 && cmp(left.y - right.y) == 0 &&
			cmp(left.z - right.z) == 0);
}

bool operator !=(const Vector3& left , const Vector3& right)
{
	return !(left == right);
}

void print_vector3(FILE* fp , const Vector3& v)
{
	fprintf(fp , "(%.3lf,%.3lf,%.3lf)" , v.x , v.y , v.z);
}