#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "shape.h"
#include "triangle.h"

class TriangleList : public Shape
{
public:
    TriangleList( const std::vector<float>& values, Shader& material, double scale = 1.0, const vec3& offset = vec3(0, 0, 0)) : Shape(material)
    {
        for (std::size_t i = 0; i < values.size(); i += 9) {
            vec3 a(values[i],     values[i + 1], values[i + 2]);
            vec3 b(values[i + 3], values[i + 4], values[i + 5]);
            vec3 c(values[i + 6], values[i + 7], values[i + 8]);

            a = scale * a + offset;
            b = scale * b + offset;
            c = scale * c + offset;

            triangles.emplace_back(a, b, c, material);
        }
    }

    bool intersect(const ray& r, float t_min, float& t_max, HitStruct& hit) const override
    {
        bool hitAnything = false;

        // iterate over all triangles
        for (const Triangle& triangle : triangles) {
            HitStruct candidate;

            if (triangle.intersect(r, t_min, t_max, candidate)) {
                hit = candidate;
                hit.shape = this;
                hitAnything = true;
            }
        }

        return hitAnything;
    }

    vec3 getColor() const override
    {
        return vec3(1, 1, 1);
    }

private:
    // list of all triangles
    std::vector<Triangle> triangles;
};