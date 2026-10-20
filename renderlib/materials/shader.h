#pragma once
#include "../core/HitStruct.h"
#include <vector>
#include "../lighting/lightSample.h"

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