#pragma once

#include "../core/vec3.h"
#include "shape.h"


class Sphere : public Shape
{
public:
    Sphere(vec3 c, float r, Shader& shader)
    : Shape(shader),
      center(c),
      radius(r),
      color(vec3(1, 1, 1))
{
}

    bool intersect(const ray& r, float t_min, float& t_max, HitStruct& hit) const override;
    vec3 getColor() const override;
private:
    vec3 center;
    float radius;
    vec3 color;
};