#include "math.h"

Real clamp_val(Real val , Real min_val , Real max_val)
{
	return std::min(max_val , std::max(val , min_val));
}


int cmp(const Real& x)
{
	return ((x < -eps) ? -1 : (x > eps));
}
