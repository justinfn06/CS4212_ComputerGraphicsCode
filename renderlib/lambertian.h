#pragma once
#include <algorithm>
#include "shader.h"

class Lambertian : public Shader
{
public:
    explicit Lambertian(vec3 diffuseColor)
        : kd(diffuseColor) {}

    vec3 rayColor(const HitStruct& h) override
    {
        vec3 toLight = lightPosition - h.point;

        if (toLight.length_squared() == 0.0) {
            return vec3(0, 0, 0);
        }

        vec3 lightDir = unit_vector(toLight);
        double nDotL = std::max(0.0, dot(h.normal, lightDir));

        return kd * nDotL;
    }

private:
    vec3 kd;
    vec3 lightPosition = vec3(0, 10, 5);
};