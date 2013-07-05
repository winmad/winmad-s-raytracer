#ifndef MATH_H
#define MATH_H

#include <cstdio>
#include <cmath>
#include <algorithm>

#define SQR(x) ((x) * (x))

typedef double Real;

const Real PI = acos(-1.0);
const Real eps = 1e-3;
const Real inf = 1e20;

Real clamp_val(Real val , Real min_val , Real max_val);

int cmp(const Real& x);

#endif