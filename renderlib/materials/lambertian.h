#pragma once
#include <algorithm>
#include "shader.h"

class Lambertian : public Shader
{
public:
    explicit Lambertian(vec3 diffuseColor)
        : kd(diffuseColor) {}

    vec3 rayColor(
        const HitStruct& h,
        const std::vector<LightSample>& lights,
        const Scene&,
        int
    ) override
    {
        vec3 result(0, 0, 0);

        for (const auto& light : lights) {
            double nDotL =
                std::max(0.0, dot(h.normal, light.direction));

            result += kd * light.intensity * nDotL;
        }

        return result;
    }

private:
    vec3 kd;
};