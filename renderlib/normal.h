#pragma once
#include "shader.h"

class Normal :public Shader
{
    public:

    vec3 rayColor(
        const HitStruct& h,
        const std::vector<LightSample>&,
        const Scene&,
        int
    ) override
    {
        return (h.normal + vec3(1.0, 1.0, 1.0)) / 2;
    }
};