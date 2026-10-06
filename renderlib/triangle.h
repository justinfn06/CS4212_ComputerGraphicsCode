#pragma once

#include "vec3.h"
#include "Shape.h"

class Triangle : public Shape
{
public:
    Triangle() : vertex_a(vec3(0, 0, 0)), vertex_b(vec3(1, 0, 0)), vertex_c(vec3(0, 1, 0)), color(vec3(1.0, 1.0, 1.0)) {}
    Triangle(const vec3& a, const vec3& b, const vec3& c) : vertex_a(a), vertex_b(b), vertex_c(c), color(vec3(1.0, 1.0, 1.0)) {}
    Triangle(const vec3& a, const vec3& b, const vec3& c, const vec3& col) : vertex_a(a), vertex_b(b), vertex_c(c), color(col) {}

    bool intersect(const ray& r, float t_min, float& t_max, HitStruct& hit) const override;
    vec3 getColor() const override;
private:
    vec3 vertex_a, vertex_b, vertex_c;
    vec3 color;
};