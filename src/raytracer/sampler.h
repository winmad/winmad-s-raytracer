#ifndef SAMPLER_H
#define SAMPLER_H

#include "../math/vector.h"
#include "../scene/scene.h"
#include <vector>

Vector3 sample_on_triangle(
    const Vector3& v1 , const Vector3& v2 ,
    const Vector3& v3);

Vector3 sample_on_rectangle(
    const Vector3& v0 , const Vector3& v1 ,
    const Vector3& v2);

Vector3 sample_on_rectangle_stratified(
    const Vector3& v0 , const Vector3& v1 ,
    const Vector3& v2 , int curLayer , int totLayer);

/* Importance sampling: cosine , pdf = cos(theta) / (2 * PI) */
Vector3 sample_dir_on_hemisphere(const Vector3& n);

/* Uniform sampling , pdf = 1 / (4 * PI) */
Vector3 uniform_sample_dir_on_sphere();

std::vector<PointLight> sample_points_on_area_light(
    std::vector<Triangle>& triangles , int totSamples);

#endif
