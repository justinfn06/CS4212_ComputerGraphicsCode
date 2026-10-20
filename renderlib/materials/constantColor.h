#pragma once

#include "shader.h"

class ConstantColor : public Shader
{
public:
    explicit ConstantColor(vec3 value)
        : value(value) {}

    vec3 rayColor(
        const HitStruct&,
        const std::vector<LightSample>&,
        const Scene&,
        int
    ) override
    {
        return value;
    }

private:
    vec3 value;
};