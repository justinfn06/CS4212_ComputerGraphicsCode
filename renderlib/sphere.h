#pragma once

#include "vec3.h"
#include "Shape.h"


class Sphere : public Shape
{
public:
    Sphere(): center(vec3(0, 0, 0)), radius(1.0), color(vec3(1.0, 1.0, 1.0)) {}
    Sphere(vec3 c, float r): center(c), radius(r), color(vec3(1.0, 1.0, 1.0)) {}
    Sphere(vec3 c, float r, vec3 col): center(c), radius(r), color(col) {}

    bool intersect(const ray& r, float t_min, float& t_max, HitStruct& hit) const override;
    vec3 getColor() const override;
private:
    vec3 center;
    float radius;
    vec3 color;
};