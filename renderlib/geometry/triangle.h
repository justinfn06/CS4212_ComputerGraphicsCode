#pragma once

#include "../core/vec3.h"
#include "shape.h"
#include <algorithm>

class Triangle : public Shape
{
public:
    Triangle(const vec3& a, const vec3& b, const vec3& c,
         Shader& shader)
    : Shape(shader),
      vertex_a(a),
      vertex_b(b),
      vertex_c(c),
      color(vec3(1, 1, 1))
{
}

    bool intersect(const ray& r, float t_min, float& t_max, HitStruct& hit) const override;
    vec3 getColor() const override;

    BoundingBox boundingBox() const override
    {
        point3 minimum;
        point3 maximum;

        for (int axis = 0; axis < 3; ++axis) {
            minimum[axis] = std::min({
                vertex_a[axis],
                vertex_b[axis],
                vertex_c[axis]
            });

            maximum[axis] = std::max({
                vertex_a[axis],
                vertex_b[axis],
                vertex_c[axis]
            });
        }

        return BoundingBox(minimum, maximum);
    }

    point3 centroid() const override
    {
        return (vertex_a + vertex_b + vertex_c) / 3.0;
    }
private:
    vec3 vertex_a, vertex_b, vertex_c;
    vec3 color;
};