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

        // 3 floats = 1 vertex
        auto vertex = [&](std::size_t i) {
            return scale * vec3(values[i], values[i + 1], values[i + 2]) + offset;
        };

        // 3 vertex = 1 triangle
        for (std::size_t i = 0; i < values.size(); i += 9) {
            triangles.emplace_back(vertex(i), vertex(i + 3), vertex(i + 6), material);
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