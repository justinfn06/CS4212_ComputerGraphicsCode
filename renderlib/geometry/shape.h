#pragma once

#include "../core/HitStruct.h"
#include "../math/ray.h"
#include "../materials/shader.h"

class Shape
{
public:
    explicit Shape(Shader& shader)
        : shader(shader) {}

    virtual ~Shape() = default;

    virtual bool intersect(
        const ray& r,
        float t_min,
        float& t_max,
        HitStruct& hit
    ) const = 0;

    virtual vec3 getColor() const = 0;

    vec3 shade(
        const HitStruct& hit,
        const std::vector<LightSample>& lights,
        const Scene& scene,
        int depth
    ) const
    {
        return shader.rayColor(hit, lights, scene, depth);
    }

private:
    Shader& shader;
};