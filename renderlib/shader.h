#pragma once
#include "HitStruct.h"
#include <vector>
#include "lightSample.h"

struct Scene;

class Shader
{
public:
    virtual ~Shader() = default;
    virtual vec3 rayColor(
        const HitStruct& h, 
        const std::vector<LightSample>& lights,
        const Scene& scene,
        int depth
    ) = 0;
};